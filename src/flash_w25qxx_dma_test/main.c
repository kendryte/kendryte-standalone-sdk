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
#include "fpioa.h"
#include "sysctl.h"
#include "w25qxx.h"

#define TEST_NUMBER (256 + 128)
#define DATA_ADDRESS 0x110000
uint8_t data_buf[TEST_NUMBER];

void io_mux_init(uint8_t index)
{

    if (index == 0)
    {
        fpioa_set_function(30, FUNC_SPI0_SS0);
        fpioa_set_function(32, FUNC_SPI0_SCLK);
        fpioa_set_function(34, FUNC_SPI0_D0);
        fpioa_set_function(13, FUNC_SPI0_D1);
        fpioa_set_function(15, FUNC_SPI0_D2);
        fpioa_set_function(17, FUNC_SPI0_D3);
    }
    else if(index == 2)
    {
        fpioa_set_function(30, FUNC_SPI1_SS0);
        fpioa_set_function(32, FUNC_SPI1_SCLK);
        fpioa_set_function(34, FUNC_SPI1_D0);
        fpioa_set_function(13, FUNC_SPI1_D1);
        fpioa_set_function(15, FUNC_SPI1_D2);
        fpioa_set_function(17, FUNC_SPI1_D3);
    }
}

int main(void)
{
    uint8_t manuf_id, device_id;
    uint32_t index, spi_index;
    spi_index = 3;
    printf("spi%d master test\n", spi_index);
    io_mux_init(spi_index);

    w25qxx_init_dma(spi_index, 0);

    w25qxx_enable_quad_mode_dma();

    w25qxx_read_id_dma(&manuf_id, &device_id);
    printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
    if ((manuf_id != 0xEF && manuf_id != 0xC8) || (device_id != 0x17 && device_id != 0x16))
    {
        printf("manuf_id:0x%02x, device_id:0x%02x\n", manuf_id, device_id);
        return 0;
    }

    /*write data*/
    for (index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = (uint8_t)(index);
    printf("erase sector\n");
    w25qxx_sector_erase_dma(DATA_ADDRESS);
    while (w25qxx_is_busy_dma() == W25QXX_BUSY)
        ;
    printf("write data\n");
    w25qxx_write_data_direct_dma(DATA_ADDRESS, data_buf, TEST_NUMBER);

    /* standard read test*/
    for (index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    printf("standard read test start\n");
    w25qxx_read_data_dma(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_STANDARD);
    for (index = 0; index < TEST_NUMBER; index++)
    {
        if (data_buf[index] != (uint8_t)(index))
        {
            printf("standard read test error\n");
            return 0;
        }
    }

    /*standard fast read test*/
    for (index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    printf("standard fast read test start\n");
    /*w25qxx_read_data(0, data_buf, TEST_NUMBER, W25QXX_STANDARD_FAST);*/
    w25qxx_read_data_dma(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_STANDARD_FAST);
    for (index = 0; index < TEST_NUMBER; index++)
    {
        if (data_buf[index] != (uint8_t)index)
        {
            printf("standard fast read test error\n");
            return 0;
        }
    }

    /*dual read test*/
    for (index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    printf("dual read test start\n");
    w25qxx_read_data_dma(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_DUAL);
    for (index = 0; index < TEST_NUMBER; index++)
    {
        if (data_buf[index] != (uint8_t)index)
        {
            printf("dual read test error\n");
            return 0;
        }
    }

    /*quad read test*/
    for (index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    printf("quad read test start\n");
    w25qxx_read_data_dma(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_QUAD);
    for (index = 0; index < TEST_NUMBER; index++)
    {
        if (data_buf[index] != (uint8_t)(index))
        {
            printf("quad read test error\n");
            return 0;
        }
    }

    /*dual fast read test*/
    for (index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    printf("dual fast read test start\n");
    w25qxx_read_data_dma(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_DUAL_FAST);
    for (index = 0; index < TEST_NUMBER; index++)
    {
        if (data_buf[index] != (uint8_t)index)
        {
            printf("dual fast read test error\n");
            return 0;
        }
    }

    /*quad fast read test*/
    for (index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    printf("quad fast read test start\n");
    w25qxx_read_data_dma(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_QUAD_FAST);
    for (index = 0; index < TEST_NUMBER; index++)
    {
        if (data_buf[index] != (uint8_t)(index))
        {
            printf("quad fast read test error\n");
            return 0;
        }
    }

    printf("spi%d master test ok\n", spi_index);
    while (1)
        ;
    return 0;
}
