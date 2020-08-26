#ifndef _PWM_AUDIO_H
#define _PWM_AUDIO_H
#include "timer.h"
#include "pwm.h"
/* Audio Parsing Constants */
#define  RIFF_ID        0x52494646  /* correspond to the letters 'RIFF' */
#define  WAVE_ID        0x57415645  /* correspond to the letters 'WAVE' */
#define  FMT_ID         0x666D7420  /* correspond to the letters 'fmt ' */
#define  LIST_ID        0x4C495354  /* correspond to the letters 'LIST' */
#define  DATA_ID        0x64617461  /* correspond to the letters 'data' */

typedef struct _pwm_play_info_t
{
    timer_device_number_t timer;
    timer_channel_number_t timer_channel;
    pwm_device_number_t pwm;
    pwm_channel_number_t pwm_channel;
    uint32_t count;               /* total */
    uint32_t cur_cnt;
    uint16_t bitspersample;       /* bits 8 or 16 or 24 */
    uint16_t numchannels;         /* 1:, 2: */
    uint8_t *data;                   /* audio data */
    double pwm_freq;              /* pwm freq */
    uint32_t status;              /* 0:idel, 1:busy */
}pwm_play_info_t;


typedef struct {
    uint16_t numchannels;
    uint32_t samplerate;
    uint32_t byterate;
    uint16_t blockalign;
    uint16_t bitspersample;
    uint32_t datasize;
} __attribute__((packed, aligned(4))) wav_info_t;

/* Error Identification structure */
enum errorcode_e {
    OK = 0,
    DEVICE_BUSY,
    UNVALID_RIFF_ID,
    UNVALID_RIFF_SIZE,
    UNVALID_WAVE_ID,
    UNVALID_FMT_ID,
    UNVALID_FMT_SIZE,
    UNSUPPORETD_FORMATTAG,
    UNSUPPORETD_NUMBER_OF_CHANNEL,
    UNSUPPORETD_SAMPLE_RATE,
    UNSUPPORETD_BITS_PER_SAMPLE,
    UNVALID_LIST_SIZE,
    UNVALID_DATA_ID,
    TIMER_PWM_CHANNEL_ERR,
};

int pwm_play_init(timer_device_number_t timer, pwm_device_number_t pwm);

/**
 * @brief       Set pwm duty
 *
 * @param[in]   timer               timer
 * @param[in]   timer_channel       timer channel
 * @param[in]   pwm                 pwm
 * @param[in]   pwm_channel         pwm channel

 * @param[in]   wav_ptr             pcm data
 * @param[in]   mode                0:block  1:force play  2:return right now
 *
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int pwm_play_wav(timer_device_number_t timer, timer_channel_number_t timer_channel, pwm_device_number_t pwm, pwm_channel_number_t pwm_channel, uint8_t *wav_ptr, uint8_t mode);

#endif
