#ifndef _KPU_H
#define _KPU_H

#include <stdint.h>
#include <plic.h>
#include "dmac.h"

typedef int (*plic_irq_callback_t)(void *ctx);

typedef struct
{
    union
    {
        uint64_t reg;
        struct
        {
            uint64_t int_en:1;
            uint64_t ram_flag:1;
            uint64_t full_add:1;
            uint64_t depth_wise_layer:1;
            uint64_t reserved:60;
        } data;
    } interrupt_enabe;

    union
    {
        uint64_t reg;
        struct
        {
            uint64_t image_src_addr:15;
            uint64_t reserved0:17;
            uint64_t image_dst_addr:15;
            uint64_t reserved1:17;
        } data;
    } image_addr;

    union
    {
        uint64_t reg;
        struct
        {
            uint64_t i_ch_num:10;
            uint64_t reserved0:22;
            uint64_t o_ch_num:10;
            uint64_t reserved1:6;
            uint64_t o_ch_num_coef:10;
            uint64_t reserved2:6;
        } data;
    } image_channel_num;

    union
    {
        uint64_t reg;
        struct
        {
            uint64_t i_row_wid:10;
            uint64_t i_col_high:9;
            uint64_t reserved0:13;
            uint64_t o_row_wid:10;
            uint64_t o_col_high:9;
            uint64_t reserved1:13;
        } data;
    } image_size;

    union
    {
        uint64_t reg;
        struct
        {
            uint64_t kernel_type:3;
            uint64_t pad_type:1;
            uint64_t pool_type:4;
            uint64_t first_stride:1;
            uint64_t bypass_conv:1;
            uint64_t load_para:1;
            uint64_t reserved0:5;
            uint64_t dma_burst_size:8;
            uint64_t pad_value:8;
            uint64_t bwsx_base_addr:32;
        } data;
    } kernel_pool_type_cfg;

    union
    {
        uint64_t reg;
        struct
        {
            uint64_t load_coor:1;
            uint64_t load_time:6;
            uint64_t reserved0:8;
            uint64_t para_size:17;
            uint64_t para_start_addr:32;
        } data;
    } kernel_load_cfg;

    union
    {
        uint64_t reg;
        struct
        {
            uint64_t coef_column_offset:4;
            uint64_t coef_row_offset:12;
            uint64_t reserved0:48;
        } data;
    } kernel_offset;

    union
    {
        uint64_t reg;
        struct
        {
            uint64_t channel_switch_addr:15;
            uint64_t reserved:1;
            uint64_t row_switch_addr:4;
            uint64_t coef_size:8;
            uint64_t coef_group:3;
            uint64_t load_act:1;
            uint64_t active_addr:32;
        } data;
    } kernel_calc_type_cfg;

    union
    {
        uint64_t reg;
        struct
        {
            uint64_t wb_channel_switch_addr:15;
            uint64_t reserved0:1;
            uint64_t wb_row_switch_addr:4;
            uint64_t wb_group:3;
            uint64_t reserved1:41;
        } data;
    } write_back_cfg;

    union
    {
        uint64_t reg;
        struct
        {
            uint64_t shr_w:4;
            uint64_t shr_x:4;
            uint64_t arg_w:24;
            uint64_t arg_x:24;
            uint64_t reserved0:8;
        } data;
    } conv_value;

    union
    {
        uint64_t reg;
        struct
        {
            uint64_t arg_add:40;
            uint64_t reserved:24;
        } data;
    } conv_value2;

    union
    {
        uint64_t reg;
        struct
        {
            uint64_t send_data_out:1;
            uint64_t reserved:15;
            uint64_t channel_byte_num:16;
            uint64_t dma_total_byte:32;
        } data;
    } dma_parameter;
} kpu_layer_argument_t;

typedef struct
{
    union
    {
        uint64_t reg;
        struct
        {
            uint64_t shift_number:8;
            uint64_t y_mul:16;
            uint64_t x_start:36;
        } data;
    } activate_para[16];

    union
    {
        uint64_t reg;
        struct
        {
            uint8_t result_bias[8];
        } data;
    } activate_para_bias0;

    union
    {
        uint64_t reg;
        struct
        {
            uint8_t result_bias[8];
        } data;
    } activate_para_bias1;
} kpu_activate_table_t;

typedef struct
{
    union
    {
        uint64_t reg;
        struct
        {
            uint64_t norm_mul:24;
            uint64_t norm_add:32;
            uint64_t norm_shift:4;
        } data;
    } batchnorm;
} kpu_batchnorm_argument_t;


typedef struct
{
    union
    {
        uint64_t reg;
        struct
        {
            uint16_t weight[9];
        } data;
    } weights;
} kpu_weights_kernel_16_3x3_t;

typedef struct
{
    uint64_t calc_done_int:1;
    uint64_t layer_cfg_almost_empty_int:1;
    uint64_t layer_cfg_almost_full_int:1;
    uint64_t reserved:61;
} kpu_config_interrupt_t;

typedef struct
{
    uint64_t fifo_full_threshold:4;
    uint64_t fifo_empty_threshold:4;
    uint64_t reserved:56;
} kpu_config_fifo_threshold_t;

typedef struct
{
    uint64_t dma_fifo_flush_n:1;
    uint64_t gs_fifo_flush_n:1;
    uint64_t cfg_fifo_flush_n:1;
    uint64_t cmd_fifo_flush_n:1;
    uint64_t resp_fifo_flush_n:1;
    uint64_t reserved:59;
} kpu_config_fifo_ctrl_t;

typedef struct
{
    uint64_t eight_bit_mode:1;
    uint64_t reserved:63;
} kpu_config_eight_bit_mode_t;


typedef struct
{
    volatile uint64_t layer_argument_fifo;

    volatile union
    {
        uint64_t reg;
        kpu_config_interrupt_t data;
    } interrupt_status;

    volatile  union
    {
        uint64_t reg;
        kpu_config_interrupt_t  data;
    } interrupt_raw;

    volatile  union {
        uint64_t reg;
        kpu_config_interrupt_t  data;
    } interrupt_mask;

    volatile  union
    {
        uint64_t reg;
        kpu_config_interrupt_t data;
    } interrupt_clear;

    volatile  union
    {
        uint64_t reg;
        kpu_config_fifo_threshold_t  data;
    } fifo_threshold;

    volatile uint64_t fifo_data_out;

    volatile  union
    {
        uint64_t reg;
        kpu_config_fifo_ctrl_t  data;
    } fifo_ctrl;

    volatile  union
    {
        uint64_t reg;
        kpu_config_eight_bit_mode_t  data;
    } eight_bit_mode;
} kpu_config_t;

extern volatile kpu_config_t *const kpu;

typedef struct
{
    kpu_layer_argument_t* layers;
    uint32_t length;
    int dma_ch;
    uint64_t* dst;
    uint32_t dst_length;
    plic_irq_callback_t cb;
} kpu_task_t;

/**
 * @brief       Modle complier init kpu handler
 *
 * @param[in]   task            Kpu handler
 *
 * @return      Kpu handler
 */
extern kpu_task_t* kpu_task_init(kpu_task_t* task);

/**
 * @brief       Kpu run for AI
 *
 * @param[in]   task                Kpu handler
 * @param[in]   dma_ch              DMA for kpu
 * @param[in]   src                 The picture data
 * @param[in]   dest                The result of kpu
 * @param[in]   callback            The callback of kpu
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail.Kpu is busy.
 */
int kpu_run(kpu_task_t* task, dmac_channel_number_t dma_ch, const void *src, void* dest, plic_irq_callback_t callback);

/**
 * @brief       Get kpu result buf
 *
 * @param[in]   task                Kpu handler
 *
 * @return      Kpu result buf
 */
uint8_t *kpu_get_output_buf(kpu_task_t* task);

/**
 * @brief       Release kpu output buf
 *
 * @param[in]   output_buf                Kpu output buf
 *
 */
void kpu_release_output_buf(uint8_t *output_buf);

#endif
