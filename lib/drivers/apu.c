#include <stddef.h>
#include <stdint.h>
#include <atomic.h>
#include <math.h>
#include <i2s.h>
#include <fpioa.h>
#include <encoding.h>
#include <syscalls.h>
#include <sysctl.h>
#include <printf.h>
#include "apu.h"

#define BEAFORMING_BASE_ADDR    (0x50250200U)

volatile struct apu_reg_t *const apu = (volatile struct apu_reg_t *)BEAFORMING_BASE_ADDR;

semaphore_t apu_dir_ready = {0};
semaphore_t apu_voc_ready = {0};
volatile void* apu_dir_buffer = NULL;
volatile void* apu_voc_buffer = NULL;
volatile int apu_using_fft = 0;
volatile dmac_channel_number_t apu_dma_dir_ch;
volatile dmac_channel_number_t apu_dma_voc_ch;

/*
 *
Voice strength average value right shift factor.  When performing sound direction detect,
the average value of samples from different channels is required, this right shift factor
is used to perform division.
0x0: no right shift;               0x1: right shift by 1-bit;
 . . . . . .
0xF: right shift by 14-bit.
*/
void apu_set_audio_gain(uint16_t gain)
{
    struct apu_ch_cfg_t ch_cfg = apu->bf_ch_cfg_reg;

    ch_cfg.we_bf_target_dir = 0;
    ch_cfg.we_bf_sound_ch_en = 0;
    ch_cfg.we_data_src_mode = 0;
    ch_cfg.we_audio_gain = 1;
    ch_cfg.audio_gain = gain;
    apu->bf_ch_cfg_reg = ch_cfg;
}

// set sampling shift
void apu_set_smpl_shift(uint8_t smpl_shift)
{
    struct apu_dwsz_cfg_t tmp = apu->bf_dwsz_cfg_reg;

    tmp.smpl_shift_bits = smpl_shift;
    apu->bf_dwsz_cfg_reg = tmp;
}
// get sampling shift
uint8_t apu_get_smpl_shift(void)
{
    struct apu_dwsz_cfg_t tmp = apu->bf_dwsz_cfg_reg;

    return tmp.smpl_shift_bits;
}


/*
 *
 * BF unit sound channel enable control bits.  Bit 'x' corresponds to enable bit for sound
 * channel 'x' (x = 0, 1, 2, . . ., 7).  BF sound channels are related with I2S host RX channels.
 * BF sound channel 0/1 correspond to the left/right channel of I2S RX0; BF channel 2/3 correspond
 * to left/right channels of I2S RX1; and things like that.  Software write '1' to enable a sound
 * channel and hardware automatically clear the bit after the sample buffers used for direction
 * searching is filled full.
 * 0x1: writing '1' to enable the corresponding BF sound channel.
 */
void apu_set_channel_enabled(uint8_t channel_bit)
{
    struct apu_ch_cfg_t ch_cfg;

    ch_cfg.we_audio_gain = 0;
    ch_cfg.we_bf_target_dir = 0;
    ch_cfg.we_bf_sound_ch_en = 1;
    ch_cfg.bf_sound_ch_en = channel_bit;
    apu->bf_ch_cfg_reg = ch_cfg;
}

/*
 *
 * BF unit sound channel enable control bits.  Bit 'x' corresponds to enable bit for sound
 * channel 'x' (x = 0, 1, 2, . . ., 7).  BF sound channels are related with I2S host RX channels.
 * BF sound channel 0/1 correspond to the left/right channel of I2S RX0; BF channel 2/3 correspond
 * to left/right channels of I2S RX1; and things like that.  Software write '1' to enable a sound
 * channel and hardware automatically clear the bit after the sample buffers used for direction
 * searching is filled full.
 * 0x1: writing '1' to enable the corresponding BF sound channel.
 */
void apu_channel_enable(uint8_t channel_bit)
{
    struct apu_ch_cfg_t ch_cfg = apu->bf_ch_cfg_reg;

    ch_cfg.we_audio_gain = 0;
    ch_cfg.we_bf_target_dir = 0;
    ch_cfg.we_data_src_mode = 0;
    ch_cfg.we_bf_sound_ch_en = 1;
    ch_cfg.bf_sound_ch_en = channel_bit;
    apu->bf_ch_cfg_reg = ch_cfg;
}
/**
 *  audio data source configure parameter.  This parameter controls where the audio data source comes from.
 *  0x0: audio data directly sourcing from apu internal buffer;
 *  0x1: audio data sourcing from FFT result buffer.
 */
void apu_set_src_mode(uint8_t src_mode)
{
    struct apu_ch_cfg_t ch_cfg = apu->bf_ch_cfg_reg;

    ch_cfg.we_audio_gain = 0;
    ch_cfg.we_bf_target_dir = 0;
    ch_cfg.we_bf_sound_ch_en = 0;
    ch_cfg.we_data_src_mode = 1;
    ch_cfg.data_src_mode = src_mode;
    apu->bf_ch_cfg_reg = ch_cfg;
}

/*
 * I2S host beam-forming direction sample ibuffer read index configure register
 */
void apu_set_direction_delay(uint8_t dir_num, uint8_t *dir_bidx)
{
    apu->bf_dir_bidx[dir_num][0] =
    (struct apu_dir_bidx_t){
        .dir_rd_idx0 = dir_bidx[0],
        .dir_rd_idx1 = dir_bidx[1],
        .dir_rd_idx2 = dir_bidx[2],
        .dir_rd_idx3 = dir_bidx[3]
    };
    apu->bf_dir_bidx[dir_num][1] =
    (struct apu_dir_bidx_t){
        .dir_rd_idx0 = dir_bidx[4],
        .dir_rd_idx1 = dir_bidx[5],
        .dir_rd_idx2 = dir_bidx[6],
        .dir_rd_idx3 = dir_bidx[7]
    };
}
/*
 *
 S *ound direction searching enable bit.  Software writes '1' to start sound direction searching function.
 When all the sound sample buffers are filled full, this bit is cleared by hardware (this sample buffers
 are used for direction detect only).
 0x1: enable direction searching.
 */
void apu_dir_enable(void)
{
    struct apu_ctl_t bf_en_tmp = apu->bf_ctl_reg;

    bf_en_tmp.we_bf_dir_search_en = 1;
    bf_en_tmp.bf_dir_search_en = 1;
    apu->bf_ctl_reg = bf_en_tmp;
}
void apu_dir_reset(void)
{
    struct apu_ctl_t bf_en_tmp = apu->bf_ctl_reg;

    bf_en_tmp.we_search_path_rst = 1;
    bf_en_tmp.search_path_reset = 1;
    apu->bf_ctl_reg = bf_en_tmp;
}
/*
 *
 V *alid voice sample stream generation enable bit.  After sound direction searching is done, software can
 configure this bit to generate a stream of voice samples for voice recognition.
 0x1: enable output of voice sample stream.
 0x0: stop the voice samlpe stream output.
 */
void apu_voc_enable(uint8_t enable_flag)
{
    struct apu_ctl_t bf_en_tmp = apu->bf_ctl_reg;

    bf_en_tmp.we_bf_stream_gen = 1;
    bf_en_tmp.bf_stream_gen_en = enable_flag;
    apu->bf_ctl_reg = bf_en_tmp;
}
void apu_voc_reset(void)
{
    struct apu_ctl_t bf_en_tmp = apu->bf_ctl_reg;

    bf_en_tmp.we_voice_gen_path_rst = 1;
    bf_en_tmp.voice_gen_path_reset = 1;
    apu->bf_ctl_reg = bf_en_tmp;
}

/*
 *
Target direction select for valid voice output.  When the source voice direaction searching
is done, software can use this field to select one from 16 sound directions for the following
voice recognition
0x0: select sound direction 0;   0x1: select sound direction 1;
 . . . . . .
0xF: select sound direction 15.
*/
void apu_voc_set_direction(enum en_bf_dir direction)
{
    struct apu_ch_cfg_t ch_cfg = apu->bf_ch_cfg_reg;

    ch_cfg.we_bf_sound_ch_en = 0;
    ch_cfg.we_audio_gain = 0;
    ch_cfg.we_data_src_mode = 0;
    ch_cfg.we_bf_target_dir = 1;
    ch_cfg.bf_target_dir = direction;
    apu->bf_ch_cfg_reg = ch_cfg;

    struct apu_ctl_t bf_en_tmp = apu->bf_ctl_reg;

    bf_en_tmp.we_update_voice_dir = 1;
    bf_en_tmp.update_voice_dir = 1;
    apu->bf_ctl_reg = bf_en_tmp;
}


/*
 *I2S host beam-forming Filter FIR16 Coefficient Register
 */
void apu_dir_set_prev_fir(uint16_t *fir_coef)
{
    uint8_t i = 0;

    for (i = 0; i < 9; i++) {
        apu->bf_pre_fir0_coef[i] =
        (struct apu_fir_coef_t){
            .fir_tap0 = fir_coef[i * 2],
            .fir_tap1 = i == 8 ? 0 : fir_coef[i * 2 + 1]
        };
    }
}
void apu_dir_set_post_fir(uint16_t *fir_coef)
{
    uint8_t i = 0;

    for (i = 0; i < 9; i++) {
        apu->bf_post_fir0_coef[i] =
        (struct apu_fir_coef_t){
            .fir_tap0 = fir_coef[i * 2],
            .fir_tap1 = i == 8 ? 0 : fir_coef[i * 2 + 1]
        };
    }
}
void apu_voc_set_prev_fir(uint16_t *fir_coef)
{
    uint8_t i = 0;

    for (i = 0; i < 9; i++) {
        apu->bf_pre_fir1_coef[i] =
        (struct apu_fir_coef_t){
            .fir_tap0 = fir_coef[i * 2],
            .fir_tap1 = i == 8 ? 0 : fir_coef[i * 2 + 1]
        };
    }
}
void apu_voc_set_post_fir(uint16_t *fir_coef)
{
    uint8_t i = 0;

    for (i = 0; i < 9; i++) {
        apu->bf_post_fir1_coef[i] =
        (struct apu_fir_coef_t){
            .fir_tap0 = fir_coef[i * 2],
            .fir_tap1 = i == 8 ? 0 : fir_coef[i * 2 + 1]
        };
    }
}

void apu_set_fft_shift_factor(uint8_t enable_flag, uint16_t shift_factor)
{
    apu->bf_fft_cfg_reg =
    (struct apu_fft_cfg_t){
        .fft_enable = enable_flag,
        .fft_shift_factor = shift_factor
    };

    struct apu_ch_cfg_t ch_cfg = apu->bf_ch_cfg_reg;

    ch_cfg.we_data_src_mode = 1;
    ch_cfg.data_src_mode = enable_flag;
    apu->bf_ch_cfg_reg = ch_cfg;
}

void apu_dir_set_down_size(uint8_t dir_dwn_size)
{
    struct apu_dwsz_cfg_t tmp = apu->bf_dwsz_cfg_reg;

    tmp.dir_dwn_siz_rate = dir_dwn_size;
    apu->bf_dwsz_cfg_reg = tmp;
}

void apu_dir_set_interrupt_mask(uint8_t dir_int_mask)
{
    struct apu_int_mask_t tmp = apu->bf_int_mask_reg;

    tmp.dir_data_rdy_msk = dir_int_mask;
    apu->bf_int_mask_reg = tmp;
}

void apu_voc_set_down_size(uint8_t voc_dwn_size)
{
    struct apu_dwsz_cfg_t tmp = apu->bf_dwsz_cfg_reg;

    tmp.voc_dwn_siz_rate = voc_dwn_size;
    apu->bf_dwsz_cfg_reg = tmp;
}
void apu_voc_set_interrupt_mask(uint8_t voc_int_mask)
{
    struct apu_int_mask_t tmp = apu->bf_int_mask_reg;

    tmp.voc_buf_rdy_msk = voc_int_mask;
    apu->bf_int_mask_reg = tmp;
}


void apu_set_down_size(uint8_t dir_dwn_size, uint8_t voc_dwn_size)
{
    struct apu_dwsz_cfg_t tmp = apu->bf_dwsz_cfg_reg;

    tmp.dir_dwn_siz_rate = dir_dwn_size;
    tmp.voc_dwn_siz_rate = voc_dwn_size;
    apu->bf_dwsz_cfg_reg = tmp;
}
void apu_set_interrupt_mask(uint8_t dir_int_mask, uint8_t voc_int_mask)
{
    apu->bf_int_mask_reg =
    (struct apu_int_mask_t){
        .dir_data_rdy_msk = dir_int_mask,
        .voc_buf_rdy_msk = voc_int_mask
    };
}

void apu_dir_clear_int_state(void)
{
    apu->bf_int_stat_reg =
    (struct apu_int_stat_t){
        .dir_search_data_rdy = 1
    };
}

void apu_voc_clear_int_state(void)
{
    apu->bf_int_stat_reg =
    (struct apu_int_stat_t){
        .voc_buf_data_rdy = 1
    };
}

// reset saturation_counter
void apu_voc_reset_saturation_counter(void)
{
    apu->saturation_counter = 1<<31;
}

// get saturation counter
// heigh 16 bit is counter, low 16 bit is total.
uint32_t apu_voc_get_saturation_counter(void)
{
    return apu->saturation_counter;
}

// set saturation limit
void apu_voc_set_saturation_limit(uint16_t upper, uint16_t bottom)
{
    apu->saturation_limits = (uint32_t)bottom<<16 | upper;
}

// get saturation limit
// heigh 16 bit is counter, low 16 bit is total.
uint32_t apu_voc_get_saturation_limit(void)
{
    return apu->saturation_limits;
}

static void print_fir(const char *member_name, volatile struct apu_fir_coef_t *pfir)
{
    printf("  for(int i = 0; i < 9; i++){\n");
    for (int i = 0; i < 9; i++) {
        struct apu_fir_coef_t fir = pfir[i];

        printf("    apu->%s[%d] = (struct apu_fir_coef_t){\n", member_name, i);
        printf("      .fir_tap0 = 0x%x,\n", fir.fir_tap0);
        printf("      .fir_tap1 = 0x%x\n", fir.fir_tap1);
        printf("    };\n");
    }
    printf("  }\n");
}

void apu_print_setting(void)
{
    printf("void apu_setting(void) {\n");
    struct apu_ch_cfg_t bf_ch_cfg_reg = apu->bf_ch_cfg_reg;

    printf("  apu->bf_ch_cfg_reg = (struct apu_ch_cfg_t){\n");
    printf("    .we_audio_gain = 1, .we_bf_target_dir = 1, .we_bf_sound_ch_en = 1,\n");
    printf("    .audio_gain = 0x%x, .bf_target_dir = %d, .bf_sound_ch_en = %d, .data_src_mode = %d\n",
           bf_ch_cfg_reg.audio_gain, bf_ch_cfg_reg.bf_target_dir, bf_ch_cfg_reg.bf_sound_ch_en, bf_ch_cfg_reg.data_src_mode);
    printf("  };\n");

    struct apu_ctl_t bf_ctl_reg = apu->bf_ctl_reg;

    printf("  apu->bf_ctl_reg = (struct apu_ctl_t){\n");
    printf("    .we_bf_stream_gen = 1, .we_bf_dir_search_en = 1,\n");
    printf("    .bf_stream_gen_en = %d, .bf_dir_search_en = %d\n",
           bf_ctl_reg.bf_stream_gen_en, bf_ctl_reg.bf_dir_search_en);
    printf("  };\n");

    printf("  for(int i = 0; i < 16; i++){\n");
    for (int i = 0; i < 16; i++) {
        struct apu_dir_bidx_t bidx0 = apu->bf_dir_bidx[i][0];
        struct apu_dir_bidx_t bidx1 = apu->bf_dir_bidx[i][1];

        printf("    apu->bf_dir_bidx[%d][0] = (struct apu_dir_bidx_t){\n", i);
        printf("      .dir_rd_idx0 = 0x%x,\n", bidx0.dir_rd_idx0);
        printf("      .dir_rd_idx1 = 0x%x,\n", bidx0.dir_rd_idx1);
        printf("      .dir_rd_idx2 = 0x%x,\n", bidx0.dir_rd_idx2);
        printf("      .dir_rd_idx3 = 0x%x\n", bidx0.dir_rd_idx3);
        printf("    };\n");
        printf("    apu->bf_dir_bidx[%d][1] = (struct apu_dir_bidx_t){\n", i);
        printf("      .dir_rd_idx0 = 0x%x,\n", bidx1.dir_rd_idx0);
        printf("      .dir_rd_idx1 = 0x%x,\n", bidx1.dir_rd_idx1);
        printf("      .dir_rd_idx2 = 0x%x,\n", bidx1.dir_rd_idx2);
        printf("      .dir_rd_idx3 = 0x%x\n", bidx1.dir_rd_idx3);
        printf("    };\n");
    }
    printf("  }\n");

    print_fir("bf_pre_fir0_coef", apu->bf_pre_fir0_coef);
    print_fir("bf_post_fir0_coef", apu->bf_post_fir0_coef);
    print_fir("bf_pre_fir1_coef", apu->bf_pre_fir1_coef);
    print_fir("bf_post_fir1_coef", apu->bf_post_fir1_coef);


    struct apu_dwsz_cfg_t bf_dwsz_cfg_reg = apu->bf_dwsz_cfg_reg;

    printf("  apu->bf_dwsz_cfg_reg = (struct apu_dwsz_cfg_t){\n");
    printf("    .dir_dwn_siz_rate = %d, .voc_dwn_siz_rate = %d\n",
           bf_dwsz_cfg_reg.dir_dwn_siz_rate, bf_dwsz_cfg_reg.voc_dwn_siz_rate);
    printf("  };\n");

    struct apu_fft_cfg_t bf_fft_cfg_reg = apu->bf_fft_cfg_reg;

    printf("  apu->bf_fft_cfg_reg = (struct apu_fft_cfg_t){\n");
    printf("    .fft_enable = %d, .fft_shift_factor = 0x%x\n",
           bf_fft_cfg_reg.fft_enable, bf_fft_cfg_reg.fft_shift_factor);
    printf("  };\n");

    struct apu_int_mask_t bf_int_mask_reg = apu->bf_int_mask_reg;

    printf("  apu->bf_int_mask_reg = (struct apu_int_mask_t){\n");
    printf("    .dir_data_rdy_msk = %d, .voc_buf_rdy_msk = %d\n",
           bf_int_mask_reg.dir_data_rdy_msk, bf_int_mask_reg.voc_buf_rdy_msk);
    printf("  };\n");

    printf("}\n");
}


/*
 * R:radius mic_num_a_circle: the num of mic per circle; center: 0: no center mic, 1:have center mic
 */
void apu_set_delay(
    float R_cm, uint8_t mic_num_a_circle, uint8_t center, 
    float sound_speed_m, int sample_rate, int direction_res
){
#ifndef M_PI
    static const double M_PI = (double)3.14159265358979323846;
#endif
	uint8_t offsets[16][8];
	int i, j;
	float mic_angle_dleta[8], delay[8], arc_tmp;
	float cm_tick = (float)sound_speed_m * 100 / sample_rate; //distance per tick (cm)
	float min;

	for (i = 0; i < mic_num_a_circle; ++i)
	{
		mic_angle_dleta[i] = 360 * i / mic_num_a_circle;
		arc_tmp = 2 * M_PI * mic_angle_dleta[i] / 360;
		delay[i] = R_cm * (1 - cos(arc_tmp)) / cm_tick;
	}
	if (center)
		delay[mic_num_a_circle] = R_cm / cm_tick;

	for (i = 0; i < mic_num_a_circle + center; ++i)
	{
		offsets[0][i] = (int)(delay[i] + 0.5);
	}
	for (; i < 8; i++)
		offsets[0][i] = 0;

	for (j = 1; j < direction_res; ++j)
	{
		for (i = 0; i < mic_num_a_circle; ++i)
		{
			mic_angle_dleta[i] -= 360 / direction_res;
			arc_tmp = 2 * M_PI * mic_angle_dleta[i] / 360;
			delay[i] = R_cm * (1 - cos(arc_tmp)) / cm_tick;
		}
		if (center)
			delay[mic_num_a_circle] = R_cm / cm_tick;

		min = 2 * R_cm;
		for (i = 0; i < mic_num_a_circle; ++i)
		{
			if (delay[i] < min)
				min = delay[i];
		}
		if (min)
		{
			for (i = 0; i < mic_num_a_circle + center; ++i)
			{
				delay[i] = delay[i] - min;
			}
		}

		for (i = 0; i < mic_num_a_circle + center; ++i)
		{
			offsets[j][i] = (int)(delay[i] + 0.5);
		}
		for (; i < 8; i++)
			offsets[0][i] = 0;
	}
	for (size_t i = 0; i < direction_res; i++)
	{ //
		apu_set_direction_delay(i, offsets[i]);
	}
}

static void init_dma_ch(int ch, uint32_t *src_reg, void *buffer, size_t size_of_byte)
{
	dmac_set_single_mode(
        ch, src_reg, buffer, 
        DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT, 
        DMAC_MSIZE_16, DMAC_TRANS_WIDTH_32, size_of_byte / 4
    );
}

static int int_apu(void *ctx)
{
	struct apu_int_stat_t rdy_reg = apu->bf_int_stat_reg;
    static int dir_fft_current_ch = 0;

	if (rdy_reg.dir_search_data_rdy)
	{
		apu_dir_clear_int_state();
        if(apu_using_fft){
            dir_fft_current_ch = (dir_fft_current_ch + 1) % 16;
            for (uint32_t i = 0; i < 512; i++)
            { //
                uint32_t data = apu->sobuf_dma_rdata;

                ((uint32_t*)apu_dir_buffer)[dir_fft_current_ch*512 + i] = data;
            }
            if (dir_fft_current_ch == 0)
            { //
                semaphore_signal(&apu_dir_ready, 1);
            }
        }else{
            for (uint32_t ch = 0; ch < 16; ch++)
            {
                for (uint32_t i = 0; i < 256; i++)
                { //
                    uint32_t data = apu->sobuf_dma_rdata;
                    ((int16_t*)apu_dir_buffer)[ch*512 + i * 2 + 0] = data & 0xffff;
                    ((int16_t*)apu_dir_buffer)[ch*512 + i * 2 + 1] = (data >> 16) & 0xffff;
                }
            }
            semaphore_signal(&apu_dir_ready, 1);
        }
	}
	else if (rdy_reg.voc_buf_data_rdy)
	{
		apu_voc_clear_int_state();

        if(apu_using_fft){
            for (uint32_t i = 0; i < 512; i++)
            { //
                uint32_t data = apu->vobuf_dma_rdata;

                ((uint32_t*)apu_voc_buffer)[i] = data;
            }
        }else{
            for (uint32_t i = 0; i < 256; i++)
            { //
                uint32_t data = apu->vobuf_dma_rdata;

                ((int16_t*)apu_voc_buffer)[i * 2 + 0] = data & 0xffff;
                ((int16_t*)apu_voc_buffer)[i * 2 + 1] = (data >> 16) & 0xffff;
            }
        }

        semaphore_signal(&apu_voc_ready, 1);
	}
	else
	{ //
		printk("[waring]: unknown %s interrupt cause.\n", __func__);
	}
	return 0;
}

static int int_apu_dir_dma(void *ctx)
{
	static int ch = 0;
    if(apu_using_fft){
        ch = (ch + 1) % 16;
        init_dma_ch(apu_dma_dir_ch,
            (uint32_t*)&apu->sobuf_dma_rdata,
            ((uint32_t*)apu_dir_buffer) + ch * 512, 512 * 4);
    }else{
        init_dma_ch(apu_dma_dir_ch,
            (uint32_t*)&apu->sobuf_dma_rdata, ((uint32_t*)apu_dir_buffer),
            512 * 16 * 2);
    }

    if(apu_using_fft){
        if (ch == 0)
        {
            semaphore_signal(&apu_dir_ready, 1);
        }
    }else{
        semaphore_signal(&apu_dir_ready, 1);
    }

	return 0;
}

static int int_apu_voc_dma(void *ctx)
{
    if(apu_using_fft){
        init_dma_ch(apu_dma_voc_ch,
                    (uint32_t*)&apu->vobuf_dma_rdata, ((int16_t*)apu_voc_buffer),
                    512 * 4);
    }else{
        init_dma_ch(apu_dma_voc_ch,
                    (uint32_t*)&apu->vobuf_dma_rdata, ((int16_t*)apu_voc_buffer),
                    512 * 2);
    }
    semaphore_signal(&apu_voc_ready, 1);

	return 0;
}

int event_loop_step(plic_irq_callback_t dir_logic, plic_irq_callback_t voc_logic)
{
	clear_csr(mie, MIP_MEIP);
    if (semaphore_count(&apu_dir_ready) > 0) {
        semaphore_wait(&apu_dir_ready, 1);
        dir_logic(NULL);
    }
    if (semaphore_count(&apu_voc_ready) > 0) {
        semaphore_wait(&apu_voc_ready, 1);
        voc_logic(NULL);
    }
	set_csr(mie, MIP_MEIP);
	return 1;
}

void apu_init_default(
	int reinit_fpioa, int sclk, int ws, int d0, int d1, int d2, int d3,
	int reinit_plic,
	int using_dma, int reinit_dma,
	dmac_channel_number_t dma_dir_ch, dmac_channel_number_t dma_voc_ch,
	int reinit_i2s, uint32_t sample_rate,
	i2s_word_length_t word_length,  // RESOLUTION_16_BIT
    i2s_word_select_cycles_t word_select_size,  // SCLK_CYCLES_32
    i2s_fifo_threshold_t trigger_level,  // TRIGGER_LEVEL_4
    i2s_work_mode_t word_mode,  // STANDARD_MODE
	int using_fft,
	int using_dir, int using_voc, 
	void* dir_buffer, void* voc_buffer
){
	if(reinit_fpioa){
		fpioa_init();
	}
	fpioa_set_function(sclk, FUNC_I2S0_SCLK);
	fpioa_set_function(ws, FUNC_I2S0_WS);
	fpioa_set_function(d0, FUNC_I2S0_IN_D0);
	fpioa_set_function(d1, FUNC_I2S0_IN_D1);
	fpioa_set_function(d2, FUNC_I2S0_IN_D2);
	fpioa_set_function(d3, FUNC_I2S0_IN_D3);

	if(reinit_plic){
		plic_init();
	}
	if(using_dma){
		dmac_irq_register(dma_dir_ch, int_apu_dir_dma, NULL, 4);
		dmac_irq_register(dma_voc_ch, int_apu_voc_dma, NULL, 4);
	}else{
		plic_set_priority(IRQN_I2S0_INTERRUPT, 4);
		plic_irq_enable(IRQN_I2S0_INTERRUPT);
		plic_irq_register(IRQN_I2S0_INTERRUPT, int_apu, NULL);
	}
	if(using_dma && reinit_dma){
		dmac_init();
	}
	if(using_dma){
        sysctl_dma_select(SYSCTL_DMA_CHANNEL_0 + dma_dir_ch,
                SYSCTL_DMA_SELECT_I2S0_BF_DIR_REQ);
        sysctl_dma_select(SYSCTL_DMA_CHANNEL_0 + dma_voc_ch,
                SYSCTL_DMA_SELECT_I2S0_BF_VOICE_REQ);

        apu_dma_dir_ch = dma_dir_ch;
        apu_dma_voc_ch = dma_voc_ch;

		if(using_fft){	
			init_dma_ch(dma_dir_ch,
					(uint32_t*)&apu->sobuf_dma_rdata,
					dir_buffer, 512 * 4);
			init_dma_ch(dma_voc_ch,
					(uint32_t*)&apu->vobuf_dma_rdata, voc_buffer,
					512 * 4);
		}else{
			init_dma_ch(dma_dir_ch,
						(uint32_t*)&apu->sobuf_dma_rdata, dir_buffer,
						512 * 16 * 2);
			init_dma_ch(dma_voc_ch,
						(uint32_t*)&apu->vobuf_dma_rdata, voc_buffer,
					    512 * 2);
		}
	}

	if(reinit_i2s){
		i2s_init(I2S_DEVICE_0, I2S_RECEIVER, 0x3);

		i2s_rx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_0,
							word_length, word_select_size,
							trigger_level, word_mode);
		i2s_rx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_1,
							word_length, word_select_size,
							trigger_level, word_mode);
		i2s_rx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_2,
							word_length, word_select_size,
							trigger_level, word_mode);
		i2s_rx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_3,
							word_length, word_select_size,
							trigger_level, word_mode);

		sysctl_pll_set_freq(SYSCTL_PLL2, sample_rate * 1024);
		i2s_set_sample_rate(I2S_DEVICE_0, sample_rate);
	}

	printk("init\n");

	uint16_t fir_neg_one[17] = {
        0x8000,
        0
	};
	apu_dir_set_prev_fir(fir_neg_one);
	apu_dir_set_post_fir(fir_neg_one);
	apu_voc_set_prev_fir(fir_neg_one);
	apu_voc_set_post_fir(fir_neg_one);
	apu_set_down_size(0, 0);
	apu_set_delay(3, 7, 1, 340, sample_rate, 16);
	apu_set_channel_enabled(0xff);
	apu_set_smpl_shift(0x00);
	apu_voc_set_saturation_limit(0x07ff, 0xf800);
	apu_set_audio_gain(1<<10);
	apu_voc_set_direction(0);

    apu_using_fft = using_fft;
	if(using_fft){
		apu_set_fft_shift_factor(1, 0xaa);
	}else{
		apu_set_fft_shift_factor(0, 0);
	}

    apu_dir_buffer = dir_buffer;
    apu_voc_buffer = voc_buffer;

	apu_set_interrupt_mask(using_dma, using_dma);
	if(using_dir){
		apu_dir_enable();
	}
	if(using_voc){
		apu_voc_enable(1);
	}else{
		apu_voc_enable(0);
	}
}
