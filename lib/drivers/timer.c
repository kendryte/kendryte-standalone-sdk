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
#include "timer.h"
#include "sysctl.h"
#include "stddef.h"
#include "common.h"
#include "plic.h"
#include "io.h"

volatile kendryte_timer_t *const timer[3] =
{
    (volatile kendryte_timer_t *)TIMER0_BASE_ADDR,
    (volatile kendryte_timer_t *)TIMER1_BASE_ADDR,
    (volatile kendryte_timer_t *)TIMER2_BASE_ADDR
};

void timer_init(uint32_t tim)
{
    sysctl_clock_enable(SYSCTL_CLOCK_TIMER0 + tim);
}

void timer_set_clock_div(uint32_t tim, uint32_t div)
{
    sysctl_clock_set_threshold(tim == 0 ? SYSCTL_THRESHOLD_TIMER0 :
        tim == 1 ? SYSCTL_THRESHOLD_TIMER1 :
        SYSCTL_THRESHOLD_TIMER2, div);
}

void timer_enable(uint32_t tim, uint32_t channel)
{
    timer[tim]->channel[channel].control |= TIMER_CR_ENABLE;
}

void timer_disable(uint32_t tim, uint32_t channel)
{
    timer[tim]->channel[channel].control &= (~TIMER_CR_ENABLE);
}

void timer_enable_pwm(uint32_t tim, uint32_t channel)
{
    timer[tim]->channel[channel].control |= TIMER_CR_PWM_ENABLE;
}

void timer_disable_pwm(uint32_t tim, uint32_t channel)
{
    timer[tim]->channel[channel].control &= (~TIMER_CR_PWM_ENABLE);
}

void timer_enable_interrupt(uint32_t tim, uint32_t channel)
{
    timer[tim]->channel[channel].control &= (~TIMER_CR_INTERRUPT_MASK);
}

void timer_disable_interrupt(uint32_t tim, uint32_t channel)
{
    timer[tim]->channel[channel].control |= TIMER_CR_INTERRUPT_MASK;
}

void timer_set_mode(uint32_t tim, uint32_t channel, uint32_t mode)
{
    timer[tim]->channel[channel].control &= (~TIMER_CR_MODE_MASK);
    timer[tim]->channel[channel].control |= mode;
}

void timer_set_reload(uint32_t tim, uint32_t channel, uint32_t count)
{
    timer[tim]->channel[channel].load_count = count;
}

void timer_set_reload2(uint32_t tim, uint32_t channel, uint32_t count)
{
    timer[tim]->load_count2[channel] = count;
}

uint32_t timer_get_count(uint32_t tim, uint32_t channel)
{
    return timer[tim]->channel[channel].current_value;
}

uint32_t timer_get_reload(uint32_t tim, uint32_t channel)
{
    return timer[tim]->channel[channel].load_count;
}

uint32_t timer_get_reload2(uint32_t tim, uint32_t channel)
{
    return timer[tim]->load_count2[channel];
}

uint32_t timer_get_interrupt_status(uint32_t tim)
{
    return timer[tim]->intr_stat;
}

uint32_t timer_get_raw_interrupt_status(uint32_t tim)
{
    return timer[tim]->raw_intr_stat;
}

uint32_t timer_channel_get_interrupt_status(uint32_t tim, uint32_t channel)
{
    return timer[tim]->channel[channel].intr_stat;
}

void timer_clear_interrupt(uint32_t tim)
{
    timer[tim]->eoi = timer[tim]->eoi;
}

void timer_channel_clear_interrupt(uint32_t tim, uint32_t channel)
{
    timer[tim]->channel[channel].eoi = timer[tim]->channel[channel].eoi;
}

void timer_set_enable(uint32_t tim, uint32_t channel, uint32_t enable){
    if (enable)
        timer[tim]->channel[channel].control = TIMER_CR_USER_MODE | TIMER_CR_ENABLE;
    else
        timer[tim]->channel[channel].control = TIMER_CR_INTERRUPT_MASK;
}

size_t timer_set_interval(uint32_t tim, uint32_t channel, size_t nanoseconds)
{
    uint32_t clk_freq = sysctl_clock_get_freq(SYSCTL_CLOCK_TIMER0 + tim);

    double min_step = 1e9 / clk_freq;
    size_t value = (size_t)(nanoseconds / min_step);
    configASSERT(value > 0 && value < UINT32_MAX);
    timer[tim]->channel[channel].load_count = (uint32_t)value;
    return (size_t)(min_step * value);
}

typedef void(*timer_ontick)();
timer_ontick time_irq[3][4] = { NULL };

static int timer_isr(void *parm)
{
    uint32_t tim;
    for (tim = 0; tim < 3; tim++)
    {
        if (parm == timer[tim])
            break;
    }

    uint32_t channel = timer[tim]->intr_stat;
    size_t i = 0;
    for (i = 0; i < 4; i++)
    {
        if (channel & 1)
        {
            if (time_irq[tim][i])
                (time_irq[tim][i])();
            break;
        }

        channel >>= 1;
    }

    readl(&timer[tim]->eoi);
    return 0;
}

void timer_set_irq(uint32_t tim, uint32_t channel, void(*func)(), uint32_t priority)
{
    time_irq[tim][channel] = func;
    if (channel < 2)
    {
        plic_set_priority(IRQN_TIMER0A_INTERRUPT + tim * 2, priority);
        plic_irq_register(IRQN_TIMER0A_INTERRUPT + tim * 2, timer_isr, (void *)timer[tim]);
        plic_irq_enable(IRQN_TIMER0A_INTERRUPT + tim * 2);
    }
    else
    {
        plic_set_priority(IRQN_TIMER0B_INTERRUPT + tim * 2, priority);
        plic_irq_register(IRQN_TIMER0B_INTERRUPT + tim * 2, timer_isr, NULL);
        plic_irq_enable(IRQN_TIMER0B_INTERRUPT + tim * 2);
    }
}

void pwm_set_enable(uint32_t tim, uint32_t channel, int enable)
{
    if (enable)
        timer[tim]->channel[channel].control = TIMER_CR_INTERRUPT_MASK | TIMER_CR_PWM_ENABLE | TIMER_CR_USER_MODE | TIMER_CR_ENABLE;
    else
        timer[tim]->channel[channel].control = TIMER_CR_INTERRUPT_MASK;
}

double pwm_set_frequency(uint32_t tim, uint32_t channel, double frequency, double duty)
{
    uint32_t clk_freq = sysctl_clock_get_freq(SYSCTL_CLOCK_TIMER0 + tim);

    int32_t periods = (int32_t)(clk_freq / frequency);
    configASSERT(periods > 0 && periods <= INT32_MAX);
    frequency = clk_freq / (double)periods;

    uint32_t percent = (uint32_t)(duty * periods);
    timer[tim]->channel[channel].load_count = periods - percent;
    timer[tim]->load_count2[channel] = percent;

    return frequency;
}

