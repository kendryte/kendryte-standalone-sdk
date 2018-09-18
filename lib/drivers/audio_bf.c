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
#include "audio_bf.h"
#include "syscalls.h"
#include "sysctl.h"

#define BEAFORMING_BASE_ADDR (0x50250200U)

volatile audio_bf_reg_t* const audio_bf = (volatile audio_bf_reg_t*)BEAFORMING_BASE_ADDR;

/**
 * Voice strength average value right shift factor.  When performing sound direction detect,
 * the average value of samples from different channels is required, this right shift factor
 * is used to perform division.
 * 0x0: no right shift;
 * 0x1: right shift by 1-bit;
 *  . . . . . .
 * 0xF: right shift by 14-bit.
*/
void audio_bf_set_audio_gain(uint16_t gain)
{
    audio_bf_ch_cfg_t ch_cfg = audio_bf->bf_ch_cfg_reg;

    ch_cfg.we_bf_target_dir = 0;
    ch_cfg.we_bf_sound_ch_en = 0;
    ch_cfg.we_data_src_mode = 0;
    ch_cfg.we_audio_gain = 1;
    ch_cfg.audio_gain = gain;
    audio_bf->bf_ch_cfg_reg = ch_cfg;
}

void audio_bf_set_smpl_shift(uint8_t smpl_shift)
{
    audio_bf_dwsz_cfg_t tmp = audio_bf->bf_dwsz_cfg_reg;

    tmp.smpl_shift_bits = smpl_shift;
    audio_bf->bf_dwsz_cfg_reg = tmp;
}

uint8_t audio_bf_get_smpl_shift(void)
{
    audio_bf_dwsz_cfg_t tmp = audio_bf->bf_dwsz_cfg_reg;

    return tmp.smpl_shift_bits;
}

/**
 * BF unit sound channel enable control bits.  Bit 'x' corresponds to enable bit for sound
 * channel 'x' (x = 0, 1, 2, . . ., 7).  BF sound channels are related with I2S host RX channels.
 * BF sound channel 0/1 correspond to the left/right channel of I2S RX0; BF channel 2/3 correspond
 * to left/right channels of I2S RX1; and things like that.  Software write '1' to enable a sound
 * channel and hardware automatically clear the bit after the sample buffers used for direction
 * searching is filled full.
 * 0x1: writing '1' to enable the corresponding BF sound channel.
 */
void audio_bf_set_channel_enabled(uint8_t channel_bit)
{
    audio_bf_ch_cfg_t ch_cfg;

    ch_cfg.we_audio_gain = 0;
    ch_cfg.we_bf_target_dir = 0;
    ch_cfg.we_bf_sound_ch_en = 1;
    ch_cfg.bf_sound_ch_en = channel_bit;
    audio_bf->bf_ch_cfg_reg = ch_cfg;
}

/**
 * BF unit sound channel enable control bits.  Bit 'x' corresponds to enable bit for sound
 * channel 'x' (x = 0, 1, 2, . . ., 7).  BF sound channels are related with I2S host RX channels.
 * BF sound channel 0/1 correspond to the left/right channel of I2S RX0; BF channel 2/3 correspond
 * to left/right channels of I2S RX1; and things like that.  Software write '1' to enable a sound
 * channel and hardware automatically clear the bit after the sample buffers used for direction
 * searching is filled full.
 * 0x1: writing '1' to enable the corresponding BF sound channel.
 */
void audio_bf_channel_enable(uint8_t channel_bit)
{
    audio_bf_ch_cfg_t ch_cfg = audio_bf->bf_ch_cfg_reg;

    ch_cfg.we_audio_gain = 0;
    ch_cfg.we_bf_target_dir = 0;
    ch_cfg.we_data_src_mode = 0;
    ch_cfg.we_bf_sound_ch_en = 1;
    ch_cfg.bf_sound_ch_en = channel_bit;
    audio_bf->bf_ch_cfg_reg = ch_cfg;
}

/**
 * audio data source configure parameter.  This parameter controls where the audio data source comes from.
 * 0x0: audio data directly sourcing from audio_bf internal buffer;
 * 0x1: audio data sourcing from FFT result buffer.
 */
void audio_bf_set_src_mode(uint8_t src_mode)
{
    audio_bf_ch_cfg_t ch_cfg = audio_bf->bf_ch_cfg_reg;

    ch_cfg.we_audio_gain = 0;
    ch_cfg.we_bf_target_dir = 0;
    ch_cfg.we_bf_sound_ch_en = 0;
    ch_cfg.we_data_src_mode = 1;
    ch_cfg.data_src_mode = src_mode;
    audio_bf->bf_ch_cfg_reg = ch_cfg;
}

/**
 * I2S host beam-forming direction sample ibuffer read index configure register
 */
void audio_bf_set_direction_delay(uint8_t dir_num, uint8_t* dir_bidx)
{
    audio_bf->bf_dir_bidx[dir_num][0] = (audio_bf_dir_bidx_t){
        .dir_rd_idx0 = dir_bidx[0],
        .dir_rd_idx1 = dir_bidx[1],
        .dir_rd_idx2 = dir_bidx[2],
        .dir_rd_idx3 = dir_bidx[3]};
    audio_bf->bf_dir_bidx[dir_num][1] = (audio_bf_dir_bidx_t){
        .dir_rd_idx0 = dir_bidx[4],
        .dir_rd_idx1 = dir_bidx[5],
        .dir_rd_idx2 = dir_bidx[6],
        .dir_rd_idx3 = dir_bidx[7]};
}

/**
 * Sound direction searching enable bit.  Software writes '1' to start sound direction searching function.
 * When all the sound sample buffers are filled full, this bit is cleared by hardware (this sample buffers
 * are used for direction detect only).
 * 0x1: enable direction searching.
 */
void audio_bf_dir_enable(void)
{
    audio_bf_ctl_t bf_en_tmp = audio_bf->bf_ctl_reg;

    bf_en_tmp.we_bf_dir_search_en = 1;
    bf_en_tmp.bf_dir_search_en = 1;
    audio_bf->bf_ctl_reg = bf_en_tmp;
}

void audio_bf_dir_reset(void)
{
    audio_bf_ctl_t bf_en_tmp = audio_bf->bf_ctl_reg;

    bf_en_tmp.we_search_path_rst = 1;
    bf_en_tmp.search_path_reset = 1;
    audio_bf->bf_ctl_reg = bf_en_tmp;
}

/**
 * Valid voice sample stream generation enable bit.  After sound direction searching is done, software can
 * configure this bit to generate a stream of voice samples for voice recognition.
 * 0x1: enable output of voice sample stream.
 * 0x0: stop the voice samlpe stream output.
 */
void audio_bf_voc_enable(uint8_t enable_flag)
{
    audio_bf_ctl_t bf_en_tmp = audio_bf->bf_ctl_reg;

    bf_en_tmp.we_bf_stream_gen = 1;
    bf_en_tmp.bf_stream_gen_en = enable_flag;
    audio_bf->bf_ctl_reg = bf_en_tmp;
}

void audio_bf_voc_reset(void)
{
    audio_bf_ctl_t bf_en_tmp = audio_bf->bf_ctl_reg;

    bf_en_tmp.we_voice_gen_path_rst = 1;
    bf_en_tmp.voice_gen_path_reset = 1;
    audio_bf->bf_ctl_reg = bf_en_tmp;
}

/**
 * Target direction select for valid voice output.  When the source voice direaction searching
 * is done, software can use this field to select one from 16 sound directions for the following
 * voice recognition
 * 0x0: select sound direction 0;   0x1: select sound direction 1;
 *  . . . . . .
 * 0xF: select sound direction 15.
*/
void audio_bf_voc_set_direction(en_bf_dir direction)
{
    audio_bf_ch_cfg_t ch_cfg = audio_bf->bf_ch_cfg_reg;

    ch_cfg.we_bf_sound_ch_en = 0;
    ch_cfg.we_audio_gain = 0;
    ch_cfg.we_data_src_mode = 0;
    ch_cfg.we_bf_target_dir = 1;
    ch_cfg.bf_target_dir = direction;
    audio_bf->bf_ch_cfg_reg = ch_cfg;

    audio_bf_ctl_t bf_en_tmp = audio_bf->bf_ctl_reg;

    bf_en_tmp.we_update_voice_dir = 1;
    bf_en_tmp.update_voice_dir = 1;
    audio_bf->bf_ctl_reg = bf_en_tmp;
}

/**
 * I2S host beam-forming Filter FIR16 Coefficient Register
 */
void audio_bf_dir_set_prev_fir(uint16_t* fir_coef)
{
    uint8_t i = 0;

    for (i = 0; i < 9; i++)
    {
        audio_bf->bf_pre_fir0_coef[i] = (audio_bf_fir_coef_t){
            .fir_tap0 = fir_coef[i * 2],
            .fir_tap1 = i == 8 ? 0 : fir_coef[i * 2 + 1]};
    }
}

void audio_bf_dir_set_post_fir(uint16_t* fir_coef)
{
    uint8_t i = 0;

    for (i = 0; i < 9; i++)
    {
        audio_bf->bf_post_fir0_coef[i] = (audio_bf_fir_coef_t){
            .fir_tap0 = fir_coef[i * 2],
            .fir_tap1 = i == 8 ? 0 : fir_coef[i * 2 + 1]};
    }
}

void audio_bf_voc_set_prev_fir(uint16_t* fir_coef)
{
    uint8_t i = 0;

    for (i = 0; i < 9; i++)
    {
        audio_bf->bf_pre_fir1_coef[i] = (audio_bf_fir_coef_t){
            .fir_tap0 = fir_coef[i * 2],
            .fir_tap1 = i == 8 ? 0 : fir_coef[i * 2 + 1]};
    }
}

void audio_bf_voc_set_post_fir(uint16_t* fir_coef)
{
    uint8_t i = 0;

    for (i = 0; i < 9; i++)
    {
        audio_bf->bf_post_fir1_coef[i] = (audio_bf_fir_coef_t){
            .fir_tap0 = fir_coef[i * 2],
            .fir_tap1 = i == 8 ? 0 : fir_coef[i * 2 + 1]};
    }
}

void audio_bf_set_fft_shift_factor(uint8_t enable_flag, uint16_t shift_factor)
{
    audio_bf->bf_fft_cfg_reg = (audio_bf_fft_cfg_t){
        .fft_enable = enable_flag,
        .fft_shift_factor = shift_factor};

    audio_bf_ch_cfg_t ch_cfg = audio_bf->bf_ch_cfg_reg;

    ch_cfg.we_data_src_mode = 1;
    ch_cfg.data_src_mode = enable_flag;
    audio_bf->bf_ch_cfg_reg = ch_cfg;
}

void audio_bf_dir_set_down_size(uint8_t dir_dwn_size)
{
    audio_bf_dwsz_cfg_t tmp = audio_bf->bf_dwsz_cfg_reg;

    tmp.dir_dwn_siz_rate = dir_dwn_size;
    audio_bf->bf_dwsz_cfg_reg = tmp;
}

void audio_bf_dir_set_interrupt_mask(uint8_t dir_int_mask)
{
    audio_bf_int_mask_t tmp = audio_bf->bf_int_mask_reg;

    tmp.dir_data_rdy_msk = dir_int_mask;
    audio_bf->bf_int_mask_reg = tmp;
}

void audio_bf_voc_set_down_size(uint8_t voc_dwn_size)
{
    audio_bf_dwsz_cfg_t tmp = audio_bf->bf_dwsz_cfg_reg;

    tmp.voc_dwn_siz_rate = voc_dwn_size;
    audio_bf->bf_dwsz_cfg_reg = tmp;
}

void audio_bf_voc_set_interrupt_mask(uint8_t voc_int_mask)
{
    audio_bf_int_mask_t tmp = audio_bf->bf_int_mask_reg;

    tmp.voc_buf_rdy_msk = voc_int_mask;
    audio_bf->bf_int_mask_reg = tmp;
}

void audio_bf_set_down_size(uint8_t dir_dwn_size, uint8_t voc_dwn_size)
{
    audio_bf_dwsz_cfg_t tmp = audio_bf->bf_dwsz_cfg_reg;

    tmp.dir_dwn_siz_rate = dir_dwn_size;
    tmp.voc_dwn_siz_rate = voc_dwn_size;
    audio_bf->bf_dwsz_cfg_reg = tmp;
}

void audio_bf_set_interrupt_mask(uint8_t dir_int_mask, uint8_t voc_int_mask)
{
    audio_bf->bf_int_mask_reg = (audio_bf_int_mask_t){
        .dir_data_rdy_msk = dir_int_mask,
        .voc_buf_rdy_msk = voc_int_mask};
}

void audio_bf_dir_clear_int_state(void)
{
    audio_bf->bf_int_stat_reg = (audio_bf_int_stat_t){
        .dir_search_data_rdy = 1};
}

void audio_bf_voc_clear_int_state(void)
{
    audio_bf->bf_int_stat_reg = (audio_bf_int_stat_t){
        .voc_buf_data_rdy = 1};
}

void audio_bf_voc_reset_saturation_counter(void)
{
    audio_bf->saturation_counter = 1 << 31;
}

/* heigh 16 bit is counter, low 16 bit is total.*/
uint32_t audio_bf_voc_get_saturation_counter(void)
{
    return audio_bf->saturation_counter;
}

void audio_bf_voc_set_saturation_limit(uint16_t upper, uint16_t bottom)
{
    audio_bf->saturation_limits = (uint32_t)bottom << 16 | upper;
}

/* heigh 16 bit is counter, low 16 bit is total.*/
uint32_t audio_bf_voc_get_saturation_limit(void)
{
    return audio_bf->saturation_limits;
}

static void print_fir(const char* member_name, volatile audio_bf_fir_coef_t* pfir)
{
    int i;
    printf("  for(int i = 0; i < 9; i++){\n");
    for (i = 0; i < 9; i++)
    {
        audio_bf_fir_coef_t fir = pfir[i];

        printf("    audio_bf->%s[%d] = (audio_bf_fir_coef_t){\n", member_name, i);
        printf("      .fir_tap0 = 0x%x,\n", fir.fir_tap0);
        printf("      .fir_tap1 = 0x%x\n", fir.fir_tap1);
        printf("    };\n");
    }
    printf("  }\n");
}

void audio_bf_print_setting(void)
{
    int i;
    printf("void audio_bf_setting(void) {\n");
    audio_bf_ch_cfg_t bf_ch_cfg_reg = audio_bf->bf_ch_cfg_reg;

    printf("  audio_bf->bf_ch_cfg_reg = (audio_bf_ch_cfg_t){\n");
    printf("    .we_audio_gain = 1, .we_bf_target_dir = 1, .we_bf_sound_ch_en = 1,\n");
    printf("    .audio_gain = 0x%x, .bf_target_dir = %d, .bf_sound_ch_en = %d, .data_src_mode = %d\n",
        bf_ch_cfg_reg.audio_gain, bf_ch_cfg_reg.bf_target_dir, bf_ch_cfg_reg.bf_sound_ch_en, bf_ch_cfg_reg.data_src_mode);
    printf("  };\n");

    audio_bf_ctl_t bf_ctl_reg = audio_bf->bf_ctl_reg;

    printf("  audio_bf->bf_ctl_reg = (audio_bf_ctl_t){\n");
    printf("    .we_bf_stream_gen = 1, .we_bf_dir_search_en = 1,\n");
    printf("    .bf_stream_gen_en = %d, .bf_dir_search_en = %d\n",
        bf_ctl_reg.bf_stream_gen_en, bf_ctl_reg.bf_dir_search_en);
    printf("  };\n");

    printf("  for(int i = 0; i < 16; i++){\n");
    for (i = 0; i < 16; i++)
    {
        audio_bf_dir_bidx_t bidx0 = audio_bf->bf_dir_bidx[i][0];
        audio_bf_dir_bidx_t bidx1 = audio_bf->bf_dir_bidx[i][1];

        printf("    audio_bf->bf_dir_bidx[%d][0] = (audio_bf_dir_bidx_t){\n", i);
        printf("      .dir_rd_idx0 = 0x%x,\n", bidx0.dir_rd_idx0);
        printf("      .dir_rd_idx1 = 0x%x,\n", bidx0.dir_rd_idx1);
        printf("      .dir_rd_idx2 = 0x%x,\n", bidx0.dir_rd_idx2);
        printf("      .dir_rd_idx3 = 0x%x\n", bidx0.dir_rd_idx3);
        printf("    };\n");
        printf("    audio_bf->bf_dir_bidx[%d][1] = (audio_bf_dir_bidx_t){\n", i);
        printf("      .dir_rd_idx0 = 0x%x,\n", bidx1.dir_rd_idx0);
        printf("      .dir_rd_idx1 = 0x%x,\n", bidx1.dir_rd_idx1);
        printf("      .dir_rd_idx2 = 0x%x,\n", bidx1.dir_rd_idx2);
        printf("      .dir_rd_idx3 = 0x%x\n", bidx1.dir_rd_idx3);
        printf("    };\n");
    }
    printf("  }\n");

    print_fir("bf_pre_fir0_coef", audio_bf->bf_pre_fir0_coef);
    print_fir("bf_post_fir0_coef", audio_bf->bf_post_fir0_coef);
    print_fir("bf_pre_fir1_coef", audio_bf->bf_pre_fir1_coef);
    print_fir("bf_post_fir1_coef", audio_bf->bf_post_fir1_coef);

    audio_bf_dwsz_cfg_t bf_dwsz_cfg_reg = audio_bf->bf_dwsz_cfg_reg;

    printf("  audio_bf->bf_dwsz_cfg_reg = (audio_bf_dwsz_cfg_t){\n");
    printf("    .dir_dwn_siz_rate = %d, .voc_dwn_siz_rate = %d\n",
        bf_dwsz_cfg_reg.dir_dwn_siz_rate, bf_dwsz_cfg_reg.voc_dwn_siz_rate);
    printf("  };\n");

    audio_bf_fft_cfg_t bf_fft_cfg_reg = audio_bf->bf_fft_cfg_reg;

    printf("  audio_bf->bf_fft_cfg_reg = (audio_bf_fft_cfg_t){\n");
    printf("    .fft_enable = %d, .fft_shift_factor = 0x%x\n",
        bf_fft_cfg_reg.fft_enable, bf_fft_cfg_reg.fft_shift_factor);
    printf("  };\n");

    audio_bf_int_mask_t bf_int_mask_reg = audio_bf->bf_int_mask_reg;

    printf("  audio_bf->bf_int_mask_reg = (audio_bf_int_mask_t){\n");
    printf("    .dir_data_rdy_msk = %d, .voc_buf_rdy_msk = %d\n",
        bf_int_mask_reg.dir_data_rdy_msk, bf_int_mask_reg.voc_buf_rdy_msk);
    printf("  };\n");

    printf("}\n");
}
