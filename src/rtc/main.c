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
#include <time.h>
#include <unistd.h>
#include "rtc.h"


void get_date_time(bool alarm)
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    rtc_timer_get(&year, &month, &day, &hour, &minute, &second);
    if (!alarm)
        printf("%4d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, minute, second);
    else
        printf("Alarm at --> %4d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, minute, second);
}

int on_timer_interrupt(void *ctx)
{
    get_date_time(false);
	return 0;
}

int on_alarm_interrupt(void *ctx)
{
    get_date_time(true);
	return 0;
}

int main(void)
{
    rtc_init();
    rtc_timer_set(2018, 9, 12, 23, 30, 50);
    rtc_alarm_set(2018, 9, 12, 23, 31, 00);

    printf("RTC Tick and Alarm Test\n" "Compiled in " __DATE__ " " __TIME__ "\n");

    rtc_tick_irq_register(
        false,
        RTC_INT_SECOND,
        on_timer_interrupt,
        NULL,
        1
    );

    rtc_alarm_irq_register(
        false,
        (rtc_mask_t) {
            .second = 0, /* Second mask */
            .minute = 1, /* Minute mask */
            .hour = 0,   /* Hour mask */
            .week = 0,   /* Week mask */
            .day = 0,    /* Day mask */
            .month = 0,  /* Month mask */
            .year = 0,   /* Year mask */
        },
        on_alarm_interrupt,
        NULL,
        1
    );

    while(1)
    {
        sleep(1);
        // get_date_time();
    }
    return 0;
}
