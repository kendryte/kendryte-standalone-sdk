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

#define TIMER_NOR (0)
#define TIMER_CHN (0)
#define TIMER_PWM (1)
#define TIMER_PWM_CHN0 (0)
#define TIMER_PWM_CHN1 (1)

#define SERVO_FREQ (50)
#define SERVO0_DUTY_MAX (0.125)
#define SERVO0_DUTY_MIN (0.025)
#define SERVO1_DUTY_MAX (0.12)
#define SERVO1_DUTY_MIN (0.03)

#define SERVO0_STEP (0.001)
#define SERVO1_STEP (0.001)

int timer_callback(void *ctx)
{
    static int count = 0;
    static double cnt0 = (SERVO0_DUTY_MAX + SERVO0_DUTY_MIN) / 2;
    static double cnt1 = (SERVO1_DUTY_MAX + SERVO1_DUTY_MIN) / 2;
    static int flag0 = 0;
    static int flag1 = 0;

    count++;
    if (count % 5 == 0)
    {
        pwm_set_frequency(TIMER_PWM, TIMER_PWM_CHN0, SERVO_FREQ, cnt0);
        pwm_set_frequency(TIMER_PWM, TIMER_PWM_CHN1, SERVO_FREQ, cnt1);

        flag0 ? (cnt0 -= SERVO0_STEP) : (cnt0 += SERVO0_STEP);
        flag1 ? (cnt1 -= SERVO1_STEP) : (cnt1 += SERVO1_STEP);
        if (cnt0 > SERVO0_DUTY_MAX)
        {
            cnt0 = SERVO0_DUTY_MAX;
            flag0 = 1;
        }
        else if (cnt0 < SERVO0_DUTY_MIN)
        {
            cnt0 = SERVO0_DUTY_MIN;
            flag0 = 0;
        }

        if (cnt1 > SERVO1_DUTY_MAX)
        {
            cnt1 = SERVO1_DUTY_MAX;
            flag1 = 1;
        }
        else if (cnt1 < SERVO1_DUTY_MIN)
        {
            cnt1 = SERVO1_DUTY_MIN;
            flag1 = 0;
        }
        printf("Duty is %g, %g\n", cnt0 * 100, cnt1 * 100);
    }
    return 0;
}

int main(void)
{
    LOGI(__func__, "Kendryte "__DATE__ " " __TIME__);
    LOGI(__func__, "PWM Servo test");
    /* Init FPIOA pin mapping */
    fpioa_set_function(9, FUNC_TIMER1_TOGGLE1);
    fpioa_set_function(10, FUNC_TIMER1_TOGGLE2);
    /* Init Platform-Level Interrupt Controller(PLIC) */
    plic_init();
    /* Enable global interrupt for machine mode of RISC-V */
    sysctl_enable_irq();
    /* Init timer */
    timer_init(TIMER_NOR);
    /* Set timer interval to 10ms */
    timer_set_interval(TIMER_NOR, TIMER_CHN, 10000000);
    /* Set timer callback function with repeat method */
    timer_irq_register(TIMER_NOR, TIMER_CHN, 0, 1, timer_callback, NULL);
    /* Enable timer */
    timer_set_enable(TIMER_NOR, TIMER_CHN, 1);
    /* Init PWM */
    pwm_init(TIMER_PWM);
    /* Set PWM to 50Hz */
    pwm_set_frequency(TIMER_PWM, TIMER_PWM_CHN0, SERVO_FREQ, (SERVO0_DUTY_MAX + SERVO0_DUTY_MIN) / 2);
    pwm_set_enable(TIMER_PWM, TIMER_PWM_CHN0, 1);
    /* Set PWM to 50Hz */
    pwm_set_frequency(TIMER_PWM, TIMER_PWM_CHN1, SERVO_FREQ, (SERVO1_DUTY_MAX + SERVO1_DUTY_MIN) / 2);
    pwm_set_enable(TIMER_PWM, TIMER_PWM_CHN1, 1);

    while (1)
        continue;
}
