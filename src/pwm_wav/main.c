/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <syslog.h>
#include <timer.h>
#include <pwm.h>
#include <plic.h>
#include <sysctl.h>
#include <fpioa.h>
#include "test_pcm.h"

#define TIMER_NOR   0
#define TIMER_CHN   0
#define TIMER_PWM   1
#define TIMER_PWM_CHN 0

int timer_callback(void *ctx)
{
    static uint32_t count = 0;
    /* 16K 16bit */
    int16_t *v_pcm = (int16_t *)test_pcm;
    int16_t wav = v_pcm[count++];
    if(count >= sizeof(test_pcm) / 2)
    {
        count = 0;
    }
    uint16_t v_wav = wav + 32768;
    double duty = v_wav / 65536.0;
    /* 16K * 60 HZ */
    pwm_set_frequency(TIMER_PWM, TIMER_PWM_CHN, 960000, duty);

    return 0;
}

int main(void)
{
    printf("PWM wav test\n");
    /* Init FPIOA pin mapping for PWM*/
    fpioa_set_function(24, FUNC_TIMER1_TOGGLE1);
    /* Init Platform-Level Interrupt Controller(PLIC) */
    plic_init();
    /* Enable global interrupt for machine mode of RISC-V */
    sysctl_enable_irq();
    /* Init timer */
    timer_init(TIMER_NOR);
    /* 16k sample rate */
    timer_set_interval(TIMER_NOR, TIMER_CHN, 62500);
    /* Set timer callback function with repeat method */
    timer_irq_register(TIMER_NOR, TIMER_CHN, 0, 1, timer_callback, NULL);
    /* Enable timer */
    timer_set_enable(TIMER_NOR, TIMER_CHN, 1);
    /* Init PWM */
    pwm_init(TIMER_PWM);
    /* Enable PWM */
    pwm_set_enable(TIMER_PWM, TIMER_PWM_CHN, 1);

    while(1)
        continue;

}
