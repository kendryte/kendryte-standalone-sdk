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

#include <stdlib.h>
#include "atomic.h"
#include "clint.h"
#include "dmac.h"
#include "entry.h"
#include "fpioa.h"
#include "platform.h"
#include "plic.h"
#include "sysctl.h"
#include "syslog.h"
#include "uarths.h"

#define PLL0_OUTPUT_FREQ 320000000UL
#define PLL1_OUTPUT_FREQ 160000000UL
#define PLL2_OUTPUT_FREQ 45158400UL

volatile char * const ram = (volatile char*)RAM_BASE_ADDR;

extern char _heap_start[];
extern char _heap_end[];

static volatile uint32_t g_wake_up[2] = { 1, 0 };

void thread_entry(int core_id)
{
    while (!atomic_read(&g_wake_up[core_id]));
}

void core_enable(int core_id)
{
    clint_ipi_send(core_id);
    atomic_set(&g_wake_up[core_id], 1);
}

int __attribute__((weak)) os_entry(int core_id, int number_of_cores, int (*user_main)(int, char**))
{
    /* Call main if there is no OS */
    return user_main(0, 0);
}

void _init_bsp(int core_id, int number_of_cores)
{
    extern int main(int argc, char* argv[]);
    extern void __libc_init_array(void);
    extern void __libc_fini_array(void);
    /* Initialize thread local data */
    init_tls();

    if (core_id == 0)
    {
        /* Initialize bss data to 0 */
        init_bss();
        /* Init FPIOA */
        fpioa_init();
        /* PLL init */
        sysctl_set_pll_frequency(PLL0_OUTPUT_FREQ, PLL1_OUTPUT_FREQ, PLL2_OUTPUT_FREQ);
        /* Init UART */
        uarths_init();
        /* Dmac init */
        dmac_init();
        /* Plic init */
        plic_init();
        /* Register finalization function */
        atexit(__libc_fini_array);
        /* Init libc array for C++ */
        __libc_init_array();
    }
    else
    {
        thread_entry(core_id);
    }

    if (core_id == 0)
    {
        /* Enable Core 1 to run main */
        core_enable(1);
    }

    int ret = os_entry(core_id, number_of_cores, main);

    exit(ret);
}

