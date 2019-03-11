#include <dmac.h>

#define DMA_CH_I2S_RX DMAC_CHANNEL0
#define DMA_CH_I2S_TX DMAC_CHANNEL1
#define DMA_CH_FFT_RX DMAC_CHANNEL2
#define DMA_CH_FFT_TX DMAC_CHANNEL3
#define SAMPLE_RATE 44100UL

// aec
#define AEC_FRAME_LEN  4096UL
#define AEC_TAIL AEC_FRAME_LEN*7UL  // *7

// devererberation
#define KAL_FRAME_LEN  512UL
#define KAL_FRAME_SHIFT 512UL  // 256
#define KAL_NRE 257UL
#define KAL_WPE_L 10UL  // 16
#define KAL_ALPHA  0.98f
