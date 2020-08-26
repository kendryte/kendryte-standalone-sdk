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
#include "pwm_play_audio.h"
#include <unistd.h>
#include <stdlib.h>

#include "test_wav.h"
#include "test_8bit_wav.h"
#include "test_16bit_wav.h"
#include "test_24bit_wav.h"
#include "test_16bit_mono_wav.h"
#include "test_welcome.h"

#define TIMER_NOR   0
#define TIMER_CHN   1
#define TIMER_PWM   0
#define TIMER_PWM_CHN 0

int main(void)
{
    printf("PWM wav test\n");
    /* Init FPIOA pin mapping for PWM*/
    fpioa_set_function(24, FUNC_TIMER0_TOGGLE1);
    /* Init Platform-Level Interrupt Controller(PLIC) */
    plic_init();
    /* Enable global interrupt for machine mode of RISC-V */
    sysctl_enable_irq();

    pwm_play_init(TIMER_NOR, TIMER_PWM);

    while(1)
    {
//        pwm_play_wav(TIMER_NOR, TIMER_CHN, TIMER_PWM, TIMER_PWM_CHN, test_wav, 0);
//        pwm_play_wav(TIMER_NOR, TIMER_CHN, TIMER_PWM, TIMER_PWM_CHN, test_8bit_wav, 0);
//        pwm_play_wav(TIMER_NOR, TIMER_CHN, TIMER_PWM, TIMER_PWM_CHN, test_16bit_mono_wav, 0);
//        pwm_play_wav(TIMER_NOR, TIMER_CHN, TIMER_PWM, TIMER_PWM_CHN, test_16bit_wav, 0);
//        pwm_play_wav(TIMER_NOR, TIMER_CHN, TIMER_PWM, TIMER_PWM_CHN, test_24bit_wav, 0);
        pwm_play_wav(TIMER_NOR, TIMER_CHN, TIMER_PWM, TIMER_PWM_CHN, test_welcome_wav, 0);
    }
}
