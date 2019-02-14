#pragma once
#include <stdint.h>
#include <plic.h>
#include <i2s.h>
#include <sysctl.h>
#include <dmac.h>
#include <fpioa.h>
#include "gpiohs.h"


#ifndef AUDIO_BF_DIR_ENABLE
#define AUDIO_BF_DIR_ENABLE 1
#endif

#ifndef AUDIO_BF_VOC_ENABLE
#define AUDIO_BF_VOC_ENABLE 1
#endif

#ifndef AUDIO_BF_DMA_ENABLE
#define AUDIO_BF_DMA_ENABLE 0
#endif

#ifndef AUDIO_BF_FFT_ENABLE
#define AUDIO_BF_FFT_ENABLE 0
#endif

#ifndef AUDIO_BF_DATA_DEBUG
#define AUDIO_BF_DATA_DEBUG 0
#endif

#ifndef AUDIO_BF_GAIN_DEBUG
#define AUDIO_BF_GAIN_DEBUG 0
#endif

#ifndef AUDIO_BF_SETDIR_DEBUG
#define AUDIO_BF_SETDIR_DEBUG 0
#endif

#ifndef AUDIO_BF_SMPL_SHIFT
#define AUDIO_BF_SMPL_SHIFT 0x00
#endif

#ifndef AUDIO_BF_SATURATION_DEBUG
#define AUDIO_BF_SATURATION_DEBUG 0
#endif

#ifndef AUDIO_BF_SATURATION_VPOS_DEBUG
#define AUDIO_BF_SATURATION_VPOS_DEBUG 0x07ff
#endif

#ifndef AUDIO_BF_SATURATION_VNEG_DEBUG
#define AUDIO_BF_SATURATION_VNEG_DEBUG 0xf800
#endif

#ifndef AUDIO_BF_INPUT_CONST_DEBUG
#define AUDIO_BF_INPUT_CONST_DEBUG 0x0
#endif

#ifndef AUDIO_BF_SMPL_SHIFT_DEBUG
#define AUDIO_BF_SMPL_SHIFT_DEBUG 0
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

#ifndef AUDIO_BF_AUDIO_GAIN_TEST
#define AUDIO_BF_AUDIO_GAIN_TEST (1 << 10)
#endif

#ifndef AUDIO_BF_PRESETN_DEBUG
#define AUDIO_BF_PRESETN_DEBUG 1
#endif

#ifndef AUDIO_BF_DEBUG_NO_EXIT
#define AUDIO_BF_DEBUG_NO_EXIT 1
#endif


#define AUDIO_BF_DIR_DMA_CHANNEL DMAC_CHANNEL3
#define AUDIO_BF_VOC_DMA_CHANNEL DMAC_CHANNEL4

#define AUDIO_BF_DIR_CHANNEL_MAX 16
#define AUDIO_BF_DIR_CHANNEL_SIZE 512
#define AUDIO_BF_VOC_CHANNEL_SIZE 512

#if AUDIO_BF_FFT_ENABLE
extern uint32_t AUDIO_BF_DIR_FFT_BUFFER[AUDIO_BF_DIR_CHANNEL_MAX]
				       [AUDIO_BF_DIR_CHANNEL_SIZE]
	__attribute__((aligned(128)));
extern uint32_t AUDIO_BF_VOC_FFT_BUFFER[AUDIO_BF_VOC_CHANNEL_SIZE]
	__attribute__((aligned(128)));
#else
extern int16_t AUDIO_BF_DIR_BUFFER[AUDIO_BF_DIR_CHANNEL_MAX]
				  [AUDIO_BF_DIR_CHANNEL_SIZE]
	__attribute__((aligned(128)));
extern int16_t AUDIO_BF_VOC_BUFFER[AUDIO_BF_VOC_CHANNEL_SIZE]
	__attribute__((aligned(128)));
#endif


extern uint64_t dir_logic_count;
extern uint64_t voc_logic_count;

void init_all(void);
