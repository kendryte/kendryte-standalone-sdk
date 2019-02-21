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

extern const int16_t test_pcm[1223019];
#define PCM_LEN (sizeof(test_pcm)/2)
#define FRAME_LEN  256
uint32_t pcm_buf[FRAME_LEN * 4];
uint32_t echo_buf[FRAME_LEN * 4];
int16_t aec_buf[FRAME_LEN * 4];
int16_t echo_data[PCM_LEN];
uint32_t g_index;

void io_mux_init(){

    fpioa_set_function(36, FUNC_I2S1_IN_D0);
    fpioa_set_function(37, FUNC_I2S1_WS);
    fpioa_set_function(38, FUNC_I2S1_SCLK);

    fpioa_set_function(33, FUNC_I2S2_OUT_D1);
    fpioa_set_function(35, FUNC_I2S2_SCLK);
    fpioa_set_function(34, FUNC_I2S2_WS);
}

#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"

#define NN FRAME_LEN
#define TAIL 2048
// 1024

int main(void)
{
    //sysctl_pll_set_freq(SYSCTL_PLL0, 320000000UL);
    //sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();
    io_mux_init();
    printf("I2S0 receive , I2S2 play...\n");

    g_index = 0;

    i2s_init(I2S_DEVICE_1, I2S_RECEIVER, 0x3);
    i2s_init(I2S_DEVICE_2, I2S_TRANSMITTER, 0xC);

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
	int sampleRate = 44100;

    st = speex_echo_state_init(NN, TAIL);
	den = speex_preprocess_state_init(NN, sampleRate);
	speex_echo_ctl(st, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
	speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_STATE, st);

    while (1)
    {
        g_index += FRAME_LEN;
        if(g_index >= PCM_LEN)
        {
            // while(1);
            g_index = 0;
        }
        uint32_t ring_buffer_pos = 2*(g_index%(FRAME_LEN*2));
        i2s_send_data_dma(I2S_DEVICE_2, &pcm_buf[FRAME_LEN*2-ring_buffer_pos], FRAME_LEN*2, DMAC_CHANNEL0);
        i2s_receive_data_dma(I2S_DEVICE_1, &echo_buf[FRAME_LEN*2-ring_buffer_pos], FRAME_LEN * 2, DMAC_CHANNEL1);
        
        
        for(int i=0; i<FRAME_LEN; i++){
            echo_data[g_index+i] = (int32_t)echo_buf[ring_buffer_pos+i*2+1];
        }
		speex_echo_cancellation(st, &echo_data[g_index], &test_pcm[g_index], &aec_buf[ring_buffer_pos]);
		speex_preprocess_run(den, &aec_buf[ring_buffer_pos]);

        for(int i=0; i<FRAME_LEN; i++){
            pcm_buf[ring_buffer_pos+i*2] = (int32_t)test_pcm[g_index+i];
            pcm_buf[ring_buffer_pos+i*2+1] = (int32_t)aec_buf[ring_buffer_pos+i];//echo_data[g_index+i];
        }
    }

    speex_echo_state_destroy(st);
    speex_preprocess_state_destroy(den);

    return 0;
}