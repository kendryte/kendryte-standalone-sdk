#pragma once
#include <stdint.h>
#include <plic.h>
#include <i2s.h>
#include <sysctl.h>
#include <dmac.h>
#include <fpioa.h>
#include "gpiohs.h"


#ifndef APU_DIR_ENABLE
#define APU_DIR_ENABLE 1
#endif

#ifndef APU_VOC_ENABLE
#define APU_VOC_ENABLE 1
#endif

#ifndef APU_DMA_ENABLE
#define APU_DMA_ENABLE 0
#endif

#ifndef APU_FFT_ENABLE
#define APU_FFT_ENABLE 0
#endif

#ifndef APU_DATA_DEBUG
#define APU_DATA_DEBUG 0
#endif

#ifndef APU_GAIN_DEBUG
#define APU_GAIN_DEBUG 0
#endif

#ifndef APU_SETDIR_DEBUG
#define APU_SETDIR_DEBUG 0
#endif

#ifndef APU_SMPL_SHIFT
#define APU_SMPL_SHIFT 0x00
#endif

#ifndef APU_SATURATION_DEBUG
#define APU_SATURATION_DEBUG 0
#endif

#ifndef APU_SATURATION_VPOS_DEBUG
#define APU_SATURATION_VPOS_DEBUG 0x07ff
#endif

#ifndef APU_SATURATION_VNEG_DEBUG
#define APU_SATURATION_VNEG_DEBUG 0xf800
#endif

#ifndef APU_INPUT_CONST_DEBUG
#define APU_INPUT_CONST_DEBUG 0x0
#endif

#ifndef APU_SMPL_SHIFT_DEBUG
#define APU_SMPL_SHIFT_DEBUG 0
#endif

#ifndef I2S_RESOLUTION_TEST
#define I2S_RESOLUTION_TEST RESOLUTION_12_BIT
#endif

#ifndef I2S_SCLK_CYCLES_TEST
#define I2S_SCLK_CYCLES_TEST SCLK_CYCLES_16
#endif

#ifndef SYSCTL_THRESHOLD_I2S0_TEST
#define SYSCTL_THRESHOLD_I2S0_TEST 0xf
#endif

#ifndef APU_AUDIO_GAIN_TEST
#define APU_AUDIO_GAIN_TEST (1 << 10)
#endif

#ifndef APU_PRESETN_DEBUG
#define APU_PRESETN_DEBUG 1
#endif

#ifndef APU_DEBUG_NO_EXIT
#define APU_DEBUG_NO_EXIT 1
#endif


#define APU_DIR_DMA_CHANNEL DMAC_CHANNEL3
#define APU_VOC_DMA_CHANNEL DMAC_CHANNEL4

#define APU_DIR_CHANNEL_MAX 16
#define APU_DIR_CHANNEL_SIZE 512
#define APU_VOC_CHANNEL_SIZE 512

#if APU_FFT_ENABLE
extern uint32_t APU_DIR_FFT_BUFFER[APU_DIR_CHANNEL_MAX]
				       [APU_DIR_CHANNEL_SIZE]
	__attribute__((aligned(128)));
extern uint32_t APU_VOC_FFT_BUFFER[APU_VOC_CHANNEL_SIZE]
	__attribute__((aligned(128)));
#else
extern int16_t APU_DIR_BUFFER[APU_DIR_CHANNEL_MAX]
				  [APU_DIR_CHANNEL_SIZE]
	__attribute__((aligned(128)));
extern int16_t APU_VOC_BUFFER[APU_VOC_CHANNEL_SIZE]
	__attribute__((aligned(128)));
#endif


extern uint64_t dir_logic_count;
extern uint64_t voc_logic_count;

void init_all(void);
