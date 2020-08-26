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
#include "spi_master.h"
#include "fpioa.h"
#include "gpiohs.h"

#define WAIT_TIMEOUT            0xFFFFFF

#define SPI_MASTER_INT_PIN      28
#define SPI_MASTER_INT_IO       6
#define SPI_MASTER_CS_PIN       25
#define SPI_MASTER_CS_IO        7
#define SPI_MASTER_CLK_PIN      26
#define SPI_MASTER_MOSI_PIN     27
#define SPI_MASTER_MISO_PIN     27

#define SPI_MASTER_CS_LOW()    gpiohs_set_pin(SPI_MASTER_CS_IO, GPIO_PV_LOW)
#define SPI_MASTER_CS_HIGH()   gpiohs_set_pin(SPI_MASTER_CS_IO, GPIO_PV_HIGH)

static volatile uint8_t spi_slave_ready;

static void spi_slave_ready_irq(void);

int spi_master_init(void)
{
    fpioa_set_function(SPI_MASTER_CLK_PIN, FUNC_SPI0_SCLK);
    fpioa_set_function(SPI_MASTER_MOSI_PIN, FUNC_SPI0_D0);
    fpioa_set_function(SPI_MASTER_CS_PIN, FUNC_GPIOHS0 + SPI_MASTER_CS_IO);
    fpioa_set_function(SPI_MASTER_INT_PIN, FUNC_GPIOHS0 + SPI_MASTER_INT_IO);

    spi_init(0, SPI_WORK_MODE_0, SPI_FF_STANDARD, 8, 0);
    spi_set_clk_rate(0, 1000000);

    SPI_MASTER_CS_HIGH();
    gpiohs_set_drive_mode(SPI_MASTER_CS_IO, GPIO_DM_OUTPUT);

    gpiohs_set_drive_mode(SPI_MASTER_INT_IO, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_pin_edge(SPI_MASTER_INT_IO, GPIO_PE_FALLING);
    gpiohs_set_irq(SPI_MASTER_INT_IO, 5, spi_slave_ready_irq);

    return 0;
}

static void spi_slave_ready_irq(void)
{
    spi_slave_ready = 1;
}

static int spi_receive_data(uint8_t *data, uint32_t len)
{
    fpioa_set_function(SPI_MASTER_MISO_PIN, FUNC_SPI0_D1);
    SPI_MASTER_CS_LOW();

    spi_receive_data_standard(0, 0,  NULL, 0, (uint8_t *)data, len);

    SPI_MASTER_CS_HIGH();
    fpioa_set_function(SPI_MASTER_MOSI_PIN, FUNC_SPI0_D0);
    return 0;
}

static int spi_send_data(uint8_t *data, uint32_t len)
{
    SPI_MASTER_CS_LOW();

    spi_send_data_standard(0, 0, NULL, 0, (const uint8_t *)data, len);

    SPI_MASTER_CS_HIGH();
    return 0;
}

static int spi_master_send_cmd(spi_slave_command_t *cmd)
{
    uint8_t data[8];

    for (uint32_t i = 0; i < WAIT_TIMEOUT; i++)
    {
        if (gpiohs_get_pin(SPI_MASTER_INT_IO) == 1)
            break;
    }
    if (gpiohs_get_pin(SPI_MASTER_INT_IO) == 0)
        return -1;

    data[0] = cmd->cmd;
    data[1] = cmd->addr;
    data[2] = cmd->addr >> 8;
    data[3] = cmd->addr >> 16;
    data[4] = cmd->addr >> 24;
    data[5] = cmd->len;
    data[6] = cmd->len >> 8;
    data[7] = 0;
    for (uint32_t i = 0; i < 7; i++)
        data[7] += data[i];
    spi_slave_ready = 0;
    spi_send_data(data, 8);
    for (uint32_t i = 0; i < WAIT_TIMEOUT; i++)
    {
        if (spi_slave_ready != 0)
            break;
    }
    if (spi_slave_ready)
        return 0;
    else
        return -2;
}

int spi_master_transfer(uint8_t *data, uint32_t addr, uint32_t len, uint8_t mode)
{
    spi_slave_command_t cmd;

    if (mode <= READ_DATA_BYTE)
    {
        if (len > 8)
            len = 8;
        cmd.len = len;
    }
    else if (mode <= READ_DATA_BLOCK)
    {
        if (len > 0x100000)
            len = 0x100000;
        addr &= 0xFFFFFFF0;
        cmd.len = len >> 4;
    }
    else
        return -1;
    cmd.cmd = mode;
    cmd.addr = addr;
    if (spi_master_send_cmd(&cmd) != 0)
        return -2;
    if (mode & 0x01)
        spi_receive_data(data, len);
    else
        spi_send_data(data, len);
    return 0;
}
