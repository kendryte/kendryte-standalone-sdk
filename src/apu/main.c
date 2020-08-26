#include "init.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <printf.h>
#include "apu.h"

int count;
int assert_state;

int dir_logic(void)
{
	int32_t dir_sum = 0;
	int32_t dir_max = 0;
	uint16_t contex = 0;

	for (size_t ch = 0; ch < APU_DIR_CHANNEL_MAX; ch++) { //

			for (size_t i = 0; i < APU_DIR_CHANNEL_SIZE; i++) { //
				 dir_sum += (int32_t)APU_DIR_BUFFER[ch][i] * (int32_t)APU_DIR_BUFFER[ch][i];
			}
			dir_sum = dir_sum / APU_DIR_CHANNEL_SIZE;
			if(dir_sum > dir_max){
				dir_max = dir_sum;
				contex = ch;
			}
			printf("%d	", dir_sum);
		}
		printf("   %d\n", contex);
		printf("\n");

	apu_dir_enable();
	return 0;
}

int voc_logic(void)
{

	return 0;
}

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

	event_loop();

}
