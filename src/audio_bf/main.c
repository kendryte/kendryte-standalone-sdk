#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <printf.h>
#include <dmac.h>
#include <plic.h>
#include <i2s.h>
#include <sysctl.h>
#include <dmac.h>
#include <fpioa.h>
#include "apu.h"

#define APU_DIR_DMA_CHANNEL DMAC_CHANNEL3
#define APU_VOC_DMA_CHANNEL DMAC_CHANNEL4

#define APU_DIR_CHANNEL_MAX 16
#define APU_DIR_CHANNEL_SIZE 512
#define APU_VOC_CHANNEL_SIZE 512

#define APU_FFT_ENABLE 0

int count;
int assert_state;

#if APU_FFT_ENABLE
volatile uint32_t APU_DIR_FFT_BUFFER[APU_DIR_CHANNEL_MAX]
				       [APU_DIR_CHANNEL_SIZE]
	__attribute__((aligned(128)));
volatile uint32_t APU_VOC_FFT_BUFFER[APU_VOC_CHANNEL_SIZE]
	__attribute__((aligned(128)));
#else
volatile int16_t APU_DIR_BUFFER[APU_DIR_CHANNEL_MAX]
				  [APU_DIR_CHANNEL_SIZE]
	__attribute__((aligned(128)));
volatile int16_t APU_VOC_BUFFER[APU_VOC_CHANNEL_SIZE]
	__attribute__((aligned(128)));
#endif

int16_t record[512*1024];

int dir_logic(void* ctx)
{
	static uint64_t counter = 0;
	printk("%s %lu\n", __func__, counter++);
	int32_t dir_sum = 0;
	int32_t dir_max = 0;
	uint16_t contex = 0;

#if APU_FFT_ENABLE
#else
	// for (size_t ch = 0; ch < 16; ch++) { //

	// 	for (size_t i = 0; i < 512; i++) { //
	// 			dir_sum += (int32_t)APU_DIR_BUFFER[ch][i] * (int32_t)APU_DIR_BUFFER[ch][i];
	// 	}
	// 	dir_sum = dir_sum / 512;
	// 	if(dir_sum > dir_max){
	// 		dir_max = dir_sum;
	// 		contex = ch;
	// 	}
	// 	//printf("%d	", dir_sum);
	// }
	// printf("   %d\n", contex);
	// printf("\n");
#endif

	apu_dir_enable();
	return 0;
}

int voc_logic(void* ctx)
{
	static uint64_t counter = 0;
	if(counter < 1){
		for(int i=0; i<512; i++){
			record[512*counter + i] = APU_VOC_BUFFER[i];
		}
	}
	if(counter == 1){
		for(int i=0; i<512*1; i++){
			printk("%d,\n", record[i]);
		}
		while(1);
	}
	printk("%s %lu\n", __func__, counter++);
	return 0;
}


int main(void)
{
	printk("git id: %u\n", sysctl->git_id.git_id);
	printk("init start.\n");
	clear_csr(mie, MIP_MEIP);
	//init_all();
	apu_init_default(
		1, 39, 46, 42, 43, 44, 45,
		1, 0, 1, APU_DIR_DMA_CHANNEL, APU_VOC_DMA_CHANNEL,
		1, 44100, RESOLUTION_16_BIT, SCLK_CYCLES_32, 
		TRIGGER_LEVEL_4, STANDARD_MODE,
		APU_FFT_ENABLE, 0, 1, 
#if APU_FFT_ENABLE
		(void*)APU_DIR_FFT_BUFFER, (void*)APU_VOC_FFT_BUFFER
#else
		(void*)APU_DIR_BUFFER, (void*)APU_VOC_BUFFER
#endif
	);
	apu_print_setting();
	printk("init done.\n");
	set_csr(mie, MIP_MEIP);
	set_csr(mstatus, MSTATUS_MIE);

	while(event_loop_step(dir_logic, voc_logic) | 1){}
}
