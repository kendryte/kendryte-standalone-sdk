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

#ifndef _BSP_ENTRY_H
#define _BSP_ENTRY_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void init_bss(void)
{
    extern unsigned int __bss_start;
    extern unsigned int __bss_end;
    unsigned int *dst;

    dst = &__bss_start;
    while (dst < &__bss_end)
        *dst++ = 0;
}

static inline void init_tls(void)
{
    register void *thread_pointer asm("tp");
    extern char _tls_data;

    extern __thread char _tdata_begin, _tdata_end, _tbss_end;

    size_t tdata_size = &_tdata_end - &_tdata_begin;

    memcpy(thread_pointer, &_tls_data, tdata_size);

    size_t tbss_size = &_tbss_end - &_tdata_end;

    memset(thread_pointer + tdata_size, 0, tbss_size);
}

#ifdef __cplusplus
}
#endif

#endif /* _BSP_ENTRY_H */

