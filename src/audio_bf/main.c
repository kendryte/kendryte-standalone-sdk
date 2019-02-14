#include "init.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <printf.h>
#include "audio_bf.h"

int count;
int assert_state;

int dir_logic(void)
{
//	printk("%s\n", __func__);
	int32_t dir_sum = 0;
	int32_t dir_max = 0;
	uint16_t contex = 0;

	for (size_t ch = 0; ch < AUDIO_BF_DIR_CHANNEL_MAX; ch++) { //

			for (size_t i = 0; i < AUDIO_BF_DIR_CHANNEL_SIZE; i++) { //
				 dir_sum += (int32_t)AUDIO_BF_DIR_BUFFER[ch][i] * (int32_t)AUDIO_BF_DIR_BUFFER[ch][i];
			}
			dir_sum = dir_sum / AUDIO_BF_DIR_CHANNEL_SIZE;
			if(dir_sum > dir_max){
				dir_max = dir_sum;
				contex = ch;
			}
			printf("%d	", dir_sum);
		}
		printf("   %d\n", contex);
		printf("\n");

#if 0
	if (count < 5) {
#if AUDIO_BF_DATA_DEBUG
#if AUDIO_BF_FFT_ENABLE
		for (size_t i = 0; i < AUDIO_BF_DIR_CHANNEL_SIZE; i++) { //
			printk("[%lu]%08x ", i, AUDIO_BF_DIR_FFT_BUFFER[1][i]);
		}
		printk("\n");
		printk("\n");
#else
		for (size_t ch = 1; ch < AUDIO_BF_DIR_CHANNEL_MAX; ch++) { //

			for (size_t i = 1; i < AUDIO_BF_DIR_CHANNEL_SIZE; i++) { //
				if (AUDIO_BF_DIR_BUFFER[ch][i] != AUDIO_BF_DIR_BUFFER[ch][i - 1] + 1) { //
					printk("dir at %lx %lx, expect inc %x got %x\n",
					       ch, i,
					       (AUDIO_BF_DIR_BUFFER[ch][i - 1] + 1) & 0xffff,
					       AUDIO_BF_DIR_BUFFER[ch][i] & 0xffff);
					assert_state = -1;
				}
				if (AUDIO_BF_DIR_BUFFER[ch][i] != AUDIO_BF_DIR_BUFFER[ch - 1][i] + 1) { //
					printk("dir at %lx %lx, expect offset-prev %x got %x\n",
					       ch, i,
					       (AUDIO_BF_DIR_BUFFER[ch - 1][i] + 1) & 0xffff,
					       AUDIO_BF_DIR_BUFFER[ch][i] & 0xffff);
					assert_state = -1;
				}
			}
		}
#endif
#endif
#if AUDIO_BF_SMPL_SHIFT_DEBUG
		uint32_t smpl_shift = audio_bf_get_smpl_shift();

		for (size_t ch = 1; ch < AUDIO_BF_DIR_CHANNEL_MAX; ch++) { //
			for (size_t i = 1; i < AUDIO_BF_DIR_CHANNEL_SIZE;
			     i++) { //
				int16_t expt = 1 << smpl_shift;
				int16_t current = AUDIO_BF_DIR_BUFFER[ch][i];

				if (current != expt) {
					printk("smpl_shift check dir at %lx %lx, expect %x got %x\n",
					       ch, i, expt, current);
					assert_state = -1;
				}
			}
		}

		audio_bf_set_smpl_shift(smpl_shift + 1);
#endif
	//	audio_bf_voc_set_direction(count % 12);
	//	audio_bf_dir_enable();
	//	count++;
	}
	#endif	


	audio_bf_dir_enable();
	return 0;
}

int voc_logic(void)
{
	#if 0
//	printk("%s\n", __func__);
	if (count < 5) {
#if AUDIO_BF_DATA_DEBUG & !(AUDIO_BF_GAIN_DEBUG)
#if AUDIO_BF_FFT_ENABLE
		for (size_t i = 0; i < AUDIO_BF_DIR_CHANNEL_SIZE; i++) { //
			printk("[%lu]%08x ", i, AUDIO_BF_VOC_FFT_BUFFER[i]);
		}
		printk("\n");
		printk("\n");
#else
		for (size_t i = 1; i < AUDIO_BF_VOC_CHANNEL_SIZE; i++) { //
			if (AUDIO_BF_VOC_BUFFER[i]
			    != AUDIO_BF_VOC_BUFFER[i - 1] + 1) { //
				printk("voc at %lx, expect %x got %x\n", i,
				       AUDIO_BF_VOC_BUFFER[i - 1] + 1,
				       (uint32_t)AUDIO_BF_VOC_BUFFER[i]
					       & 0xffff);
				assert_state = -1;
			}
		}
#endif
#endif
#if AUDIO_BF_GAIN_DEBUG
		for (size_t i = 1; i < AUDIO_BF_VOC_CHANNEL_SIZE; i++) { //
			if (AUDIO_BF_VOC_BUFFER[i]
			    != AUDIO_BF_VOC_BUFFER[i - 1]) {
				if (AUDIO_BF_VOC_BUFFER[i]
				    == AUDIO_BF_VOC_BUFFER[i - 1] << 1) {
					; // pass
				} else if (AUDIO_BF_VOC_BUFFER[i] <= 1) {
					; // pass
				} else {  //
					printk("voc not expect %d %d\n",
					       AUDIO_BF_VOC_BUFFER[i],
					       AUDIO_BF_VOC_BUFFER[i - 1]);
					assert_state = -1;
				}
			}
		}
#endif
#if AUDIO_BF_SMPL_SHIFT_DEBUG
		uint32_t smpl_shift = audio_bf_get_smpl_shift();

		for (size_t i = 1; i < AUDIO_BF_VOC_CHANNEL_SIZE; i++) { //
			int16_t expt = 1 << smpl_shift;
			int16_t current = AUDIO_BF_VOC_BUFFER[i];

			if (current != expt) { //
				printk("smpl_shift check voc at %lx, expect %x got %x\n",
				       i, expt, current);
				assert_state = -1;
			}
		}

		audio_bf_set_smpl_shift(smpl_shift + 1);
#endif
#if AUDIO_BF_SATURATION_DEBUG
		uint32_t counters = audio_bf_voc_get_saturation_counter();

		audio_bf_voc_reset_saturation_counter();

		uint16_t cnt_u = counters >> 16;
		uint16_t cnt_d = counters & 0xffff;

		if (AUDIO_BF_INPUT_CONST_DEBUG >= AUDIO_BF_SATURATION_VPOS_DEBUG
		    || AUDIO_BF_INPUT_CONST_DEBUG
			       <= AUDIO_BF_SATURATION_VNEG_DEBUG) {
			if (cnt_u != cnt_d) { //
				printk("saturation check voc warning: %d / %d\n",
				       cnt_u, cnt_d);
			}
		} else {
			if (cnt_u != 0) { //
				printk("saturation check voc warning: %d / %d\n",
				       cnt_u, cnt_d);
			}
		}
#endif
#if AUDIO_BF_SETDIR_DEBUG
		struct audio_bf_ch_cfg_t ch_cfg = audio_bf->bf_ch_cfg_reg;
		uint32_t dir = ch_cfg.bf_target_dir;
		uint32_t found_7 = 0;

		for (size_t i = 32; i < AUDIO_BF_VOC_CHANNEL_SIZE - 32;
		     i++) {				   //
			if (AUDIO_BF_VOC_BUFFER[i] == 7) { //
				found_7 = 1;
				if (AUDIO_BF_VOC_BUFFER[i + dir] != 1) { //
					printk("voc setdir at %lx, got\n %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
					       i, AUDIO_BF_VOC_BUFFER[i - 8],
					       AUDIO_BF_VOC_BUFFER[i - 7],
					       AUDIO_BF_VOC_BUFFER[i - 6],
					       AUDIO_BF_VOC_BUFFER[i - 5],
					       AUDIO_BF_VOC_BUFFER[i - 4],
					       AUDIO_BF_VOC_BUFFER[i - 3],
					       AUDIO_BF_VOC_BUFFER[i - 2],
					       AUDIO_BF_VOC_BUFFER[i],
					       AUDIO_BF_VOC_BUFFER[i + 1],
					       AUDIO_BF_VOC_BUFFER[i + 2],
					       AUDIO_BF_VOC_BUFFER[i + 3],
					       AUDIO_BF_VOC_BUFFER[i + 4],
					       AUDIO_BF_VOC_BUFFER[i + 5],
					       AUDIO_BF_VOC_BUFFER[i + 6],
					       AUDIO_BF_VOC_BUFFER[i + 7],
					       AUDIO_BF_VOC_BUFFER[i + 8]);
					assert_state = -1;
				}
			}
		}
		if (!found_7) {
			printk("voc setdir not found 7, please check input data.\n");
			assert_state = -1;
		}
		dir = dir + 3;
		audio_bf_voc_set_direction(dir % 16);
#endif
		count++;
	}
	#endif
	return 0;
}

/*#if AUDIO_BF_GAIN_DEBUG
int set_gain(void)
{
	for (size_t i = 0; i < 100; i++) { //
		int32_t gain = 1 << (i % 11);

		audio_bf_set_audio_gain(gain);
		struct audio_bf_ch_cfg_t ch_cfg = audio_bf->bf_ch_cfg_reg;

		if (ch_cfg.audio_gain != gain) { //
			printk("[error]: audio_bf_set_audio_gain failed.\n");
			assert_state = -1;
		}
	}
	return 0;
}
#endif*/

/*#if AUDIO_BF_SETDIR_DEBUG
int set_direction(void)
{
	for (size_t i = 0; i < 100; i++) {
		int32_t dir = (i % 16);

		audio_bf_voc_set_direction(dir);
		struct audio_bf_ch_cfg_t ch_cfg = audio_bf->bf_ch_cfg_reg;

		if (ch_cfg.bf_target_dir != dir) { //
			printk("[error]: audio_bf_voc_set_direction failed.\n");
			assert_state = -1;
		}
	}
	return 0;
}
#endif*/

int event_loop(void)
{
	while (1) {
		if (dir_logic_count > 0) {
			dir_logic();
			while (--dir_logic_count != 0) {
				printk("[warning]: %s, restart before prev callback has end\n",
				       "dir_logic");
			}
		}
		if (voc_logic_count > 0) {
			voc_logic();
			while (--voc_logic_count != 0) {
				printk("[warning]: %s, restart before prev callback has end\n",
				       "voc_logic");
			}
		}
	}
	return 0;
}

int main(void)
{
	sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
	printk("git id: %u\n", sysctl->git_id.git_id);
	printk("init start.\n");
	clear_csr(mie, MIP_MEIP);
	init_all();
	printk("init done.\n");
	set_csr(mie, MIP_MEIP);
	set_csr(mstatus, MSTATUS_MIE);

/*#if AUDIO_BF_GAIN_DEBUG
	set_gain();
#endif
#if AUDIO_BF_SETDIR_DEBUG
	set_direction();
#endif
*/
	event_loop();
	
// #endif

}
