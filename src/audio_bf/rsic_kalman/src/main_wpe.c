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
#include "../inc/kal_wpe.h"

#define FRAME_LEN  FDNDLP_K

uint32_t mic_buf[FRAME_LEN * 2 * 2]; 
uint64_t fft_buf[FRAME_LEN];
uint64_t ifft_buf[FRAME_LEN];
uint32_t echo_buf[FRAME_LEN * 2 * 2];

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
    INT16 (*FreqDataRe)[NRE][FDNDLP_MIC];
	INT16 (*FreqDataIm)[NRE][FDNDLP_MIC];
    FILE *FileId;
    char ChIn[40];
    UINT32 iRe, iMic, iFram;
    KAL_BUF_STR *KalBufStr;
    
    FreqDataRe = (INT16 (*)[NRE][FDNDLP_MIC])malloc(sizeof(INT16)*NRE*FDNDLP_MIC*FDNDLP_FRAME);
	FreqDataIm = (INT16 (*)[NRE][FDNDLP_MIC])malloc(sizeof(INT16)*NRE*FDNDLP_MIC*FDNDLP_FRAME);

    // read freq data
    //FileId = fopen("wav_file_in_fxp.txt","r");
    for (iMic = 0; iMic < FDNDLP_MIC; ++iMic)
    {
        for (iRe = 0; iRe < NRE; ++iRe)
        {
            for (iFram = 0; iFram < FDNDLP_FRAME; ++iFram)
            {
                //fgets(ChIn,40,FileId);
                FreqDataRe[iFram][iRe][iMic] = atoi(ChIn);
            }
        }
        for (iRe = 0; iRe < NRE; ++iRe)
        {
            for (iFram = 0; iFram < FDNDLP_FRAME; ++iFram)
            {
                //fgets(ChIn,40,FileId);
                FreqDataIm[iFram][iRe][iMic] = atoi(ChIn);
            }
        }
    }
    //fclose(FileId);

    // init state
    KalBufStr = kal_state_init();


    // Subband NDLP
    for ( iFram =  0; iFram < FDNDLP_FRAME ; iFram++ )
    {
        kalman_wpe(FreqDataRe[iFram], FreqDataIm[iFram], KalBufStr);
    }

    while (0)
    {
        i2s_send_data_dma(I2S_DEVICE_2, &mic_buf[FRAME_LEN*2], FRAME_LEN*2, DMAC_CHANNEL0);
        i2s_receive_data_dma(I2S_DEVICE_1, &echo_buf[FRAME_LEN*2], FRAME_LEN * 2, DMAC_CHANNEL1);
    }

    // destroy state
    kal_state_destroy(KalBufStr);
    // free
    free(FreqDataRe);
    free(FreqDataIm);

	return 0;
}
