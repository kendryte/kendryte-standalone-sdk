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
#include <stddef.h>
#include "encoding.h"
#include "utils.h"

void set_bit(volatile uint32_t *bits, uint32_t mask, uint32_t value)
{
    uint32_t org = (*bits) & ~mask;
    *bits = org | (value & mask);
}

void set_bit_offset(volatile uint32_t *bits, uint32_t mask, size_t offset, uint32_t value)
{
    set_bit(bits, mask << offset, value << offset);
}

void set_gpio_bit(volatile uint32_t *bits, size_t offset, uint32_t value)
{
    set_bit_offset(bits, 1, offset, value);
}

uint32_t get_bit(volatile uint32_t *bits, uint32_t mask, size_t offset)
{
    return ((*bits) & (mask << offset)) >> offset;
}

uint32_t get_gpio_bit(volatile uint32_t *bits, size_t offset)
{
    return get_bit(bits, 1, offset);
}

uint32_t is_memory_cache(uintptr_t address)
{
    #define MEM_CACHE_LEN (6 * 1024 * 1024)

    return ((address >= 0x80000000) && (address < 0x80000000 + MEM_CACHE_LEN));
}

uintptr_t io_to_cache(uintptr_t address)
{
#define  mem_len (6 * 1024 * 1024)

    if((address >= 0x40000000) && (address < 0x40000000 + mem_len))
        return (uintptr_t)address ^ 0xC0000000;
    else
        return address;
}

uintptr_t cache_to_io(uintptr_t address)
{
#define  mem_len (6 * 1024 * 1024)

    if((address >= 0x80000000) && (address < 0x80000000 + mem_len))
        return (uintptr_t)address ^ 0xC0000000;
    else
        return address;
}

