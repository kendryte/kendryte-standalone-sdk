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
#include <unistd.h>
#include "wdt.h"
#include "sysctl.h"

int wdt0_irq(void *ctx)
{
    static int s_wdt_irq_cnt = 0;
    printf("%s\n", __func__);
    s_wdt_irq_cnt ++;
    if(s_wdt_irq_cnt < 2)
        wdt_clear_interrupt(0);
    else
        while(1);
    return 0;
}

int main(void)
{
    printf("wdt start!\n");
    int timeout = 0;
    plic_init();
    sysctl_enable_irq();
    printf("wdt time is %d ms\n", wdt_init(0, 2000, wdt0_irq,NULL));
    while(1)
    {
        sleep(1);
        if(timeout++ < 5)
        {
            wdt_feed(0);
        }
    }
}

