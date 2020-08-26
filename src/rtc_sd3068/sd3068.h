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
#ifndef _SD3068_H
#define _SD3068_H

#include <stdint.h>
#include "i2c.h"

#define SD3068_ADDR         0x32
#define SD3068_ADDR_LENTH   7

typedef struct _sd_time
{
    uint32_t year:6;
    uint32_t month:4;
    uint32_t day:5;
    uint32_t hour:5;
    uint32_t min:6;
    uint32_t sec:6;
} __attribute__((packed, aligned(4))) sd_time_t;

void sd3068_init(i2c_device_number_t i2c_num, uint32_t slave_address, uint32_t address_width, uint32_t i2c_clk, uint32_t dma);
int sd3068_write_enable(void);
int sd3068_write_disable(void);
int sd3068_set_time(sd_time_t time);
int sd3068_get_time(sd_time_t *time);
int sd3068_write_data(uint8_t addr, uint8_t *data_buf, uint8_t length);
int sd3068_read_data(uint8_t addr, uint8_t *data_buf, uint8_t length);
int sd3068_set_time_dma(sd_time_t time);
int sd3068_write_enable_dma(void);
int sd3068_get_time_dma(sd_time_t *time);
int sd3068_read_data_dma(uint8_t addr, uint8_t *data_buf, uint8_t length);
int sd3068_write_data_dma(uint8_t addr, uint8_t *data_buf, uint8_t length);

#endif

