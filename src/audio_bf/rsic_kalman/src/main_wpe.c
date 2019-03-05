/*
 * main_wpe.c
 *
 *  Created on: 2018/9/5
 *      Author: xiexi
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <printf.h>
#include "i2s.h"
#include "sysctl.h"
#include "fpioa.h"
#include "uarths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fft.h>
#include "../inc/kal_wpe_float.h"

#define FRAME_LEN  512
#define FRAME_SHIFT 512// 256
#define DMA_CH_I2S_RX DMAC_CHANNEL0
#define DMA_CH_I2S_TX DMAC_CHANNEL1
#define DMA_CH_FFT_RX DMAC_CHANNEL2
#define DMA_CH_FFT_TX DMAC_CHANNEL3
#define SAMPLE_RATE 16000UL

uint32_t mic_buf_ab[2][FRAME_SHIFT * 2]; 
uint32_t echo_buf_ab[2][FRAME_SHIFT * 2];
int16_t frame_rx[FRAME_LEN];
int16_t frame_rx_win[FRAME_LEN];
int16_t frame_tx[FRAME_LEN];
fft_data_t fft_buf_tx[FRAME_LEN];
fft_data_t fft_buf_rx[FRAME_LEN];

int16_t hanning_512[512] = {
            1,
           5,
          11,
          20,
          31,
          44,
          60,
          79,
          99,
         123,
         148,
         177,
         207,
         240,
         276,
         314,
         354,
         397,
         442,
         489,
         539,
         591,
         646,
         703,
         762,
         824,
         888,
         954,
        1023,
        1094,
        1167,
        1242,
        1320,
        1400,
        1482,
        1567,
        1654,
        1743,
        1834,
        1927,
        2023,
        2120,
        2220,
        2322,
        2426,
        2532,
        2640,
        2751,
        2863,
        2977,
        3094,
        3212,
        3332,
        3455,
        3579,
        3705,
        3833,
        3963,
        4095,
        4228,
        4364,
        4501,
        4640,
        4781,
        4923,
        5068,
        5214,
        5361,
        5511,
        5661,
        5814,
        5968,
        6124,
        6281,
        6440,
        6600,
        6762,
        6925,
        7089,
        7255,
        7423,
        7591,
        7761,
        7932,
        8105,
        8279,
        8454,
        8630,
        8807,
        8986,
        9165,
        9346,
        9528,
        9711,
        9894,
       10079,
       10265,
       10451,
       10639,
       10827,
       11016,
       11206,
       11397,
       11589,
       11781,
       11974,
       12167,
       12362,
       12556,
       12752,
       12948,
       13144,
       13341,
       13539,
       13736,
       13935,
       14133,
       14332,
       14531,
       14731,
       14931,
       15131,
       15331,
       15531,
       15732,
       15932,
       16133,
       16333,
       16534,
       16735,
       16935,
       17136,
       17336,
       17536,
       17736,
       17936,
       18136,
       18335,
       18534,
       18733,
       18932,
       19130,
       19327,
       19524,
       19721,
       19917,
       20113,
       20308,
       20503,
       20697,
       20890,
       21082,
       21274,
       21465,
       21656,
       21845,
       22034,
       22222,
       22409,
       22595,
       22780,
       22965,
       23148,
       23330,
       23511,
       23692,
       23871,
       24048,
       24225,
       24401,
       24575,
       24748,
       24920,
       25091,
       25260,
       25428,
       25595,
       25760,
       25924,
       26086,
       26247,
       26407,
       26565,
       26721,
       26876,
       27029,
       27181,
       27331,
       27480,
       27627,
       27772,
       27915,
       28057,
       28197,
       28335,
       28471,
       28606,
       28738,
       28869,
       28998,
       29125,
       29251,
       29374,
       29495,
       29614,
       29732,
       29847,
       29960,
       30072,
       30181,
       30288,
       30393,
       30496,
       30597,
       30696,
       30792,
       30887,
       30979,
       31069,
       31157,
       31243,
       31326,
       31407,
       31486,
       31563,
       31637,
       31709,
       31779,
       31846,
       31912,
       31974,
       32035,
       32093,
       32149,
       32202,
       32253,
       32302,
       32348,
       32392,
       32434,
       32473,
       32509,
       32544,
       32575,
       32605,
       32632,
       32656,
       32678,
       32698,
       32715,
       32730,
       32742,
       32752,
       32759,
       32764,
       32767,
       32767,
       32764,
       32759,
       32752,
       32742,
       32730,
       32715,
       32698,
       32678,
       32656,
       32632,
       32605,
       32575,
       32544,
       32509,
       32473,
       32434,
       32392,
       32348,
       32302,
       32253,
       32202,
       32149,
       32093,
       32035,
       31974,
       31912,
       31846,
       31779,
       31709,
       31637,
       31563,
       31486,
       31407,
       31326,
       31243,
       31157,
       31069,
       30979,
       30887,
       30792,
       30696,
       30597,
       30496,
       30393,
       30288,
       30181,
       30072,
       29960,
       29847,
       29732,
       29614,
       29495,
       29374,
       29251,
       29125,
       28998,
       28869,
       28738,
       28606,
       28471,
       28335,
       28197,
       28057,
       27915,
       27772,
       27627,
       27480,
       27331,
       27181,
       27029,
       26876,
       26721,
       26565,
       26407,
       26247,
       26086,
       25924,
       25760,
       25595,
       25428,
       25260,
       25091,
       24920,
       24748,
       24575,
       24401,
       24225,
       24048,
       23871,
       23692,
       23511,
       23330,
       23148,
       22965,
       22780,
       22595,
       22409,
       22222,
       22034,
       21845,
       21656,
       21465,
       21274,
       21082,
       20890,
       20697,
       20503,
       20308,
       20113,
       19917,
       19721,
       19524,
       19327,
       19130,
       18932,
       18733,
       18534,
       18335,
       18136,
       17936,
       17736,
       17536,
       17336,
       17136,
       16935,
       16735,
       16534,
       16333,
       16133,
       15932,
       15732,
       15531,
       15331,
       15131,
       14931,
       14731,
       14531,
       14332,
       14133,
       13935,
       13736,
       13539,
       13341,
       13144,
       12948,
       12752,
       12556,
       12362,
       12167,
       11974,
       11781,
       11589,
       11397,
       11206,
       11016,
       10827,
       10639,
       10451,
       10265,
       10079,
        9894,
        9711,
        9528,
        9346,
        9165,
        8986,
        8807,
        8630,
        8454,
        8279,
        8105,
        7932,
        7761,
        7591,
        7423,
        7255,
        7089,
        6925,
        6762,
        6600,
        6440,
        6281,
        6124,
        5968,
        5814,
        5661,
        5511,
        5361,
        5214,
        5068,
        4923,
        4781,
        4640,
        4501,
        4364,
        4228,
        4095,
        3963,
        3833,
        3705,
        3579,
        3455,
        3332,
        3212,
        3094,
        2977,
        2863,
        2751,
        2640,
        2532,
        2426,
        2322,
        2220,
        2120,
        2023,
        1927,
        1834,
        1743,
        1654,
        1567,
        1482,
        1400,
        1320,
        1242,
        1167,
        1094,
        1023,
         954,
         888,
         824,
         762,
         703,
         646,
         591,
         539,
         489,
         442,
         397,
         354,
         314,
         276,
         240,
         207,
         177,
         148,
         123,
          99,
          79,
          60,
          44,
          31,
          20,
          11,
           5,
           1
};

void io_mux_init(){

    fpioa_set_function(36, FUNC_I2S1_IN_D0);
    fpioa_set_function(37, FUNC_I2S1_WS);
    fpioa_set_function(38, FUNC_I2S1_SCLK);

    fpioa_set_function(33, FUNC_I2S2_OUT_D1);
    fpioa_set_function(35, FUNC_I2S2_SCLK);
    fpioa_set_function(34, FUNC_I2S2_WS);
}

int init_system(){
    sysctl_pll_set_freq(SYSCTL_PLL2, SAMPLE_RATE*1024);
    uarths_init();
    io_mux_init();

    i2s_init(I2S_DEVICE_1, I2S_RECEIVER, 0x3);
    i2s_init(I2S_DEVICE_2, I2S_TRANSMITTER, 0xC);

    i2s_rx_channel_config(I2S_DEVICE_1, I2S_CHANNEL_0,
        RESOLUTION_16_BIT, SCLK_CYCLES_32,
        TRIGGER_LEVEL_4, STANDARD_MODE
    );

    i2s_tx_channel_config(I2S_DEVICE_2, I2S_CHANNEL_1,
        RESOLUTION_16_BIT, SCLK_CYCLES_32,
        TRIGGER_LEVEL_4,
        RIGHT_JUSTIFYING_MODE
    );

    i2s_set_sample_rate(I2S_DEVICE_1, SAMPLE_RATE);
    i2s_set_sample_rate(I2S_DEVICE_2, SAMPLE_RATE);

    return 0;
}

// void dump_print(int16_t data[dump_frames][NRE]){
//     for(int i=0; i<dump_frames; i++){
//         for(int band=0; band<NRE; band++){
//             if(band%16==0){printf("...\n");}
//             printf("%d,", data[i][band]);
//         }
//         printf(";\n");
//     }
// }


void process_frame_float(int16_t frame_in[FRAME_LEN], int16_t frame_acc[FRAME_LEN]){
    z_t data[NRE];
    for(int i=0; i<FRAME_LEN/2; i++){
        uint32_t idx = i*2;
        fft_buf_tx[i].I1 = 0;
        fft_buf_tx[i].R1 = (int16_t)frame_in[idx];
        fft_buf_tx[i].I2 = 0;
        fft_buf_tx[i].R2 = (int16_t)frame_in[idx+1];
    }
    fft_complex_uint16_dma(DMA_CH_FFT_TX, DMA_CH_FFT_RX, 0x4a, FFT_DIR_FORWARD, fft_buf_tx, 512, fft_buf_rx);
    for(int i=0; i<128; i++){
        data[i*2].im = fft_buf_rx[i].I1;
        data[i*2].re = fft_buf_rx[i].R1;
        data[i*2+1].im = fft_buf_rx[i].I2;
        data[i*2+1].re = fft_buf_rx[i].R2;
    }
    data[256].im = fft_buf_rx[128].I1;
    data[256].re = fft_buf_rx[128].R1;

    kalman_wpe_float(data);

    for(int i=0; i<128; i++){
        fft_buf_tx[i].I1 = data[i*2].im;
        fft_buf_tx[i].R1 = data[i*2].re;
        fft_buf_tx[i].I2 = data[i*2+1].im;
        fft_buf_tx[i].R2 = data[i*2+1].re;
        fft_buf_tx[255-i].I1 = -data[i*2+2].im;
        fft_buf_tx[255-i].R1 = data[i*2+2].re;
        fft_buf_tx[255-i].I2 = -data[i*2+1].im;
        fft_buf_tx[255-i].R2 = data[i*2+1].re;
    }

    fft_complex_uint16_dma(DMA_CH_FFT_TX, DMA_CH_FFT_RX, 0x4a, FFT_DIR_BACKWARD, fft_buf_tx, 512, fft_buf_rx);
    for(int i=0; i<FRAME_LEN/2; i++){
        uint32_t idx = i*2;
        frame_acc[idx] += (int32_t)fft_buf_rx[i].R1;
        frame_acc[idx+1] += (int32_t)fft_buf_rx[i].R2;
    }
}


int main(void)
{
    init_system();
    // init state
    kalman_wpe_float_init();


    uint64_t last_cycle=0, current_cycle, base_cycle;
    int ping_pong=0;
    while (1)
    {
        ping_pong = 1-ping_pong;

        base_cycle = read_csr(mcycle);
        i2s_receive_data_dma(I2S_DEVICE_1, mic_buf_ab[1-ping_pong], FRAME_SHIFT * 2, DMA_CH_I2S_RX);
        i2s_send_data_dma(I2S_DEVICE_2, echo_buf_ab[1-ping_pong], FRAME_SHIFT * 2, DMA_CH_I2S_TX);

        current_cycle = read_csr(mcycle);
        printk("%lu %lu\n", current_cycle-last_cycle, current_cycle-base_cycle);
        last_cycle = current_cycle;

        // shift_window
        for(int i=0; i<FRAME_LEN-FRAME_SHIFT; i++){
            frame_rx[i] = frame_rx[FRAME_SHIFT+i];
        }
        for(int i=0; i<FRAME_SHIFT; i++){
            frame_rx[FRAME_LEN-FRAME_SHIFT+i] = (int32_t)mic_buf_ab[ping_pong][i*2+1];
        }

        // // add window func
        // for(int i=0; i<FRAME_LEN; i++){
        //     frame_rx_win[i] = ((int32_t)frame_rx[i] * (int32_t)hanning_512[i])>>16;
        // }

        process_frame_float(frame_rx, frame_tx);

        // output frame
        for(int i=0; i<FRAME_SHIFT; i++){
            echo_buf_ab[ping_pong][i*2+1] = (int32_t)frame_tx[i];
        }
        // shift frame
        for(int i=0; i<FRAME_LEN-FRAME_SHIFT; i++){
            frame_tx[i] = frame_tx[FRAME_SHIFT+i];
        }
        for(int i=FRAME_LEN-FRAME_SHIFT; i<FRAME_LEN; i++){
            frame_tx[i] = 0;
        }

    }

	return 0;
}
