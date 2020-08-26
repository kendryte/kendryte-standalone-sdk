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
#include "sd3068.h"
#include "fpioa.h"
#include "utils.h"
#include "sysctl.h"
#include <stdlib.h>
#include <string.h>

uint32_t g_sd3068_i2c_number = 0;
uint32_t g_sd3068_use_dma = 0;

void sd3068_init(i2c_device_number_t i2c_num, uint32_t slave_address, uint32_t address_width, uint32_t i2c_clk, uint32_t dma)
{
    g_sd3068_i2c_number = i2c_num;
    g_sd3068_use_dma = dma;
    i2c_init(i2c_num, slave_address, address_width, i2c_clk);
}

static int sd3068_write_reg(uint8_t reg, uint8_t *data_buf, uint8_t length)
{
    int ret = 0;
    uint8_t *buf = malloc(length + 1);
    buf[0] = reg;
    memcpy(buf + 1, data_buf, length);
    if(g_sd3068_use_dma)
        i2c_send_data_dma(DMAC_CHANNEL0, g_sd3068_i2c_number, buf, length + 1);
    else
        ret = i2c_send_data(g_sd3068_i2c_number, buf, length + 1);
    free(buf);
    return ret;
}

static int sd3068_read_reg(uint8_t reg, uint8_t *data_buf, uint8_t length)
{
    int ret = 0;
    if(g_sd3068_use_dma)
        i2c_recv_data_dma(DMAC_CHANNEL0, DMAC_CHANNEL1, g_sd3068_i2c_number, &reg, 1, data_buf, length);
    else
        ret = i2c_recv_data(g_sd3068_i2c_number, &reg, 1, data_buf, length);
    return ret;
}

static uint8_t hex2bcd(uint8_t data)
{
    return data / 10 * 16 + data % 10;
}

static uint8_t bcd2hex(uint8_t data)
{
    return data / 16 * 10 + data % 16;
}

int sd3068_write_enable(void)
{
    int ret = 0;
    uint8_t data[2] = {0};

    data[0] = 0xFF;
    data[1] = 0x80;
    ret |= sd3068_write_reg(0x10, &data[1], 1);
    ret |= sd3068_write_reg(0x0F, &data[0], 1);

    return ret;
}

int sd3068_write_disable(void)
{
    uint8_t data[2] = {0};

    data[0] = 0x7B;
    data[1] = 0;
    return sd3068_write_reg(0x0F, data, 2);
}

int sd3068_write_data(uint8_t addr, uint8_t *data_buf, uint8_t length)
{
    addr = ((addr <= 69) ? addr : 69);
    length = ((length <= 70 - addr) ? length : 70 - addr);
    return sd3068_write_reg(0x2C + addr, data_buf, length);

}

int sd3068_read_data(uint8_t addr, uint8_t *data_buf, uint8_t length)
{
    addr = ((addr <= 69) ? addr : 69);
    length = ((length <= 70 - addr) ? length : 70 - addr);
    return sd3068_read_reg(0x2C + addr, data_buf, length);
}

int sd3068_set_time(sd_time_t time)
{
    uint8_t data[7] = {0};

    data[0] = hex2bcd(time.sec);
    data[1] = hex2bcd(time.min);
    data[2] = hex2bcd(time.hour) | 0x80;
    data[3] = hex2bcd(5);
    data[4] = hex2bcd(time.day);
    data[5] = hex2bcd(time.month);
    data[6] = hex2bcd(time.year);
    return sd3068_write_reg(0x00, data, 7);
}

int sd3068_get_time(sd_time_t *time)
{
    int ret;
    uint8_t data[7] = {0};

    ret = sd3068_read_reg(0x00, data, 7);
    if(ret == 0)
    {
        time->sec = bcd2hex(data[0]);
        time->min = bcd2hex(data[1]);
        time->hour = bcd2hex(data[2] & 0x7F);
        time->day = bcd2hex(data[4]);
        time->month = bcd2hex(data[5]);
        time->year = bcd2hex(data[6]);
    }

    return 0;
}

