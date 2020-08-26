#include <stdio.h>
#include <timer.h>
#include <pwm.h>
#include <plic.h>
#include <string.h>
#include "pwm_play_audio.h"

#define BG_READ_WORD(x) ((((uint32_t)wav_head_buff[x + 0]) << 24) | (((uint32_t)wav_head_buff[x + 1]) << 16) |\
            (((uint32_t)wav_head_buff[x + 2]) << 8) | (((uint32_t)wav_head_buff[x + 3]) << 0))
#define LG_READ_WORD(x) ((((uint32_t)wav_head_buff[x + 3]) << 24) | (((uint32_t)wav_head_buff[x + 2]) << 16) |\
            (((uint32_t)wav_head_buff[x + 1]) << 8) | (((uint32_t)wav_head_buff[x + 0]) << 0))
#define LG_READ_HALF(x) ((((uint16_t)wav_head_buff[x + 1]) << 8) | (((uint16_t)wav_head_buff[x + 0]) << 0))

static volatile pwm_play_info_t pwm_play_info;

static void pwm_play_disable(timer_device_number_t timer, timer_channel_number_t timer_channel, pwm_device_number_t pwm, pwm_channel_number_t pwm_channel)
{
    timer_set_enable(timer, timer_channel, 0);
    timer_irq_unregister(timer, timer_channel);
    pwm_set_enable(pwm, pwm_channel, 0);
}

static int timer_callback(void *ctx)
{
    pwm_play_info_t *v_pwm_play_info = (pwm_play_info_t *)ctx;

    if(v_pwm_play_info->cur_cnt >= v_pwm_play_info->count)
    {
        pwm_play_disable(v_pwm_play_info->timer, v_pwm_play_info->timer_channel, v_pwm_play_info->pwm, v_pwm_play_info->pwm_channel);
        v_pwm_play_info->cur_cnt = 0;
        __sync_lock_test_and_set(&v_pwm_play_info->status, 0);
        return 0;
    }

    double duty = 0.0;
    if(v_pwm_play_info->bitspersample == 16)
    {
        int16_t v_i16data = *(((int16_t *)v_pwm_play_info->data) + (v_pwm_play_info->numchannels * (v_pwm_play_info->cur_cnt)++));
        uint16_t v_u16data = v_i16data + 32768;
        duty = v_u16data / 65536.0;
    }
    else if(v_pwm_play_info->bitspersample == 8)
    {
        uint8_t v_8data = *(v_pwm_play_info->data + (v_pwm_play_info->numchannels * (v_pwm_play_info->cur_cnt)++));
        duty = v_8data / 256.0;
    }
    else  /* 24bit */
    {
        uint8_t v_data[4] = {0, 0, 0, 0};
        memcpy(v_data, v_pwm_play_info->data + (v_pwm_play_info->numchannels * (3 * (v_pwm_play_info->cur_cnt)++)), 3);
        if(v_data[2] & 0x80) /* sign expand */
        {
            v_data[3] = 0xFF;
        }
        uint32_t v_u32data = *((int32_t *)v_data) + 0x800000UL;
        duty = (double)v_u32data / 0x1000000UL;
    }

    pwm_set_frequency(v_pwm_play_info->pwm, v_pwm_play_info->pwm_channel, v_pwm_play_info->pwm_freq, duty);

    return 0;
}

int pwm_play_init(timer_device_number_t timer, pwm_device_number_t pwm)
{
    /* Init timer */
    timer_init(timer);

    /* Init PWM */
    if((uint32_t)timer != (uint32_t)pwm)
        pwm_init(pwm);

    return 0;
}

int pwm_play_wav(timer_device_number_t timer, timer_channel_number_t timer_channel, pwm_device_number_t pwm, pwm_channel_number_t pwm_channel, uint8_t *wav_ptr, uint8_t mode)
{
    uint8_t *wav_head_buff = wav_ptr;
    uint32_t index;
    wav_info_t wav_info;

    if((uint32_t)timer == (uint32_t)pwm && (uint32_t)timer_channel == (uint32_t)pwm_channel)
    {
        return TIMER_PWM_CHANNEL_ERR;
    }

    if(mode == 1)   /* force play */
    {
        if(__sync_lock_test_and_set(&pwm_play_info.status, 1))
        {
            pwm_play_disable(timer, timer_channel, pwm, pwm_channel);
        }
    }
    else if(mode == 2)   /* return */
    {
        if(__sync_lock_test_and_set(&pwm_play_info.status, 1))
        {
            return DEVICE_BUSY;
        }
    }
    else /* block */
    {
        while(__sync_lock_test_and_set(&pwm_play_info.status, 1));
    }

    index = 0;
    if (BG_READ_WORD(index) != RIFF_ID)
        return UNVALID_RIFF_ID;
    index += 4;
    index += 4;
    if (BG_READ_WORD(index) != WAVE_ID)
        return UNVALID_WAVE_ID;
    index += 4;
    if (BG_READ_WORD(index) != FMT_ID)
        return UNVALID_FMT_ID;
    index += 4;
    uint32_t v_fmt_size = LG_READ_WORD(index);
    index += 4;
    if (LG_READ_HALF(index) != 0x01)
        return UNSUPPORETD_FORMATTAG;
    index += 2;
    wav_info.numchannels = LG_READ_HALF(index);
    if (wav_info.numchannels != 1 && wav_info.numchannels != 2)
        return UNSUPPORETD_NUMBER_OF_CHANNEL;
    index += 2;
    wav_info.samplerate = LG_READ_WORD(index);
//  if (wav_info.samplerate != 16000 && wav_info.samplerate != 11025 && wav_info.samplerate != 22050 && wav_info.samplerate != 44100)
//      return UNSUPPORETD_SAMPLE_RATE;
    index += 4;
    wav_info.byterate = LG_READ_WORD(index);
    index += 4;
    wav_info.blockalign = LG_READ_HALF(index);
    index += 2;
    wav_info.bitspersample = LG_READ_HALF(index);
    if (wav_info.bitspersample != 8 && wav_info.bitspersample != 16 && wav_info.bitspersample != 24)
        return UNSUPPORETD_BITS_PER_SAMPLE;
    index += 2;
    index = index - 16 + v_fmt_size;
    if (BG_READ_WORD(index) == LIST_ID) {
        index += 4;
        index += LG_READ_WORD(index);
        index += 4;
        if (index >= 500)
            return UNVALID_LIST_SIZE;
    }
    if (BG_READ_WORD(index) != DATA_ID)
        return UNVALID_DATA_ID;
    index += 4;
    wav_info.datasize = LG_READ_WORD(index);
    index += 4;
    printf("numchannels:%d\n", wav_info.numchannels);
    printf("samplerate:%d\n", wav_info.samplerate);
    printf("byterate:%d\n", wav_info.byterate);
    printf("blockalign:%d\n", wav_info.blockalign);
    printf("bitspersample:%d\n", wav_info.bitspersample);
    printf("datasize:%d\n", wav_info.datasize);

    pwm_play_info.timer = timer;
    pwm_play_info.timer_channel = timer_channel;
    pwm_play_info.pwm = pwm;
    pwm_play_info.pwm_channel = pwm_channel;
    pwm_play_info.count = wav_info.datasize / (wav_info.numchannels * wav_info.bitspersample / 8);
    pwm_play_info.cur_cnt = 0;
    pwm_play_info.bitspersample = wav_info.bitspersample;
    pwm_play_info.numchannels = wav_info.numchannels;
    pwm_play_info.data = wav_ptr + index;

    /* disable timer and pwm */
    pwm_play_disable(timer, timer_channel, pwm, pwm_channel);

    pwm_play_info.pwm_freq = 1e6 / wav_info.samplerate * wav_info.samplerate;
    pwm_set_frequency(pwm, pwm_channel, pwm_play_info.pwm_freq, 0.000001);
    /* Enable PWM */
    pwm_set_enable(pwm, pwm_channel, 1);

    uint64_t v_peroid = 1e9 / wav_info.samplerate;
    /* 16k sample rate */
    timer_set_interval(timer, timer_channel, v_peroid);
    /* Set timer callback function with repeat method */
    timer_irq_register(timer, timer_channel, 0, 1, timer_callback, (void *)&pwm_play_info);
    /* Enable timer */
    timer_set_enable(timer, timer_channel, 1);

    return OK;
}

