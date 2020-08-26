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

#define FRAME_LEN   512
int16_t rx_buf[FRAME_LEN * 2 * 2];
uint32_t g_index;
uint32_t g_tx_len;

uint32_t g_rx_dma_buf[FRAME_LEN * 2 * 2];
uint8_t i2s_rec_flag;

void io_mux_init(){

    fpioa_set_function(36, FUNC_I2S0_IN_D0);
    fpioa_set_function(37, FUNC_I2S0_WS);
    fpioa_set_function(38, FUNC_I2S0_SCLK);

    fpioa_set_function(33, FUNC_I2S2_OUT_D1);
    fpioa_set_function(35, FUNC_I2S2_SCLK);
    fpioa_set_function(34, FUNC_I2S2_WS);
}
int i2s_dma_irq(void *ctx)
{
    uint32_t i;

    if(g_index)
    {
        
        i2s_receive_data_dma(I2S_DEVICE_0, &g_rx_dma_buf[g_index], FRAME_LEN * 2, DMAC_CHANNEL1);
        g_index = 0;
        for(i = 0; i < FRAME_LEN; i++)
        {
            rx_buf[2 * i] = (int16_t)(g_rx_dma_buf[2 * i + 1] & 0xffff);
            rx_buf[2 * i + 1] = (int16_t)(g_rx_dma_buf[2 * i + 1] & 0xffff);
        }
        i2s_rec_flag = 1;
    }
    else
    {
        i2s_receive_data_dma(I2S_DEVICE_0, &g_rx_dma_buf[0], FRAME_LEN * 2, DMAC_CHANNEL1);
        g_index = FRAME_LEN * 2;
        for(i = FRAME_LEN; i < FRAME_LEN * 2; i++)
        {
            rx_buf[2 * i] = (int16_t)(g_rx_dma_buf[2 * i + 1] & 0xffff);
            rx_buf[2 * i + 1] = (int16_t)(g_rx_dma_buf[2 * i + 1] & 0xffff);
        }
        i2s_rec_flag = 2;
    }
    return 0;
}
int main(void)
{
    sysctl_pll_set_freq(SYSCTL_PLL0, 320000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();
    io_mux_init();
    printf("I2S0 receive , I2S2 play...\n");

    g_index = 0;
    g_tx_len = 0;

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
    i2s_set_sample_rate(I2S_DEVICE_0, 16000);
    i2s_set_sample_rate(I2S_DEVICE_2, 16000);
    plic_init();
    dmac_init();
    dmac_set_irq(DMAC_CHANNEL1, i2s_dma_irq, NULL, 4);

    i2s_receive_data_dma(I2S_DEVICE_0, &rx_buf[g_index], FRAME_LEN * 2, DMAC_CHANNEL1);
    
    while (1)
    {
        if(i2s_rec_flag == 1)
        {
            i2s_play(I2S_DEVICE_2,
            DMAC_CHANNEL0, (uint8_t *)(&rx_buf[0]), FRAME_LEN * 4, 1024, 16, 2);
            i2s_rec_flag = 0;
        }
        else if(i2s_rec_flag == 2)
        {
            i2s_play(I2S_DEVICE_2,
            DMAC_CHANNEL0, (uint8_t *)(&rx_buf[FRAME_LEN * 2]), FRAME_LEN * 4, 1024, 16, 2);
            i2s_rec_flag = 0;
        }
    }

    return 0;
}
