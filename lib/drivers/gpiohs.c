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
#include "gpiohs.h"
#include "utils.h"
#include "fpioa.h"
#include "sysctl.h"
#define GPIOHS_MAX_PINNO 32

volatile gpiohs_t* const gpiohs = (volatile gpiohs_t*)GPIOHS_BASE_ADDR;

typedef struct _gpiohs_pin_context
{
    size_t pin;
    gpio_pin_edge_t edge;
    void (*callback)();
} gpiohs_pin_context;

gpiohs_pin_context pin_context[32];

void gpiohs_set_drive_mode(uint8_t pin, gpio_drive_mode_t mode)
{
    configASSERT(pin < GPIOHS_MAX_PINNO);
    int io_number = fpioa_get_io_by_function(FUNC_GPIOHS0 + pin);
    configASSERT(io_number > 0);

    fpioa_pull_t pull;
    uint32_t dir;

    switch (mode)
    {
    case GPIO_DM_INPUT:
        pull = FPIOA_PULL_NONE;
        dir = 0;
        break;
    case GPIO_DM_INPUT_PULL_DOWN:
        pull = FPIOA_PULL_DOWN;
        dir = 0;
        break;
    case GPIO_DM_INPUT_PULL_UP:
        pull = FPIOA_PULL_UP;
        dir = 0;
        break;
    case GPIO_DM_OUTPUT:
        pull = FPIOA_PULL_DOWN;
        dir = 1;
        break;
    default:
        configASSERT(!"GPIO drive mode is not supported.") break;
    }

    fpioa_set_io_pull(io_number, pull);
    volatile uint32_t *reg = dir ? gpiohs->output_en.u32 : gpiohs->input_en.u32;
    volatile uint32_t *reg_d = !dir ? gpiohs->output_en.u32 : gpiohs->input_en.u32;
    set_gpio_bit(reg_d, pin, 0);
    set_gpio_bit(reg, pin, 1);
}

gpio_pin_value_t gpiohs_get_pin(uint8_t pin)
{
    configASSERT(pin < GPIOHS_MAX_PINNO);
    return get_gpio_bit(gpiohs->input_val.u32, pin);
}

void gpiohs_set_pin(uint8_t pin, gpio_pin_value_t value)
{
    configASSERT(pin < GPIOHS_MAX_PINNO);
    set_gpio_bit(gpiohs->output_val.u32, pin, value);
}

void gpiohs_set_pin_edge(uint8_t pin, gpio_pin_edge_t edge)
{
    uint32_t rise, fall, irq;
    switch (edge)
    {
    case GPIO_PE_NONE:
        rise = fall = irq = 0;
        break;
    case GPIO_PE_FALLING:
        rise = 0;
        fall = irq = 1;
        break;
    case GPIO_PE_RISING:
        fall = 0;
        rise = irq = 1;
        break;
    case GPIO_PE_BOTH:
        rise = fall = irq = 1;
        break;
    default:
        configASSERT(!"Invalid gpio edge");
        break;
    }

    set_gpio_bit(gpiohs->rise_ie.u32, pin, rise);
    set_gpio_bit(gpiohs->fall_ie.u32, pin, fall);
    pin_context[pin].edge = edge;
}

int gpiohs_pin_onchange_isr(void *userdata)
{
    gpiohs_pin_context *ctx = (gpiohs_pin_context *)userdata;
    size_t pin = ctx->pin;
    uint32_t rise, fall;
    switch (ctx->edge)
    {
    case GPIO_PE_NONE:
        rise = fall = 0;
        break;
    case GPIO_PE_FALLING:
        rise = 0;
        fall = 1;
        break;
    case GPIO_PE_RISING:
        fall = 0;
        rise = 1;
        break;
    case GPIO_PE_BOTH:
        rise = fall = 1;
        break;
    default:
        configASSERT(!"Invalid gpio edge");
        break;
    }

    if (rise)
    {
        set_gpio_bit(gpiohs->rise_ie.u32, pin, 0);
        set_gpio_bit(gpiohs->rise_ip.u32, pin, 1);
        set_gpio_bit(gpiohs->rise_ie.u32, pin, 1);
    }

    if (fall)
    {
        set_gpio_bit(gpiohs->fall_ie.u32, pin, 0);
        set_gpio_bit(gpiohs->fall_ip.u32, pin, 1);
        set_gpio_bit(gpiohs->fall_ie.u32, pin, 1);
    }

    if (ctx->callback)
        ctx->callback();
    return 0;
}

void gpiohs_set_irq(uint8_t pin, uint32_t priority, void (*func)())
{

    pin_context[pin].pin = pin;
    pin_context[pin].callback = func;

    plic_set_priority(IRQN_GPIOHS0_INTERRUPT + pin, priority);
    plic_irq_register(IRQN_GPIOHS0_INTERRUPT + pin, gpiohs_pin_onchange_isr, &(pin_context[pin]));
    plic_irq_enable(IRQN_GPIOHS0_INTERRUPT + pin);
}

void gpiohs_irq_disable(size_t pin)
{
    plic_irq_disable(IRQN_GPIOHS0_INTERRUPT + pin);
}

