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
#include "sd3068.h"
#include "fpioa.h"
#include "bsp.h"

sd_time_t time_now;
uint8_t data_test[70];

#define     I2C_BUSNO       0

void io_mux_init(void)
{
    fpioa_set_function(18, FUNC_I2C0_SCLK);
    fpioa_set_function(19, FUNC_I2C0_SDA);
}

int main(void)
{
    io_mux_init();
    sd3068_init(I2C_BUSNO, SD3068_ADDR, SD3068_ADDR_LENTH, 200000, 1);

    printf("write enable\n");
    sd3068_write_enable();

    printf("set time\n");
    time_now.year = 17;
    time_now.month = 9;
    time_now.day = 8;
    time_now.hour = 20;
    time_now.min = 0;
    time_now.sec = 0;
    sd3068_set_time(time_now);
    printf("write data\n");
    uint8_t index;
    for (index = 0; index < 70; index++)
        data_test[index] = index;
    sd3068_write_data(0, data_test, 70);

    printf("read data\n");
    for (index = 0; index < 70; index++)
        data_test[index] = 0;
    sd3068_read_data(0, data_test, 70);
    for (index = 0; index < 70; index++)
    {
        if (data_test[index] != index)
        {
            printf("i2c master test error\n");
            return 0;
        }
    }
    printf("i2c master test ok\n");

    while (1)
    {
        sd3068_get_time(&time_now);
        printf("%4d-%02d-%02d %02d:%02d:%02d\n", time_now.year + 2000,
            time_now.month, time_now.day, time_now.hour,
            time_now.min, time_now.sec);
        sleep(1);
    }
    return 0;
}
