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
/**
 * @file
 * @brief      A real-time clock (RTC) is a computer clock that keeps track of
 *             the current time.
 */

#ifndef _DRIVER_RTC_H
#define _DRIVER_RTC_H

#include <stdint.h>
#include <time.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      RTC timer mode
 *
 *             Timer mode selector
 *             | Mode | Description            |
 *             |------|------------------------|
 *             | 0    | Timer pause            |
 *             | 1    | Timer time running     |
 *             | 2    | Timer time setting     |
 */
typedef enum _rtc_timer_mode_e
{
    /* 0: Timer pause */
    RTC_TIMER_PAUSE,
    /* 1: Timer time running */
    RTC_TIMER_RUNNING,
    /* 2: Timer time setting */
    RTC_TIMER_SETTING,
    /* Max count of this enum*/
    RTC_TIMER_MAX
} rtc_timer_mode_t;

/*
 * @brief      RTC tick interrupt mode
 *
 *             Tick interrupt mode selector
 *             | Mode | Description            |
 *             |------|------------------------|
 *             | 0    | Interrupt every second |
 *             | 1    | Interrupt every minute |
 *             | 2    | Interrupt every hour   |
 *             | 3    | Interrupt every day    |
 */
typedef enum _rtc_tick_interrupt_mode_e
{
    /* 0: Interrupt every second */
    RTC_INT_SECOND,
    /* 1: Interrupt every minute */
    RTC_INT_MINUTE,
    /* 2: Interrupt every hour */
    RTC_INT_HOUR,
    /* 3: Interrupt every day */
    RTC_INT_DAY,
    /* Max count of this enum*/
    RTC_INT_MAX
} rtc_tick_interrupt_mode_t;

/**
 * @brief      RTC mask structure
 *
 *             RTC mask structure for common use
 */
typedef struct _rtc_mask
{
    /* Reserved */
    uint32_t resv : 1;
    /* Second mask */
    uint32_t second : 1;
    /* Minute mask */
    uint32_t minute : 1;
    /* Hour mask */
    uint32_t hour : 1;
    /* Week mask */
    uint32_t week : 1;
    /* Day mask */
    uint32_t day : 1;
    /* Month mask */
    uint32_t month : 1;
    /* Year mask */
    uint32_t year : 1;
} __attribute__((packed, aligned(1))) rtc_mask_t;

/**
 * @brief       RTC register
 *
 * @note        RTC register table
 *
 * | Offset    | Name           | Description                         |
 * |-----------|----------------|-------------------------------------|
 * | 0x00      | date           | Timer date information              |
 * | 0x04      | time           | Timer time information              |
 * | 0x08      | alarm_date     | Alarm date information              |
 * | 0x0c      | alarm_time     | Alarm time information              |
 * | 0x10      | initial_count  | Timer counter initial value         |
 * | 0x14      | current_count  | Timer counter current value         |
 * | 0x18      | interrupt_ctrl | RTC interrupt settings              |
 * | 0x1c      | register_ctrl  | RTC register settings               |
 * | 0x20      | reserved0      | Reserved                            |
 * | 0x24      | reserved1      | Reserved                            |
 * | 0x28      | extended       | Timer extended information          |
 *
 */


/**
 * @brief       Timer date information
 *
 *              No. 0 Register (0x00)
 */
typedef struct _rtc_date
{
    /* Week. Range [0,6]. 0 is Sunday. */
    uint32_t week : 3;
    /* Reserved */
    uint32_t resv0 : 5;
    /* Day. Range [1,31] or [1,30] or [1,29] or [1,28] */
    uint32_t day : 5;
    /* Reserved */
    uint32_t resv1 : 3;
    /* Month. Range [1,12] */
    uint32_t month : 4;
    /* Year. Range [0,99] */
    uint32_t year : 12;
} __attribute__((packed, aligned(4))) rtc_date_t;

/**
 * @brief       Timer time information
 *
 *              No. 1 Register (0x04)
 */
typedef struct _rtc_time
{
    /* Reserved */
    uint32_t resv0 : 10;
    /* Second. Range [0,59] */
    uint32_t second : 6;
    /* Minute. Range [0,59] */
    uint32_t minute : 6;
    /* Reserved */
    uint32_t resv1 : 2;
    /* Hour. Range [0,23] */
    uint32_t hour : 5;
    /* Reserved */
    uint32_t resv2 : 3;
} __attribute__((packed, aligned(4))) rtc_time_t;

/**
 * @brief       Alarm date information
 *
 *              No. 2 Register (0x08)
 */
typedef struct _rtc_alarm_date
{
    /* Alarm Week. Range [0,6]. 0 is Sunday. */
    uint32_t week : 3;
    /* Reserved */
    uint32_t resv0 : 5;
    /* Alarm Day. Range [1,31] or [1,30] or [1,29] or [1,28] */
    uint32_t day : 5;
    /* Reserved */
    uint32_t resv1 : 3;
    /* Alarm Month. Range [1,12] */
    uint32_t month : 4;
    /* Alarm Year. Range [0,99] */
    uint32_t year : 12;
} __attribute__((packed, aligned(4))) rtc_alarm_date_t;

/**
 * @brief       Alarm time information
 *
 *              No. 3 Register (0x0c)
 */
typedef struct _rtc_alarm_time
{
    /* Reserved */
    uint32_t resv0 : 10;
    /* Alarm Second. Range [0,59] */
    uint32_t second : 6;
    /* Alarm Minute. Range [0,59] */
    uint32_t minute : 6;
    /* Reserved */
    uint32_t resv1 : 2;
    /* Alarm Hour. Range [0,23] */
    uint32_t hour : 5;
    /* Reserved */
    uint32_t resv2 : 3;
} __attribute__((packed, aligned(4))) rtc_alarm_time_t;

/**
 * @brief       Timer counter initial value
 *
 *              No. 4 Register (0x10)
 */
typedef struct _rtc_initial_count
{
    /* RTC counter initial value */
    uint32_t count : 32;
} __attribute__((packed, aligned(4))) rtc_initial_count_t;

/**
 * @brief       Timer counter current value
 *
 *              No. 5 Register (0x14)
 */
typedef struct _rtc_current_count
{
    /* RTC counter current value */
    uint32_t count : 32;
} __attribute__((packed, aligned(4))) rtc_current_count_t;

/**
 * @brief      RTC interrupt settings
 *
 *             No. 6 Register (0x18)
 */
typedef struct _rtc_interrupt_ctrl
{
    /* Reserved */
    uint32_t tick_enable : 1;
    /* Alarm interrupt enable */
    uint32_t alarm_enable : 1;
    /* Tick interrupt enable */
    uint32_t tick_int_mode : 2;
    /* Reserved */
    uint32_t resv : 20;
    /* Alarm compare mask for interrupt */
    uint32_t alarm_compare_mask : 8;
} __attribute__((packed, aligned(4))) rtc_interrupt_ctrl_t;

/**
 * @brief       RTC register settings
 *
 *              No. 7 Register (0x1c)
 */
typedef struct _rtc_register_ctrl
{
    /* RTC timer read enable */
    uint32_t read_enable : 1;
    /* RTC timer write enable */
    uint32_t write_enable : 1;
    /* Reserved */
    uint32_t resv0 : 11;
    /* RTC timer mask */
    uint32_t timer_mask : 8;
    /* RTC alarm mask */
    uint32_t alarm_mask : 8;
    /* RTC counter initial count value mask */
    uint32_t initial_count_mask : 1;
    /* RTC interrupt register mask */
    uint32_t interrupt_register_mask : 1;
    /* Reserved */
    uint32_t resv1 : 1;
} __attribute__((packed, aligned(4))) rtc_register_ctrl_t;

/**
 * @brief       Reserved
 *
 *              No. 8 Register (0x20)
 */
typedef struct _rtc_reserved0
{
    /* Reserved */
    uint32_t resv : 32;
} __attribute__((packed, aligned(4))) rtc_reserved0_t;

/**
 * @brief      Reserved
 *
 *             No. 9 Register (0x24)
 */
typedef struct _rtc_reserved1
{
    /* Reserved */
    uint32_t resv : 32;
} __attribute__((packed, aligned(4))) rtc_reserved1_t;

/**
 * @brief      Timer extended information
 *
 *             No. 10 Register (0x28)
 */
typedef struct _rtc_extended
{
    /* Century. Range [0,31] */
    uint32_t century : 5;
    /* Is leap year. 1 is leap year, 0 is not leap year */
    uint32_t leap_year : 1;
    /* Reserved */
    uint32_t resv : 26;
} __attribute__((packed, aligned(4))) rtc_extended_t;


/**
 * @brief       Real-time clock struct
 *
 *              A real-time clock (RTC) is a computer clock that keeps track of
 *              the current time.
 */
typedef struct _rtc
{
    /* No. 0 (0x00): Timer date information */
    rtc_date_t date;
    /* No. 1 (0x04): Timer time information */
    rtc_time_t time;
    /* No. 2 (0x08): Alarm date information */
    rtc_alarm_date_t alarm_date;
    /* No. 3 (0x0c): Alarm time information */
    rtc_alarm_time_t alarm_time;
    /* No. 4 (0x10): Timer counter initial value */
    rtc_initial_count_t initial_count;
    /* No. 5 (0x14): Timer counter current value */
    rtc_current_count_t current_count;
    /* No. 6 (0x18): RTC interrupt settings */
    rtc_interrupt_ctrl_t interrupt_ctrl;
    /* No. 7 (0x1c): RTC register settings */
    rtc_register_ctrl_t register_ctrl;
    /* No. 8 (0x20): Reserved */
    rtc_reserved0_t reserved0;
    /* No. 9 (0x24): Reserved */
    rtc_reserved1_t reserved1;
    /* No. 10 (0x28): Timer extended information */
    rtc_extended_t extended;
} __attribute__((packed, aligned(4))) rtc_t;


/**
 * @brief       Real-time clock object
 */
extern volatile rtc_t *const rtc;
extern volatile uint32_t *const rtc_base;

/**
 * @brief       Set date time to RTC
 *
 * @param[in]   year        The year
 * @param[in]   month       The month
 * @param[in]   day         The day
 * @param[in]   hour        The hour
 * @param[in]   minute      The minute
 * @param[in]   second      The second
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int rtc_timer_set(int year, int month, int day, int hour, int minute, int second);

/**
 * @brief       Get date time from RTC
 *
 * @param       year        The year
 * @param       month       The month
 * @param       day         The day
 * @param       hour        The hour
 * @param       minute      The minute
 * @param       second      The second
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int rtc_timer_get(int *year, int *month, int *day, int *hour, int *minute, int *second);

/**
 * @brief       Initialize RTC
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int rtc_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_RTC_H */
