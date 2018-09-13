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
#ifndef _DRIVER_TIMER_H
#define _DRIVER_TIMER_H

#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
struct timer_channel_t
{
    /* TIMER_N Load Count Register              (0x00+(N-1)*0x14) */
    volatile uint32_t load_count;
    /* TIMER_N Current Value Register           (0x04+(N-1)*0x14) */
    volatile uint32_t current_value;
    /* TIMER_N Control Register                 (0x08+(N-1)*0x14) */
    volatile uint32_t control;
    /* TIMER_N Interrupt Clear Register         (0x0c+(N-1)*0x14) */
    volatile uint32_t eoi;
    /* TIMER_N Interrupt Status Register        (0x10+(N-1)*0x14) */
    volatile uint32_t intr_stat;
} __attribute__((packed, aligned(4)));

struct timer_t
{
    /* TIMER_N Register                         (0x00-0x4c) */
    volatile struct timer_channel_t channel[4];
    /* reserverd                                (0x50-0x9c) */
    volatile uint32_t resv1[20];
    /* TIMER Interrupt Status Register          (0xa0) */
    volatile uint32_t intr_stat;
    /* TIMER Interrupt Clear Register           (0xa4) */
    volatile uint32_t eoi;
    /* TIMER Raw Interrupt Status Register      (0xa8) */
    volatile uint32_t raw_intr_stat;
    /* TIMER Component Version Register         (0xac) */
    volatile uint32_t comp_version;
    /* TIMER_N Load Count2 Register             (0xb0-0xbc) */
    volatile uint32_t load_count2[4];
} __attribute__((packed, aligned(4)));

/* TIMER Control Register */
#define TIMER_CR_ENABLE             0x00000001
#define TIMER_CR_MODE_MASK          0x00000002
#define TIMER_CR_FREE_MODE          0x00000000
#define TIMER_CR_USER_MODE          0x00000002
#define TIMER_CR_INTERRUPT_MASK     0x00000004
#define TIMER_CR_PWM_ENABLE         0x00000008
/* clang-format on */

extern volatile struct timer_t *const timer[3];

/**
 * @brief       Set timer clock frequency
 *
 * @param[in]   timer       timer
 * @param[in]   div         clock divide value
 */
void timer_set_clock_div(uint32_t timer, uint32_t div);

/**
 * @brief       Enable timer channel
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 */
void timer_enable(uint32_t timer, uint32_t channel);

/**
 * @brief       Disable timer channel
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 */
void timer_disable(uint32_t timer, uint32_t channel);

/**
 * @brief       Enable timer channel PWM
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 */
void timer_enable_pwm(uint32_t timer, uint32_t channel);

/**
 * @brief       Disable timer channel PWM
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 */
void timer_disable_pwm(uint32_t timer, uint32_t channel);

/**
 * @brief       Enable timer channel interrupt
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 */
void timer_enable_interrupt(uint32_t timer, uint32_t channel);

/**
 * @brief       Disable timer channel interrupt
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 */
void timer_disable_interrupt(uint32_t timer, uint32_t channel);

/**
 * @brief       Set timer channel mode
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 * @param[in]   mode        mode
 */
void timer_set_mode(uint32_t timer, uint32_t channel, uint32_t mode);

/**
 * @brief       Set timer channel reload value
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 * @param[in]   count       count
 */
void timer_set_reload(uint32_t timer, uint32_t channel, uint32_t count);

/**
 * @brief       Set timer channel reload value2
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 * @param[in]   count       count
 */
void timer_set_reload2(uint32_t timer, uint32_t channel, uint32_t count);

/**
 * @brief       Get timer channel count
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 *
 * @return      current value
 */
uint32_t timer_get_count(uint32_t timer, uint32_t channel);

/**
 * @brief       Get timer channel reload value
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 *
 * @return      reload value
 */
uint32_t timer_get_reload(uint32_t timer, uint32_t channel);

/**
 * @brief       Get timer channel reload value2
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 *
 * @return      reload value2
 */
uint32_t timer_get_reload2(uint32_t timer, uint32_t channel);

/**
 * @brief       Get timer interrupt status
 *
 * @param[in]   timer       timer
 *
 * @return      interrupt status
 */
uint32_t timer_get_interrupt_status(uint32_t timer);

/**
 * @brief       Get timer raw interrupt status
 *
 * @param[in]   timer    timer
 *
 * @return      raw interrupt status
 */
uint32_t timer_get_raw_interrupt_status(uint32_t timer);

/**
 * @brief       Get timer interrupt status
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 *
 * @return      interrupt status
 */
uint32_t timer_channel_get_interrupt_status(uint32_t timer, uint32_t channel);

/**
 * @brief       Clear interrupt
 *
 * @param[in]   timer       timer
 */
void timer_clear_interrupt(uint32_t timer);

/**
 * @brief       Clear interrupt
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 */
void timer_channel_clear_interrupt(uint32_t timer, uint32_t channel);


/**
 * @brief       Set timer timeout
 *
 * @param[in]   timer           timer
 * @param[in]   channel         channel
 * @param[in]   nanoseconds     timeout
 *
 * @return      the real timeout
 */
size_t timer_set_interval(uint32_t timer, uint32_t channel, size_t nanoseconds);

/**
 * @brief       Init timer
 *
 * @param[in]   timer       timer
 */
void timer_init(uint32_t timer);

/**
 * @brief       Set timer timeout function
 *
 * @param[in]   timer           timer
 * @param[in]   channel         channel
 * @param[in]   func            timeout function
 * @param[in]   priority        interrupt priority
 *
 */
void timer_set_irq(uint32_t timer, uint32_t channel, void(*func)(),  uint32_t priority);

/**
 * @brief       Enable timer
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 * @param[in]   enable      Enable or disable
 *
 */
void timer_set_enable(uint32_t timer, uint32_t channel, uint32_t enable);

/**
 * @brief       Enable timer
 *
 * @param[in]   timer       timer
 * @param[in]   channel     channel
 * @param[in]   enable      Enable or disable
 *
 */
void pwm_set_enable(uint32_t timer, uint32_t channel, int enable);

/**
 * @brief       Set pwm duty
 *
 * @param[in]   timer           timer
 * @param[in]   channel         channel
 * @param[in]   frequency       pwm frequency
 * @param[in]   duty            duty
 *
 */
double pwm_set_frequency(uint32_t timer, uint32_t channel, double frequency, double duty);


#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_TIMER_H */
