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
#ifndef _DRIVER_PWM_H
#define _DRIVER_PWM_H

#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief       Init pwm timer
 *
 * @param[in]   timer       timer
 */
void pwm_init(uint32_t tim);

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

#endif /* _DRIVER_PWM_H */
