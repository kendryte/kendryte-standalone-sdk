/*
 * main_wpe.c
 *
 *  Created on: 2018��9��5��
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
#include "../inc/kal_wpe.h"

#define FRAME_LEN  FDNDLP_K
#define DMA_CH_I2S_RX DMAC_CHANNEL0
#define DMA_CH_I2S_TX DMAC_CHANNEL1
#define DMA_CH_FFT_RX DMAC_CHANNEL2
#define DMA_CH_FFT_TX DMAC_CHANNEL3

uint32_t mic_buf[FRAME_LEN * 2 * 2]; 
uint32_t echo_buf[FRAME_LEN * 2 * 2];
fft_data_t fft_buf_tx[FRAME_LEN];
fft_data_t fft_buf_rx[FRAME_LEN];

void io_mux_init(){

    fpioa_set_function(36, FUNC_I2S1_IN_D0);
    fpioa_set_function(37, FUNC_I2S1_WS);
    fpioa_set_function(38, FUNC_I2S1_SCLK);

    fpioa_set_function(33, FUNC_I2S2_OUT_D1);
    fpioa_set_function(35, FUNC_I2S2_SCLK);
    fpioa_set_function(34, FUNC_I2S2_WS);
}

int init_system(){
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
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



    return 0;
}

int main(void)
{
    INT16 FreqDataRe[NRE][FDNDLP_MIC];
	INT16 FreqDataIm[NRE][FDNDLP_MIC];
    // FILE *FileId;
    // char ChIn[40];
    // UINT32 iRe, iMic, iFram;
    KAL_BUF_STR *KalBufStr;

    // read freq data
    //FileId = fopen("wav_file_in_fxp.txt","r");
    // for (iMic = 0; iMic < FDNDLP_MIC; ++iMic)
    // {
    //     for (iRe = 0; iRe < NRE; ++iRe)
    //     {
    //         for (iFram = 0; iFram < FDNDLP_FRAME; ++iFram)
    //         {
    //             //fgets(ChIn,40,FileId);
    //             FreqDataRe[iFram][iRe][iMic] = atoi(ChIn);
    //         }
    //     }
    //     for (iRe = 0; iRe < NRE; ++iRe)
    //     {
    //         for (iFram = 0; iFram < FDNDLP_FRAME; ++iFram)
    //         {
    //             //fgets(ChIn,40,FileId);
    //             FreqDataIm[iFram][iRe][iMic] = atoi(ChIn);
    //         }
    //     }
    // }
    //fclose(FileId);

    init_system();
    // init state
    KalBufStr = kal_state_init();


    // // Subband NDLP
    // for ( iFram =  0; iFram < FDNDLP_FRAME ; iFram++ )
    // {
    //     kalman_wpe(FreqDataRe[iFram], FreqDataIm[iFram], KalBufStr);
    // }

    int ping_pong=0;
    while (1)
    {
        ping_pong = 1-ping_pong;
        i2s_receive_data_dma(I2S_DEVICE_1, &mic_buf[(1-ping_pong)*FRAME_LEN*2], FRAME_LEN * 2, DMA_CH_I2S_RX);
        i2s_send_data_dma(I2S_DEVICE_2, &echo_buf[(1-ping_pong)*FRAME_LEN*2], FRAME_LEN * 2, DMA_CH_I2S_TX);

        for(int i=0; i<FRAME_LEN/2; i++){
            uint32_t idx = ping_pong*FRAME_LEN*2 + i*4 + 1;
            fft_buf_tx[i].I1 = 0;
            fft_buf_tx[i].R1 = (int16_t)mic_buf[idx];
            fft_buf_tx[i].I2 = 0;
            fft_buf_tx[i].R2 = (int16_t)mic_buf[idx+2];
        }
        fft_complex_uint16_dma(DMA_CH_FFT_TX, DMA_CH_FFT_RX, 0xaa, FFT_DIR_FORWARD, fft_buf_tx, 512, fft_buf_rx);
        for(int i=0; i<128; i++){
            FreqDataIm[i*2][0] = fft_buf_rx[i].I1;
            FreqDataRe[i*2][0] = fft_buf_rx[i].R1;
            FreqDataIm[i*2+1][0] = fft_buf_rx[i].I2;
            FreqDataRe[i*2+1][0] = fft_buf_rx[i].R2;
        }
        FreqDataIm[256][0] = fft_buf_rx[128].I1;
        FreqDataRe[256][0] = fft_buf_rx[128].R1;

        //kalman_wpe(*FreqDataRe, *FreqDataIm, KalBufStr);

        for(int i=0; i<128; i++){
            fft_buf_tx[i].I1 = FreqDataIm[i*2][0];
            fft_buf_tx[i].R1 = FreqDataRe[i*2][0];
            fft_buf_tx[i].I2 = FreqDataIm[i*2+1][0];
            fft_buf_tx[i].R2 = FreqDataRe[i*2+1][0];
            fft_buf_tx[255-i].I1 = -FreqDataIm[i*2+2][0];
            fft_buf_tx[255-i].R1 = FreqDataRe[i*2+2][0];
            fft_buf_tx[255-i].I2 = -FreqDataIm[i*2+1][0];
            fft_buf_tx[255-i].R2 = FreqDataRe[i*2+1][0];
        }

        // for(int i=0; i<FRAME_LEN/2; i++){
        //     fft_buf_tx[i] = fft_buf_rx[i];
        // }
        fft_complex_uint16_dma(DMA_CH_FFT_TX, DMA_CH_FFT_RX, 0xaa, FFT_DIR_BACKWARD, fft_buf_tx, 512, fft_buf_rx);
        for(int i=0; i<FRAME_LEN/2; i++){
            uint32_t idx = ping_pong*FRAME_LEN*2 + i*4 + 1;
            echo_buf[idx] = (int32_t)fft_buf_rx[i].R1;
            echo_buf[idx+2] = (int32_t)fft_buf_rx[i].R2;
        }
    }

    // destroy state
    kal_state_destroy(KalBufStr);

	return 0;
}
