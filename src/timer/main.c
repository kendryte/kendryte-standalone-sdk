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

static int ctx_table[TIMER_DEVICE_MAX * TIMER_CHANNEL_MAX];

/* Must use volatile variable */
volatile static int irq_count;

int timer_callback(void *ctx) {
    int number = *(int *)ctx;

    irq_count++;
    /* WARN: Do not use printf in interrupt, here just for demo */
    LOGD(__func__, "Timer callback [%d], count [%d]", number, irq_count);
    return 0;
}

int main(void)
{
    LOGI(__func__, "Kendryte "__DATE__ " " __TIME__);
    LOGI(__func__, "Timer test");

    /* Init context table */
    for(size_t i = 0; i < TIMER_DEVICE_MAX * TIMER_CHANNEL_MAX; i++)
    {
        ctx_table[i] = i;
    }

    /* Clear IRQ count */
    irq_count = 0;

    /* Init Platform-Level Interrupt Controller(PLIC) */
    plic_init();
    /* Enable global interrupt for machine mode of RISC-V */
    sysctl_enable_irq();

    for(size_t j = 0; j < TIMER_DEVICE_MAX; j++) {
        /* For every timer devices , Init timer */
        timer_init(j);
        for(size_t i = 0; i < TIMER_CHANNEL_MAX; i++) {
            /* Set timer interval to 500ms */
            timer_set_interval(j, i, 500000000);
            /* Set timer callback function with single shot method */
            timer_irq_register(j, i, 1, 1, timer_callback, &ctx_table[j * TIMER_CHANNEL_MAX + i]);
            /* Enable timer */
            timer_set_enable(j, i, 1);
            LOGD(__func__, "Timer enable (%ld, %ld)=>(%ld)", j, i, j * TIMER_CHANNEL_MAX + i);
        }
    }

    while (irq_count < 12)
        continue;

    LOGI("[PASS]", "Timer single shot test OK");

    /* Clear IRQ count */
    irq_count = 0;

    for(size_t j = 0; j < TIMER_DEVICE_MAX; j++) {
        /* For every timer devices , Init timer */
        timer_init(j);
        for(size_t i = 0; i < TIMER_CHANNEL_MAX; i++) {
            /* Set timer interval to 200ms */
            timer_set_interval(j, i, 200000000);
            /* Set timer callback function with repeat method */
            timer_irq_register(j, i, 0, 1, timer_callback, &ctx_table[j * TIMER_CHANNEL_MAX + i]);
            /* Enable timer */
            timer_set_enable(j, i, 1);
            LOGD(__func__, "Timer enable (%ld, %ld)=>(%ld)", j, i, j * TIMER_CHANNEL_MAX + i);
        }
    }

    while (irq_count < 50)
        continue;

    for(size_t j = 0; j < TIMER_DEVICE_MAX; j++) {
        /* For every timer devices */
        for(size_t i = 0; i < TIMER_CHANNEL_MAX; i++) {
            /* Disable timer */
            timer_set_enable(j, i, 0);
            /* Deregister every channel timer interrupt */
            timer_irq_unregister(j, i);
        }
    }

    LOGI("[PASS]", "Timer repeat test OK");

    return 0;
}
