#include "kpu.h"
#include <platform.h>
#include <sysctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "printf.h"
#include "dmac.h"
#include <string.h>
#include "bsp.h"

#define LAYER_BURST_SIZE 12

volatile kpu_config_t *const kpu = (volatile kpu_config_t *)AI_BASE_ADDR;
static volatile uint32_t kpu_status;

typedef struct kpu_context
{
    kpu_task_t kpu_task;
    uint32_t kpu_status;
} kpu_context_t;

volatile kpu_context_t g_kpu_context;

static int kpu_run_all_done(void* _task)
{
    atomic_swap(&g_kpu_context.kpu_status, 0);
    kpu_task_t* task = (kpu_task_t*)_task;
    task->callback(task);
    return 0;
}

int kpu_continue(void* _task)
{
    kpu_task_t* task = (kpu_task_t*)_task;
    int layer_burst_size = 1;

    kpu->interrupt_clear.data = (kpu_config_interrupt_t)
    {
        .calc_done_int=1,
        .layer_cfg_almost_empty_int=1,
        .layer_cfg_almost_full_int=1
    };

    if(task->remain_layers_length == 0)
    {
        return 0;
    }
    if(task->remain_layers_length <= layer_burst_size)
    {
        for(uint32_t i=0; i<task->remain_layers_length; i++)
        {
            kpu->layer_argument_fifo = task->remain_layers[i].interrupt_enabe.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_addr.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_channel_num.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_size.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_pool_type_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_load_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_offset.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_calc_type_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].write_back_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].conv_value.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].conv_value2.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].dma_parameter.reg;
        }
        task->remain_layers_length = 0;
    }
    else
    {
        for(uint32_t i=0; i<layer_burst_size; i++)
        {
            kpu->layer_argument_fifo = task->remain_layers[i].interrupt_enabe.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_addr.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_channel_num.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_size.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_pool_type_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_load_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_offset.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_calc_type_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].write_back_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].conv_value.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].conv_value2.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].dma_parameter.reg;
        }
        task->remain_layers += layer_burst_size;
        task->remain_layers_length -= layer_burst_size;
    }
    return 0;
}

static int kpu_run_dma_output(uint32_t dma_ch, void* dst, uint32_t length, plic_irq_callback_t cb, void* _task)
{
    sysctl_dma_select(dma_ch, SYSCTL_DMA_SELECT_AI_RX_REQ);
    dmac_irq_register(dma_ch, kpu_run_all_done, _task, 1);
    dmac_set_single_mode(dma_ch, (void *)(&kpu->fifo_data_out), (void *)(dst), DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT,
                         DMAC_MSIZE_8, DMAC_TRANS_WIDTH_64, (length+7)/8);
    return 0;
}

static int kpu_run_dma_input_done_push_layers(void* _task)
{
    kpu_task_t* task = (kpu_task_t*)_task;
    kpu->interrupt_clear.reg = 7;
    dmac->channel[task->dma_ch].intclear = 0xFFFFFFFF;
    kpu->fifo_threshold.data = (kpu_config_fifo_threshold_t)
    {
        .fifo_full_threshold = 10, .fifo_empty_threshold=1
    };
    kpu->eight_bit_mode.data = (kpu_config_eight_bit_mode_t)
    {
        .eight_bit_mode=task->eight_bit_mode
    };

    kpu_layer_argument_t* last_layer = &task->layers[task->layers_length-1];

    kpu_run_dma_output(task->dma_ch, task->dst, last_layer->dma_parameter.data.dma_total_byte+1, kpu_run_all_done, task);

    kpu->interrupt_mask.data = (kpu_config_interrupt_t)
    {
        .calc_done_int=0,
        .layer_cfg_almost_empty_int=0,
        .layer_cfg_almost_full_int=1
    };
    kpu_continue(task);
    return 0;
}

static void kpu_run_dma_input(uint32_t dma_ch, const void* src, plic_irq_callback_t cb, void* _task)
{
    kpu_task_t* task = _task;
    kpu_layer_argument_t* first_layer = &task->layers[0];
    uint64_t input_size = first_layer->kernel_calc_type_cfg.data.channel_switch_addr * 64 * (first_layer->image_channel_num.data.i_ch_num+1);
    dmac_irq_register(dma_ch, cb, _task, 1);
    dmac_set_single_mode(dma_ch, (void *)src, (void *)(AI_IO_BASE_ADDR), DMAC_ADDR_INCREMENT, DMAC_ADDR_INCREMENT,
        DMAC_MSIZE_16, DMAC_TRANS_WIDTH_64, input_size / 8);
}

int kpu_run(kpu_task_t* v_task, dmac_channel_number_t dma_ch, const void *src, void* dest, plic_irq_callback_t callback)
{
    if(atomic_cas(&g_kpu_context.kpu_status, 0, 1))
        return -1;

    memcpy((void *)&g_kpu_context.kpu_task, v_task, sizeof(kpu_task_t));
    kpu_task_t *task = (kpu_task_t *)&g_kpu_context.kpu_task;

    kpu_layer_argument_t* last_layer = &task->layers[task->layers_length-1];

    uint64_t output_size = last_layer->dma_parameter.data.dma_total_byte+1;

    last_layer->dma_parameter.data.send_data_out = 1;
    last_layer->interrupt_enabe.data.int_en = 1;

    task->dma_ch = dma_ch;
    task->dst = dest;
    task->dst_length = output_size;
    task->callback = callback;
    task->remain_layers_length = task->layers_length;
    task->remain_layers = task->layers;

    plic_irq_enable(IRQN_AI_INTERRUPT);
    plic_set_priority(IRQN_AI_INTERRUPT, 1);
    plic_irq_register(IRQN_AI_INTERRUPT, kpu_continue, task);

    kpu_run_dma_input(dma_ch, src, kpu_run_dma_input_done_push_layers, task);

    return 0;
}

uint8_t *kpu_get_output_buf(kpu_task_t* task)
{
    kpu_layer_argument_t* last_layer = &task->layers[task->layers_length-1];
    size_t output_size = ((last_layer->dma_parameter.data.dma_total_byte+1) + 7) / 8 * 8;
    return malloc(output_size);
}

void kpu_release_output_buf(uint8_t *output_buf)
{
    if(output_buf != NULL)
        free(output_buf);
}


static int kpu_done(void *ctx)
{
    atomic_swap(&kpu_status, 0);
    kpu_task_t *task = (kpu_task_t *)ctx;
    task->callback(task->ctx);
    return 0;
}

static int kpu_config_input(void *ctx)
{
    kpu_task_t *task = (kpu_task_t *)ctx;
    kpu->interrupt_clear.reg = 7;
    if (task->remain_layers_length <= LAYER_BURST_SIZE)
    {
        for (uint32_t i = 0; i < task->remain_layers_length; i++)
        {
            kpu->layer_argument_fifo = task->remain_layers[i].interrupt_enabe.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_addr.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_channel_num.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_size.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_pool_type_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_load_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_offset.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_calc_type_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].write_back_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].conv_value.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].conv_value2.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].dma_parameter.reg;
        }
        task->remain_layers_length = 0;
        kpu->interrupt_mask.reg = 7;
    }
    else
    {
        for (uint32_t i = 0; i < LAYER_BURST_SIZE; i++)
        {
            kpu->layer_argument_fifo = task->remain_layers[i].interrupt_enabe.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_addr.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_channel_num.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].image_size.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_pool_type_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_load_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_offset.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].kernel_calc_type_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].write_back_cfg.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].conv_value.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].conv_value2.reg;
            kpu->layer_argument_fifo = task->remain_layers[i].dma_parameter.reg;
        }
        task->remain_layers += LAYER_BURST_SIZE;
        task->remain_layers_length -= LAYER_BURST_SIZE;
    }
    return 0;
}

static void kpu_data_output(kpu_task_t *task)
{
    sysctl_dma_select(task->dma_ch, SYSCTL_DMA_SELECT_AI_RX_REQ);
    dmac_irq_register(task->dma_ch, kpu_done, task, 1);
    dmac_set_single_mode(task->dma_ch, (void *)(&kpu->fifo_data_out), (void *)(task->dst), DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT,
        DMAC_MSIZE_8, DMAC_TRANS_WIDTH_64, task->dst_length);
}

static int kpu_data_ready(void *ctx)
{
    kpu_task_t *task = (kpu_task_t *)ctx;

    dmac->channel[task->dma_ch].intclear = 0xFFFFFFFF;
    kpu_data_output(task);

    kpu->eight_bit_mode.reg = task->eight_bit_mode;
    kpu->interrupt_mask.reg = 7;
    kpu->interrupt_clear.reg = 7;
    kpu->fifo_threshold.data = (kpu_config_fifo_threshold_t)
    {
        .fifo_full_threshold = 12, .fifo_empty_threshold = 1
    };
    plic_irq_enable(IRQN_AI_INTERRUPT);
    plic_set_priority(IRQN_AI_INTERRUPT, 2);
    plic_irq_register(IRQN_AI_INTERRUPT, kpu_config_input, task);
    kpu_config_input(task);
    kpu->interrupt_mask.data = (kpu_config_interrupt_t)
    {
        .calc_done_int = 1,
        .layer_cfg_almost_empty_int = 0,
        .layer_cfg_almost_full_int = 1
    };
    return 0;
}

static void kpu_data_input(kpu_task_t *task)
{
    if (task->src == NULL)
    {
        kpu_data_ready(task);
        return;
    }
    dmac_irq_register(task->dma_ch, kpu_data_ready, task, 1);
    kpu_layer_argument_t *layer = &task->layers[0];
    dmac_set_single_mode(task->dma_ch, task->src, (void *)(AI_IO_BASE_ADDR + layer->image_addr.data.image_src_addr * 64), DMAC_ADDR_INCREMENT, DMAC_ADDR_INCREMENT,
        DMAC_MSIZE_16, DMAC_TRANS_WIDTH_64, task->src_length);
}

int kpu_single_task_init(kpu_task_t *task)
{
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    kpu_layer_argument_t *first_layer = &task->layers[0];
    kpu_layer_argument_t *last_layer = &task->layers[task->layers_length - 1];

    last_layer->dma_parameter.data.send_data_out = 1;
    last_layer->interrupt_enabe.data.int_en = 1;
    task->src_length = first_layer->kernel_calc_type_cfg.data.channel_switch_addr * 64 * (first_layer->image_channel_num.data.i_ch_num + 1) / 8;
    task->dst_length = ((last_layer->dma_parameter.data.dma_total_byte + 1) + 7) / 8;
    task->dst = (uint64_t *)malloc(task->dst_length * 8);
    memset(task->dst, 0, task->dst_length * 8);
    if (task->dst == NULL)
        return 1;
    return 0;
}

int kpu_single_task_deinit(kpu_task_t *task)
{
    free(task->dst);
    return 0;
}

int kpu_model_load_from_buffer(kpu_task_t *task, uint8_t *buffer, kpu_model_layer_metadata_t **meta)
{
    uintptr_t base_addr = (uintptr_t)buffer;
    kpu_model_header_t *header = (kpu_model_header_t *)buffer;
    kpu_model_layer_metadata_t *layer_meta = (kpu_model_layer_metadata_t *)(base_addr + sizeof(kpu_model_header_t));
    kpu_layer_argument_t *layers = (kpu_layer_argument_t *)(base_addr + header->layers_argument_start);

    if (header->version != 1)
        return -1;
    uint32_t layers_length = header->layers_length;
    task->layers_length = layers_length;
    task->eight_bit_mode = header->flags & 1;
    task->layers = layers;
    task->output_scale = layer_meta[layers_length - 1].output_scale;
    task->output_bias = layer_meta[layers_length - 1].output_bias;
    size_t i;
    for (i = 0; i < layers_length; i++)
    {
        layers[i].kernel_load_cfg.data.para_start_addr = (uint64_t)(base_addr + layer_meta[i].weigths_offset);
        layers[i].kernel_pool_type_cfg.data.bwsx_base_addr = (uint64_t)(base_addr + layer_meta[i].bn_offset);
        layers[i].kernel_calc_type_cfg.data.active_addr = (uint64_t)(base_addr + layer_meta[i].act_offset);
    }

    if (meta)
        *meta = layer_meta;
    return 0;
}

int kpu_start(kpu_task_t *task)
{
    if (atomic_cas(&kpu_status, 0, 1))
        return -1;

    task->remain_layers_length = task->layers_length;
    task->remain_layers = task->layers;
    kpu_data_input(task);
    return 0;
}

