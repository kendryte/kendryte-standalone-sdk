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


#ifndef _FPIOA_CFG_H
#define _FPIOA_CFG_H

#include <fpioa.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fpioa_cfg_t
{
    uint32_t version;
    uint32_t io_count;
    fpioa_function_e io_functions[FPIOA_NUM_IO];
} fpioa_cfg_t;

extern const fpioa_cfg_t g_fpioa_cfg;
int fpioa_get_io_by_func(fpioa_function_e function);


#ifdef __cplusplus
}
#endif

#endif

