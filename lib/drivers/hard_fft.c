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
#include <stddef.h>
#include <stdint.h>
#include "encoding.h"
#include "hard_fft.h"
#include "platform.h"
#include "syscalls.h"
#include "sysctl.h"

#define FFT_RESULT_ADDR (0x42100000 + 0x2000)
volatile uint64_t* fft_result = (volatile uint64_t*)FFT_RESULT_ADDR;

volatile fft_t* const fft = (volatile fft_t*)FFT_BASE_ADDR;

int fft_init(uint8_t point, uint8_t mode, uint16_t shift, uint8_t is_dma, uint8_t input_mode, uint8_t data_mode)
{
    fft->fft_ctrl.fft_point = point; /* 0:512, 1:256, 2:128, 3:64 */
    fft->fft_ctrl.fft_mode = mode;   /* 1: fft, 0: ifft */
    fft->fft_ctrl.fft_shift = shift;
    fft->fft_ctrl.dma_send = is_dma;
    fft->fft_ctrl.fft_enable = 1;
    fft->fft_ctrl.fft_input_mode = input_mode;
    fft->fft_ctrl.fft_data_mode = data_mode;
    return 0;
}

void fft_reset(void)
{
    fft->fifo_ctrl.resp_fifo_flush_n = 0;
    fft->fifo_ctrl.cmd_fifo_flush_n = 0;
    fft->fifo_ctrl.gs_fifo_flush_n = 0;
}

void fft_enable_int(void)
{
    fft->intr_mask.fft_done_mask = 1;
}

void fft_input_data(float* x, float* y, uint8_t point)
{
    uint16_t point_num = 0;
    uint16_t i;
    fft_data input_data;

    if (point == 0)
        point_num = 512;
    else if (point == 1)
        point_num = 256;
    else if (point == 2)
        point_num = 128;
    else if (point == 3)
        point_num = 64;
    point_num = point_num / 2; /* one time send two data */

    for (i = 0; i < point_num; i++)
    {
        input_data.R1 = (int16_t)(x[2 * i] * 32);
        input_data.I1 = (int16_t)(y[2 * i] * 32);
        input_data.R2 = (int16_t)(x[2 * i + 1] * 32);
        input_data.I2 = (int16_t)(y[2 * i + 1] * 32);

        fft->fft_input_fifo.fft_input_fifo = *(uint64_t*)&input_data;
        printf("%d, %d\n", input_data.R1, input_data.I1);
        printf("%d, %d\n", input_data.R2, input_data.I2);
    }
}

void fft_input_intdata(int16_t* data, uint8_t point)
{
    uint16_t point_num = 0;
    uint16_t i;
    fft_data input_data;

    if (point == 0)
        point_num = 512;
    else if (point == 1)
        point_num = 256;
    else if (point == 2)
        point_num = 128;
    else if (point == 3)
        point_num = 64;
    point_num = point_num / 2; /* one time send two data */

    for (i = 0; i < point_num; i++)
    {
        input_data.R1 = data[2 * i];
        input_data.I1 = 0;
        input_data.R2 = data[2 * i + 1];
        input_data.I2 = 0;

        fft->fft_input_fifo.fft_input_fifo = *(uint64_t*)&input_data;
    }
}

uint8_t fft_get_finish_flag(void)
{
    return (uint8_t)fft->fft_status.fft_done_status & 0x01;
}

void FixToDou(float* fData, uint32_t u32Data)
{
    if (u32Data & 0x8000)
        *fData = -((float)(u32Data & 0x7fff)) / 32;
    else
        *fData = ((float)u32Data) / 32;
}

void fft_get_result(float* x, float* y, uint8_t point)
{
    uint64_t u64Data;
    uint16_t point_num = 0;
    uint16_t i;
    fft_data output_data;

    if (point == 0)
        point_num = 512;
    else if (point == 1)
        point_num = 256;
    else if (point == 2)
        point_num = 128;
    else if (point == 3)
        point_num = 64;
    point_num = point_num / 2;

    for (i = 0; i < point_num; i++)
    {
        u64Data = fft_result[i]; /*fft->fft_output_fifo.fft_output_fifo;*/

        output_data = *(fft_data*)&u64Data;

        x[2 * i] = ((float)output_data.R1) / 32;
        y[2 * i] = ((float)output_data.I1) / 32;
        x[2 * i + 1] = ((float)output_data.R2) / 32;
        y[2 * i + 1] = ((float)output_data.I2) / 32;
    }
}

