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
#include "fpioa.h"
#include "sysctl.h"
#include "uarths.h"
#include "w25qxx.h"
#include <stdio.h>

#define TEST_NUMBER (40 * 1024 + 5)
#define DATA_ADDRESS 0xB00000
uint8_t data_buf[TEST_NUMBER];

int main(void) {
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    uarths_init();
    uint8_t manuf_id, device_id;
    uint32_t index, spi_index;
    spi_index = 3;
    printf("spi%d master test\n", spi_index);

    w25qxx_init(spi_index, 0, 60000000);

    w25qxx_read_id(&manuf_id, &device_id);
    printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
    if ((manuf_id != 0xEF && manuf_id != 0xC8) ||
        (device_id != 0x17 && device_id != 0x16)) {
        printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
        return 0;
    }

    printf("write data\n");

    for (index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = (uint8_t)(index);

    /*write data*/
    uint64_t start = sysctl_get_time_us();
    w25qxx_write_data(DATA_ADDRESS, data_buf, TEST_NUMBER);
    uint64_t stop = sysctl_get_time_us();
    printf("%ld us\n", (stop - start));

    for (index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;

    start = sysctl_get_time_us();
    w25qxx_read_data(DATA_ADDRESS, data_buf, TEST_NUMBER);
    stop = sysctl_get_time_us();
    printf("read %ld us\n", (stop - start));
    for (index = 0; index < TEST_NUMBER; index++) {
        if (data_buf[index] != (uint8_t)(index)) {
            printf("quad fast read test error\n");
            return 0;
        }
    }
    printf("spi%d master test ok\n", spi_index);
    while (1)
        ;
    return 0;
}
