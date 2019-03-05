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
#include "config.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <printf.h>
#include <i2s.h>
#include <sysctl.h>
#include <fpioa.h>
#include <uarths.h>
#include "aec/speex_echo.h"
#include "aec/speex_preprocess.h"
#include "dereverberation/kal_wpe_float.h"

extern const int16_t test_pcm[1223019];
#define TEST_PCM_LEN (sizeof(test_pcm)/2)

uint32_t pcm_buf[AEC_FRAME_LEN * 4];
uint32_t echo_buf[AEC_FRAME_LEN * 4];
int16_t aec_buf[AEC_FRAME_LEN];
int16_t echo_data[AEC_FRAME_LEN];
uint32_t g_index;

void io_mux_init(){

    fpioa_set_function(36, FUNC_I2S1_IN_D0);
    fpioa_set_function(37, FUNC_I2S1_WS);
    fpioa_set_function(38, FUNC_I2S1_SCLK);

    fpioa_set_function(33, FUNC_I2S2_OUT_D1);
    fpioa_set_function(35, FUNC_I2S2_SCLK);
    fpioa_set_function(34, FUNC_I2S2_WS);
}



int32_t imin(int32_t a, int32_t b){
    return a<=b ? a : b;
}
int32_t imax(int32_t a, int32_t b){
    return a<=b ? b : a;
}

int main(void)
{
    // sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_cpu_set_freq(500000000UL);
    //sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, SAMPLE_RATE*1024UL*2UL);
    uint32_t cpu_freq = sysctl_pll_get_freq(SYSCTL_PLL0);
    uint32_t pll2_freq = sysctl_pll_get_freq(SYSCTL_PLL2);

    uarths_init();
    io_mux_init();
    printk("I2S0 receive , I2S2 play...\n");
    printk("cpu_freq: %d, pll2_freq: %d \n", cpu_freq, pll2_freq);

    g_index = 0;

    i2s_init(I2S_DEVICE_1, I2S_RECEIVER, 0x3);
    i2s_init(I2S_DEVICE_2, I2S_TRANSMITTER, 0xC);
    i2s_set_sample_rate(I2S_DEVICE_1, SAMPLE_RATE);
    i2s_set_sample_rate(I2S_DEVICE_2, SAMPLE_RATE);

    i2s_rx_channel_config(I2S_DEVICE_1, I2S_CHANNEL_0,
        RESOLUTION_16_BIT, SCLK_CYCLES_32,
        TRIGGER_LEVEL_4, STANDARD_MODE);

    i2s_tx_channel_config(I2S_DEVICE_2, I2S_CHANNEL_1,
        RESOLUTION_16_BIT, SCLK_CYCLES_32,
        TRIGGER_LEVEL_4,
        RIGHT_JUSTIFYING_MODE
        );


	SpeexEchoState *st;
	SpeexPreprocessState *den;
	int sampleRate = SAMPLE_RATE;

    st = speex_echo_state_init(AEC_FRAME_LEN, AEC_TAIL);
	den = speex_preprocess_state_init(AEC_FRAME_LEN, sampleRate);
	speex_echo_ctl(st, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
	speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_STATE, st);

    uint64_t last_cycle=0, current_cycle, base_cycle;
    while (1)
    {
        g_index += AEC_FRAME_LEN;
        if(g_index >= TEST_PCM_LEN)
        {
            // while(1);
            g_index = 0;
        }
        uint32_t ring_buffer_pos = 2*(g_index%(AEC_FRAME_LEN*2));

        base_cycle = read_csr(mcycle);
        i2s_send_data_dma(I2S_DEVICE_2, &pcm_buf[AEC_FRAME_LEN*2-ring_buffer_pos], AEC_FRAME_LEN * 2, DMAC_CHANNEL0);
        i2s_receive_data_dma(I2S_DEVICE_1, &echo_buf[AEC_FRAME_LEN*2-ring_buffer_pos], AEC_FRAME_LEN * 2, DMAC_CHANNEL1);
        current_cycle = read_csr(mcycle);
        printk("%lu %lu\n", current_cycle-last_cycle, current_cycle-base_cycle);
        last_cycle = current_cycle;
        
        
        for(int i=0; i<AEC_FRAME_LEN; i++){
            echo_data[i] = (int32_t)echo_buf[ring_buffer_pos+i*2+1];
        }
		speex_echo_cancellation(st, echo_data, &test_pcm[g_index], aec_buf);
		speex_preprocess_run(den, aec_buf);
        for(int i=0; i<AEC_FRAME_LEN/KAL_FRAME_LEN; i++){
            process_frame_float(&aec_buf[i*KAL_FRAME_LEN], &aec_buf[i*KAL_FRAME_LEN]);
        }

        for(int i=0; i<AEC_FRAME_LEN; i++){
            pcm_buf[ring_buffer_pos+i*2] = (int32_t)test_pcm[g_index+i];
            pcm_buf[ring_buffer_pos+i*2+1] = (int32_t)aec_buf[i];//echo_data[g_index+i];
        }

        // int32_t cmin=1<<30, cmax=-(1<<30), emin=1<<30, emax=-(1<<30);
        // for(int i=0; i<FRAME_LEN; i++){
        //     cmin = imin(cmin, aec_buf[ring_buffer_pos+i]);
        //     cmax = imax(cmax, aec_buf[ring_buffer_pos+i]);
        //     emin = imin(emin, echo_buf[ring_buffer_pos+i*2+1]);
        //     emax = imax(emax, echo_buf[ring_buffer_pos+i*2+1]);
        // }
        // printf("%d\t%d\t%d\t%d\n", cmin, cmax, emin, emax);
    }   

    speex_echo_state_destroy(st);
    speex_preprocess_state_destroy(den);

    return 0;
}