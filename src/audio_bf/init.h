#pragma once
#include <stdint.h>
#include <plic.h>
#include <i2s.h>
#include <sysctl.h>
#include <dmac.h>
#include <fpioa.h>
#include "gpiohs.h"


#define DIRECTION_RES 16
#define I2S_FS 44100
#define SOUND_SPEED 340
#define M_PI       3.14159265358979323846

#ifndef APU_DIR_ENABLE
#define APU_DIR_ENABLE 1
#endif

#ifndef APU_VOC_ENABLE
#define APU_VOC_ENABLE 1
#endif

#ifndef APU_DMA_ENABLE
#define APU_DMA_ENABLE 1
#endif

#ifndef APU_FFT_ENABLE
#define APU_FFT_ENABLE 0
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

#ifndef APU_AUDIO_GAIN_TEST
#define APU_AUDIO_GAIN_TEST (1 << 10)
#endif

#define APU_DIR_DMA_CHANNEL DMAC_CHANNEL3
#define APU_VOC_DMA_CHANNEL DMAC_CHANNEL4

#define APU_DIR_CHANNEL_MAX 16
#define APU_DIR_CHANNEL_SIZE 512
#define APU_VOC_CHANNEL_SIZE 512

#if APU_FFT_ENABLE
extern volatile uint32_t APU_DIR_FFT_BUFFER[APU_DIR_CHANNEL_MAX]
				       [APU_DIR_CHANNEL_SIZE]
	__attribute__((aligned(128)));
extern volatile uint32_t APU_VOC_FFT_BUFFER[APU_VOC_CHANNEL_SIZE]
	__attribute__((aligned(128)));
#else
extern volatile int16_t APU_DIR_BUFFER[APU_DIR_CHANNEL_MAX]
				  [APU_DIR_CHANNEL_SIZE]
	__attribute__((aligned(128)));
extern volatile int16_t APU_VOC_BUFFER[APU_VOC_CHANNEL_SIZE]
	__attribute__((aligned(128)));
#endif


extern volatile uint64_t dir_logic_count;
extern volatile uint64_t voc_logic_count;

void init_all(void);
