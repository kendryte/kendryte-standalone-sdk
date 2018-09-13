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
#include "i2c.h"
#include "common.h"
#include "fpioa.h"
#include "platform.h"
#include "stdlib.h"
#include "string.h"
#include "sysctl.h"

volatile struct i2c_t* const i2c[3] =
{
    (volatile struct i2c_t*)I2C0_BASE_ADDR,
    (volatile struct i2c_t*)I2C1_BASE_ADDR,
    (volatile struct i2c_t*)I2C2_BASE_ADDR
};

void i2c_pin_init(uint8_t sel, int clk_pin, int data_pin)
{
    configASSERT(sel < I2C_MAX_NUM);
    fpioa_set_function(clk_pin, FUNC_I2C0_SCLK + (2 * sel));
    fpioa_set_function(data_pin, FUNC_I2C0_SDA + (2 * sel));
}

void i2c_clk_init(uint8_t sel)
{
    configASSERT(sel < I2C_MAX_NUM);
    sysctl_clock_enable(SYSCTL_CLOCK_I2C0 + sel);
    sysctl_clock_set_threshold(SYSCTL_THRESHOLD_I2C0 + sel, 3);
}

void i2c_init(uint8_t sel, int clk_pin, int data_pin)
{
    configASSERT(sel < I2C_MAX_NUM);
    i2c_pin_init(sel, clk_pin, data_pin);
    i2c_clk_init(sel);
    dmac_init();
}

void i2c_config(uint8_t sel, size_t slaveAddress, size_t address_width,i2c_bus_speed_mode bus_speed_mode)
{
    configASSERT(sel < I2C_MAX_NUM);
    configASSERT(address_width == 7 || address_width == 10);
    int speed_mode = 1;
    switch (bus_speed_mode)
    {
        case I2C_BS_STANDARD:
            speed_mode = 1;
            break;
        default:
            break;
    }
    /*set config*/
    volatile struct i2c_t* i2c_adapter = i2c[sel];
    i2c_adapter->enable = 0;
    i2c_adapter->con = I2C_CON_MASTER_MODE | I2C_CON_SLAVE_DISABLE | I2C_CON_RESTART_EN |
                       (address_width == 10 ? I2C_CON_10BITADDR_SLAVE : 0) | I2C_CON_SPEED(speed_mode);
    i2c_adapter->ss_scl_hcnt = I2C_SS_SCL_HCNT_COUNT(37);
    i2c_adapter->ss_scl_lcnt = I2C_SS_SCL_LCNT_COUNT(40);
    i2c_adapter->tar = I2C_TAR_ADDRESS(slaveAddress);
    i2c_adapter->intr_mask = 0;
    i2c_adapter->dma_cr = 0x3;
    i2c_adapter->dma_rdlr = 0;
    i2c_adapter->dma_tdlr = 4;
    i2c_adapter->enable = I2C_ENABLE_ENABLE;
}

int i2c_write_reg(uint8_t sel, uint8_t reg, uint8_t* data_buf, uint8_t length)
{
    configASSERT(sel < I2C_MAX_NUM);
    volatile struct i2c_t* i2c_adapter = i2c[sel];
    uint8_t fifo_len, index;

    fifo_len = length < 7 ? length : 7;
    i2c_adapter->data_cmd = I2C_DATA_CMD_DATA(reg);
    for (index = 0; index < fifo_len; index++)
        i2c_adapter->data_cmd = I2C_DATA_CMD_DATA(*data_buf++);
    length -= fifo_len;
    while (length)
    {
        fifo_len = 8 - i2c_adapter->txflr;
        fifo_len = length < fifo_len ? length : fifo_len;
        for (index = 0; index < fifo_len; index++)
            i2c_adapter->data_cmd = I2C_DATA_CMD_DATA(*data_buf++);
        if (i2c_adapter->tx_abrt_source != 0)
            return 1;
        length -= fifo_len;
    }
    while (i2c_adapter->status & I2C_STATUS_ACTIVITY)
        ;
    return 0;
}

int i2c_write_reg_dma(dmac_channel_number channel_num, uint8_t sel, uint8_t reg, uint8_t* data_buf, uint8_t length)
{
    configASSERT(sel < I2C_MAX_NUM);
    volatile struct i2c_t* i2c_adapter = i2c[sel];

    uint32_t* buf = malloc((length + 1) * sizeof(uint32_t));
    buf[0] = reg;
    int i;
    for (i = 0; i < length + 1; i++)
    {
        buf[i + 1] = data_buf[i];
    }

    sysctl_dma_select(channel_num, SYSCTL_DMA_SELECT_I2C0_TX_REQ + sel * 2);
    dmac_set_single_mode(channel_num, buf, (void*)(&i2c_adapter->data_cmd), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
        DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, length + 1);

    dmac_wait_done(channel_num);
    free((void*)buf);

    while (i2c_adapter->status & I2C_STATUS_ACTIVITY)
    {
        if (i2c_adapter->tx_abrt_source != 0)
            configASSERT(!"source abort");
    }
    return 0;
}

int i2c_read_reg(uint8_t sel, uint8_t reg, uint8_t* data_buf, uint8_t length)
{
    uint8_t fifo_len, index;
    uint8_t rx_len = length;
    configASSERT(sel < I2C_MAX_NUM);
    volatile struct i2c_t* i2c_adapter = i2c[sel];

    fifo_len = length < 7 ? length : 7;
    i2c_adapter->data_cmd = I2C_DATA_CMD_DATA(reg);
    for (index = 0; index < fifo_len; index++)
        i2c_adapter->data_cmd = I2C_DATA_CMD_CMD;
    length -= fifo_len;
    while (length || rx_len)
    {
        fifo_len = i2c_adapter->rxflr;
        fifo_len = rx_len < fifo_len ? rx_len : fifo_len;
        for (index = 0; index < fifo_len; index++)
            *data_buf++ = i2c_adapter->data_cmd;
        rx_len -= fifo_len;
        fifo_len = 8 - i2c_adapter->txflr;
        fifo_len = length < fifo_len ? length : fifo_len;
        for (index = 0; index < fifo_len; index++)
            i2c_adapter->data_cmd = I2C_DATA_CMD_CMD;
        if (i2c_adapter->tx_abrt_source != 0)
            return 1;
        length -= fifo_len;
    }
    return 0;
}

int i2c_read_reg_dma(dmac_channel_number w_channel_num, dmac_channel_number r_channel_num,
    uint8_t sel, uint8_t reg, uint8_t* data_buf, uint8_t length)
{
    configASSERT(sel < I2C_MAX_NUM);
    volatile struct i2c_t* i2c_adapter = i2c[sel];

    uint32_t* write_cmd = malloc(sizeof(uint32_t) * (1 + length));
    size_t i;
    write_cmd[0] = reg;
    for (i = 0; i < length; i++)
        write_cmd[i + 1] = I2C_DATA_CMD_CMD;

    sysctl_dma_select(w_channel_num, SYSCTL_DMA_SELECT_I2C0_TX_REQ + sel * 2);
    sysctl_dma_select(r_channel_num, SYSCTL_DMA_SELECT_I2C0_RX_REQ + sel * 2);

    dmac_set_single_mode(r_channel_num, (void*)(&i2c_adapter->data_cmd), write_cmd, DMAC_ADDR_NOCHANGE,
         DMAC_ADDR_INCREMENT,DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, length);

    dmac_set_single_mode(w_channel_num, write_cmd, (void*)(&i2c_adapter->data_cmd), DMAC_ADDR_INCREMENT,
         DMAC_ADDR_NOCHANGE,DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, length + 1);

    dmac_wait_done(w_channel_num);
    dmac_wait_done(r_channel_num);

    for (i = 0; i < length; i++)
    {
        data_buf[i] = write_cmd[i];
    }

    free(write_cmd);
    return 0;
}
