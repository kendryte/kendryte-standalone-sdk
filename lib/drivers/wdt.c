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
#include "wdt.h"
#include "platform.h"
#include "stddef.h"
#include "common.h"
#include "sysctl.h"
#include "plic.h"

plic_irq_callback_t wdt_irq[2];

volatile wdt_t *const wdt[2] =
{
    (volatile wdt_t *)WDT0_BASE_ADDR,
    (volatile wdt_t *)WDT1_BASE_ADDR
};

void wdt_feed(uint8_t id)
{
    wdt[id]->crr = WDT_CRR_MASK;
}

void wdt_enable(uint8_t id)
{
    wdt[id]->crr = WDT_CRR_MASK;
    wdt[id]->cr |= WDT_CR_ENABLE;
}

void wdt_disable(uint8_t id)
{
    wdt[id]->crr = WDT_CRR_MASK;
    wdt[id]->cr &= (~WDT_CR_ENABLE);
}

void wdt_timeout_set(uint8_t id, uint8_t timeout)
{
    wdt[id]->torr = WDT_TORR_TOP(timeout);
}

void wdt_response_mode(uint8_t id, uint8_t mode)
{
    wdt[id]->cr &= (~WDT_CR_RMOD_MASK);
    wdt[id]->cr |= mode;
}

void wdt_interrupt_clear(uint8_t id)
{
    wdt[id]->eoi = wdt[id]->eoi;
}

void wdt_set_irq(uint8_t id, plic_irq_callback_t on_irq)
{
    wdt_irq[id] = on_irq;
}

size_t wdt_get_pclk(uint8_t id)
{
    return id ? sysctl_clock_get_freq(SYSCTL_CLOCK_WDT1) : sysctl_clock_get_freq(SYSCTL_CLOCK_WDT0);
}

ssize_t log_2(size_t x)
{
    ssize_t i = 0;
    for (i = sizeof(size_t) * 8; i >= 0; i--)
    {
        if ((x >> i) & 0x1)
        {
            break;
        }
    }
    return i;
}

uint8_t wdt_get_top(uint8_t id, size_t timeout_ms)
{
    size_t wdt_clk = wdt_get_pclk(id);
    size_t ret = (timeout_ms * wdt_clk / 1000) >> 16;
    if (ret)
        ret = log_2(ret);
    if (ret > 0xf)
        ret = 0xf;
    return (uint8_t)ret;
}


int wdt_start(uint8_t id, size_t toms)
{
    wdt_disable(id);
    wdt_interrupt_clear(id);
    plic_irq_register(id ? IRQN_WDT1_INTERRUPT : IRQN_WDT0_INTERRUPT, wdt_irq[id], NULL);
    plic_set_priority(id ? IRQN_WDT1_INTERRUPT : IRQN_WDT0_INTERRUPT, 1);
    plic_irq_enable(id ? IRQN_WDT1_INTERRUPT : IRQN_WDT0_INTERRUPT);

    sysctl_reset(id ? SYSCTL_RESET_WDT1 : SYSCTL_RESET_WDT0);
    sysctl_clock_set_threshold(id ? SYSCTL_THRESHOLD_WDT1 : SYSCTL_THRESHOLD_WDT0, 0);
    sysctl_clock_enable(id ? SYSCTL_CLOCK_WDT1 : SYSCTL_CLOCK_WDT0);
    wdt_response_mode(id, WDT_CR_RMOD_INTERRUPT);
    uint8_t m_top = wdt_get_top(id, toms);
    wdt_timeout_set(id, m_top);
    wdt_enable(id);
    return 0;
}

void wdt_stop(uint8_t id)
{
    wdt_disable(id);
}

