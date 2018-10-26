#include "kpu.h"
#include <platform.h>
#include <sysctl.h>
#include <stdio.h>
#include <stdlib.h>
#include "printf.h"
#include "dmac.h"
#include <string.h>

volatile kpu_config_t *const kpu = (volatile kpu_config_t *)AI_BASE_ADDR;

typedef struct kpu_context
{
    kpu_task_t kpu_task;
    uint32_t kpu_status;
} kpu_context_t;

volatile kpu_context_t g_kpu_context;

static int kpu_run_all_done(void* _task)
{
    g_kpu_context.kpu_status = 0;
    kpu_task_t* task = (kpu_task_t*)_task;
    task->cb(task);
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

    if(task->length == 0)
    {
        return 0;
    }
    if(task->length <= layer_burst_size)
    {
        for(uint32_t i=0; i<task->length; i++)
        {
            kpu->layer_argument_fifo = task->layers[i].interrupt_enabe.reg;
            kpu->layer_argument_fifo = task->layers[i].image_addr.reg;
            kpu->layer_argument_fifo = task->layers[i].image_channel_num.reg;
            kpu->layer_argument_fifo = task->layers[i].image_size.reg;
            kpu->layer_argument_fifo = task->layers[i].kernel_pool_type_cfg.reg;
            kpu->layer_argument_fifo = task->layers[i].kernel_load_cfg.reg;
            kpu->layer_argument_fifo = task->layers[i].kernel_offset.reg;
            kpu->layer_argument_fifo = task->layers[i].kernel_calc_type_cfg.reg;
            kpu->layer_argument_fifo = task->layers[i].write_back_cfg.reg;
            kpu->layer_argument_fifo = task->layers[i].conv_value.reg;
            kpu->layer_argument_fifo = task->layers[i].conv_value2.reg;
            kpu->layer_argument_fifo = task->layers[i].dma_parameter.reg;
        }
        task->length = 0;
    }
    else
    {
        for(uint32_t i=0; i<layer_burst_size; i++)
        {
            kpu->layer_argument_fifo = task->layers[i].interrupt_enabe.reg;
            kpu->layer_argument_fifo = task->layers[i].image_addr.reg;
            kpu->layer_argument_fifo = task->layers[i].image_channel_num.reg;
            kpu->layer_argument_fifo = task->layers[i].image_size.reg;
            kpu->layer_argument_fifo = task->layers[i].kernel_pool_type_cfg.reg;
            kpu->layer_argument_fifo = task->layers[i].kernel_load_cfg.reg;
            kpu->layer_argument_fifo = task->layers[i].kernel_offset.reg;
            kpu->layer_argument_fifo = task->layers[i].kernel_calc_type_cfg.reg;
            kpu->layer_argument_fifo = task->layers[i].write_back_cfg.reg;
            kpu->layer_argument_fifo = task->layers[i].conv_value.reg;
            kpu->layer_argument_fifo = task->layers[i].conv_value2.reg;
            kpu->layer_argument_fifo = task->layers[i].dma_parameter.reg;
        }
        task->layers += layer_burst_size;
        task->length -= layer_burst_size;
    }
    return 0;
}

static int kpu_run_dma_output(uint32_t dma_ch, void* dst, uint32_t length, plic_irq_callback_t cb, void* _task)
{
    sysctl_dma_select(dma_ch, SYSCTL_DMA_SELECT_AI_RX_REQ);
    dmac_set_irq(dma_ch, cb, _task, 1);
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
        .eight_bit_mode=0
    };

    kpu_layer_argument_t* last_layer = &task->layers[task->length-1];

    kpu_run_dma_output(task->dma_ch, task->dst, last_layer->dma_parameter.data.dma_total_byte+1, kpu_run_all_done, task);

    kpu->interrupt_mask.reg = 7;
    kpu_continue(task);
    kpu->interrupt_mask.data = (kpu_config_interrupt_t)
    {
        .calc_done_int=0,
        .layer_cfg_almost_empty_int=0,
        .layer_cfg_almost_full_int=1
    };
    return 0;
}

static void kpu_run_dma_input(uint32_t dma_ch, const void* src, plic_irq_callback_t cb, void* _task)
{
    kpu_task_t* task = _task;
    kpu_layer_argument_t* first_layer = &task->layers[0];
    uint64_t input_size = first_layer->kernel_calc_type_cfg.data.channel_switch_addr * 64 * (first_layer->image_channel_num.data.i_ch_num+1);
    dmac_set_irq(dma_ch, cb, _task, 1);
    dmac_set_single_mode(dma_ch, (void *)src, (void *)(AI_IO_BASE_ADDR), DMAC_ADDR_INCREMENT, DMAC_ADDR_INCREMENT,
        DMAC_MSIZE_16, DMAC_TRANS_WIDTH_64, input_size / 8);
}

int kpu_run(kpu_task_t* v_task, dmac_channel_number_t dma_ch, const void *src, void* dest, plic_irq_callback_t callback)
{
    if(g_kpu_context.kpu_status)
        return -1;

    memcpy((void *)&g_kpu_context.kpu_task, v_task, sizeof(kpu_task_t));
    kpu_task_t *task = (kpu_task_t *)&g_kpu_context.kpu_task;

    kpu_layer_argument_t* last_layer = &task->layers[task->length-1];

    uint64_t output_size = last_layer->dma_parameter.data.dma_total_byte+1;

    last_layer->dma_parameter.data.send_data_out = 1;
    last_layer->interrupt_enabe.data.int_en = 1;

    task->dma_ch = dma_ch;
    task->dst = dest;
    task->dst_length = output_size;
    task->cb = callback;

    plic_irq_enable(IRQN_AI_INTERRUPT);
    plic_set_priority(IRQN_AI_INTERRUPT, 1);
    plic_irq_register(IRQN_AI_INTERRUPT, kpu_continue, task);

    kpu_run_dma_input(dma_ch, src, kpu_run_dma_input_done_push_layers, task);

    return 0;
}

uint8_t *kpu_get_output_buf(kpu_task_t* task)
{
    kpu_layer_argument_t* last_layer = &task->layers[task->length-1];
    size_t output_size = ((last_layer->dma_parameter.data.dma_total_byte+1) + 7) / 8 * 8;
    return malloc(output_size);
}

void kpu_release_output_buf(uint8_t *output_buf)
{
    if(output_buf != NULL)
        free(output_buf);
}

