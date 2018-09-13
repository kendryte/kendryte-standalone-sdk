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
#ifndef _SYS_CLOCK_H
#define _SYS_CLOCK_H
#include "stdint.h"

#define PLL0_OUTPUT_FREQ 320000000UL
#define PLL1_OUTPUT_FREQ 160000000UL
#define PLL2_OUTPUT_FREQ 45158400UL
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief       Init PLL freqency
 */
void sys_clock_init();

/**
 * @brief       Set frequency of CPU
 * @param[in]   frequency       The desired frequency in Hz
 *
 * @return      The actual frequency of CPU after set
 */
uint32_t system_set_cpu_frequency(uint32_t frequency);

#ifdef __cplusplus
}
#endif

#endif
