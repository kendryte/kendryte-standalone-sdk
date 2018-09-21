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
#ifndef _GPIO_COMMON_H
#define _GPIO_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _gpio_drive_mode
{
    GPIO_DM_Input,
    GPIO_DM_InputPullDown,
    GPIO_DM_InputPullUp,
    GPIO_DM_Output,
    GPIO_DM_OutputOpenDrain,
    GPIO_DM_OutputOpenDrainPullUp,
    GPIO_DM_OutputOpenSource,
    GPIO_DM_OutputOpenSourcePullDown
} gpio_drive_mode_t;

typedef enum _gpio_pin_edge
{
    GPIO_PE_None,
    GPIO_PE_Falling,
    GPIO_PE_Rising,
    GPIO_PE_Both
} gpio_pin_edge_t;

typedef enum _gpio_pin_value
{
    GPIO_PV_Low,
    GPIO_PV_High
} gpio_pin_value_t;

#ifdef __cplusplus
}
#endif

#endif /* _GPIO_COMMON_H */

