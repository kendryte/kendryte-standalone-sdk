/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "i2s.h"
#include "sysctl.h"
#include "fpioa.h"
#include "uarths.h"

#define DMA_IRQ 1

#define FRAME_LEN   128
uint32_t rx_buf[1024];

void io_mux_init(){

    fpioa_set_function(36, FUNC_I2S0_IN_D0);
    fpioa_set_function(37, FUNC_I2S0_WS);
    fpioa_set_function(38, FUNC_I2S0_SCLK);

    fpioa_set_function(33, FUNC_I2S2_OUT_D1);
    fpioa_set_function(35, FUNC_I2S2_SCLK);
    fpioa_set_function(34, FUNC_I2S2_WS);
}

typedef struct _index_t
{
    uint32_t r_index;
    uint32_t t_index;
} index_t;

index_t g_index_t;

int i2s_on_irq(void *ctx)
{
    index_t *index = (index_t *)ctx;
    index->r_index += (FRAME_LEN*2);
    if(index->r_index >= 1023)
    {
        index->r_index = 0;
    }
    i2s_data_t data = (i2s_data_t)
    {
        .rx_channel = DMAC_CHANNEL0,
        .rx_buf = &rx_buf[index->r_index],
        .rx_len = FRAME_LEN * 2,
        .transfer_mode = I2S_RECEIVE,
    };
    plic_interrupt_t irq = (plic_interrupt_t)
    {
        .callback = i2s_on_irq,
        .ctx = &g_index_t,
        .priority = 1,
    };
    i2s_handle_data_dma(I2S_DEVICE_0, data, &irq);

    if (index->r_index - index->t_index >= FRAME_LEN || index->t_index - index->r_index >= (1023 - FRAME_LEN * 2))
    {
        data = (i2s_data_t)
        {
            .tx_channel = DMAC_CHANNEL1,
            .tx_buf = &rx_buf[index->t_index],
            .tx_len = FRAME_LEN * 2,
            .transfer_mode = I2S_SEND,
        };
        i2s_handle_data_dma(I2S_DEVICE_2, data, NULL);
        index->t_index += (FRAME_LEN * 2);
        if (index->t_index >= 1023)
            index->t_index = 0;
    }
}

int main(void)
{
    sysctl_pll_set_freq(SYSCTL_PLL0, 320000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();
    io_mux_init();
    printf("I2S0 receive , I2S2 play...\n");

    i2s_init(I2S_DEVICE_0, I2S_RECEIVER, 0x3);
    i2s_init(I2S_DEVICE_2, I2S_TRANSMITTER, 0xC);

    i2s_rx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_0,
        RESOLUTION_16_BIT, SCLK_CYCLES_32,
        TRIGGER_LEVEL_4, STANDARD_MODE);

    i2s_tx_channel_config(I2S_DEVICE_2, I2S_CHANNEL_1,
        RESOLUTION_16_BIT, SCLK_CYCLES_32,
        TRIGGER_LEVEL_4,
        RIGHT_JUSTIFYING_MODE
        );

#if !DMA_IRQ
    i2s_data_t data = (i2s_data_t)
    {
        .rx_channel = DMAC_CHANNEL0,
        .rx_buf = rx_buf,
        .rx_len = FRAME_LEN * 2,
        .transfer_mode = I2S_RECEIVE,
    };
    i2s_handle_data_dma(I2S_DEVICE_0, data, NULL);
    while (1)
    {
        g_index += (FRAME_LEN*2);
        if(g_index >= 1023)
        {
            g_index = 0;
        }
        data = (i2s_data_t)
        {
            .rx_channel = DMAC_CHANNEL0,
            .rx_buf = &rx_buf[g_index],
            .rx_len = FRAME_LEN * 2,
            .transfer_mode = I2S_RECEIVE,
        };

        i2s_handle_data_dma(I2S_DEVICE_0, data, NULL);
        if (g_index - g_tx_len >= FRAME_LEN || g_tx_len - g_index >= (1023 - FRAME_LEN * 2))
        {
            data = (i2s_data_t)
            {
                .tx_channel = DMAC_CHANNEL1,
                .tx_buf = &rx_buf[g_tx_len],
                .tx_len = FRAME_LEN * 2,
                .transfer_mode = I2S_SEND,
            };
            i2s_handle_data_dma(I2S_DEVICE_2, data, NULL);
            g_tx_len += (FRAME_LEN * 2);
            if (g_tx_len >= 1023)
                g_tx_len = 0;
        }
    }
#else
    i2s_data_t data = (i2s_data_t)
    {
        .rx_channel = DMAC_CHANNEL0,
        .rx_buf = rx_buf,
        .rx_len = FRAME_LEN * 2,
        .transfer_mode = I2S_RECEIVE,
    };
    plic_interrupt_t irq = (plic_interrupt_t)
    {
        .callback = i2s_on_irq,
        .ctx = &g_index_t,
        .priority = 1,
    };
    i2s_handle_data_dma(I2S_DEVICE_0, data, &irq);
    while(1);
#endif

    return 0;
}

