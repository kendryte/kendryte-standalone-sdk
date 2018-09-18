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
#include "platform.h"
#include "spi.h"
#include "fpioa.h"
#include "common.h"
#include "sysctl.h"
#include <stddef.h>
#include <stdlib.h>

#define     SPI_MAX_NUM     4

volatile spi_t *const spi[4] =
{
    (volatile spi_t *)SPI0_BASE_ADDR,
    (volatile spi_t *)SPI1_BASE_ADDR,
    (volatile spi_t *)SPI_SLAVE_BASE_ADDR,
    (volatile spi_t *)SPI3_BASE_ADDR
};

int spi_clk_init(uint8_t spi_bus){
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    sysctl_clock_enable(SYSCTL_CLOCK_SPI0 + spi_bus);
    sysctl_clock_set_threshold(SYSCTL_THRESHOLD_SPI0 + spi_bus, 0);
    return 0;
}

int spi_init(uint8_t spi_bus){
    spi_clk_init(spi_bus);
    dmac_init();
    return 0;
}

int spi_master_config(uint8_t spi_bus, spi_mode mode, spi_frame_format frame_format, size_t data_bit_length){
    configASSERT(data_bit_length >= 4 && data_bit_length <= 32);
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);

    uint8_t dfs_offset, frf_offset;
    switch(spi_bus){
        case 0:
        case 1:
            dfs_offset = 16;
            frf_offset = 21;
            break;
        case 2:
            configASSERT(!"Spi Bus 2 Not Support!");
            break;
        case 3:
        default:
            dfs_offset = 0;
            frf_offset = 22;
            break;
    }

    switch (frame_format)
    {
    case SPI_FF_DUAL:
        configASSERT(data_bit_length % 2 == 0);
        break;
    case SPI_FF_QUAD:
        configASSERT(data_bit_length % 4 == 0);
        break;
    case SPI_FF_OCTAL:
        configASSERT(data_bit_length % 8 == 0);
        break;
    default:
        break;
    }
    volatile spi_t *spi_adapter = spi[spi_bus];

    spi_adapter->baudr = 0x14;
    spi_adapter->imr = 0x00;
    spi_adapter->dmacr = 0x00;
    spi_adapter->dmatdlr = 0x10;
    spi_adapter->dmardlr = 0x00;
    spi_adapter->ser = 0x00;
    spi_adapter->ssienr = 0x00;
    spi_adapter->ctrlr0 = (mode << 6) | (frame_format << frf_offset) | ((data_bit_length - 1) << dfs_offset);
    spi_adapter->spi_ctrlr0 = 0;
    return 0;
}

void spi_trans_config(uint8_t spi_bus, size_t instruction_length, size_t address_length,
                                size_t wait_cycles, spi_addr_inst_trans_mode trans_mode)
{
    configASSERT(wait_cycles < (1 << 5));
    configASSERT(trans_mode < 3);
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];
    uint32_t inst_l;
    switch (instruction_length)
    {
    case 0:
        inst_l = 0;
        break;
    case 4:
        inst_l = 1;
        break;
    case 8:
        inst_l = 2;
        break;
    case 16:
        inst_l = 3;
        break;
    default:
        configASSERT(!"Invalid instruction length");
        break;
    }

    configASSERT(address_length % 4 == 0 && address_length <= 60);
    uint32_t addr_l = address_length / 4;

    spi_handle->spi_ctrlr0 = (wait_cycles << 11) | (inst_l << 8) | (addr_l << 2) | trans_mode;
}

int spi_send_data(uint8_t spi_bus, uint32_t chip_sel, uint8_t *cmd_buff, uint8_t cmd_len, uint8_t *tx_buff, uint32_t tx_len)
{
    uint32_t index, fifo_len;
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);

    volatile spi_t *spi_handle = spi[spi_bus];
    spi_handle->ssienr = 0x01;
    while (cmd_len){
        spi_handle->dr[0] = *cmd_buff++;
        cmd_len--;
    }
    fifo_len = 32 - spi_handle->txflr;
    fifo_len = fifo_len < tx_len ? fifo_len : tx_len;
    for (index = 0; index < fifo_len; index++)
        spi_handle->dr[0] = *tx_buff++;
    tx_len -= fifo_len;
    spi_handle->ser = chip_sel;
    while (tx_len) {
        fifo_len = 32 - spi_handle->txflr;
        fifo_len = fifo_len < tx_len ? fifo_len : tx_len;
        for (index = 0; index < fifo_len; index++)
            spi_handle->dr[0] = *tx_buff++;
        tx_len -= fifo_len;
    }
    while ((spi_handle->sr & 0x05) != 0x04)
        ;
    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;
    return 0;
}

int spi_send_data_dma(dmac_channel_number channel_num, uint8_t spi_bus, uint32_t chip_sel,
                        uint8_t *cmd_buff, uint8_t cmd_len, uint8_t *tx_buff, uint32_t tx_len)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];

    uint32_t *buf = malloc((cmd_len + tx_len) * sizeof(uint32_t));
    int i;
    for(i = 0; i < cmd_len; i++){
        buf[i] = cmd_buff[i];
    }

    for(i = 0; i < tx_len; i++){
        buf[cmd_len + i] = tx_buff[i];
    }
    spi_handle->dmacr = 0x2;    /*enable dma transmit*/
    spi_handle->ssienr = 0x01;

    sysctl_dma_select(channel_num, SYSCTL_DMA_SELECT_SSI0_TX_REQ + spi_bus * 2);
    dmac_set_single_mode(channel_num, buf, (void *)(&spi_handle->dr[0]), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
                                DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, cmd_len + tx_len);
    spi_handle->ser = chip_sel;
    dmac_wait_done(channel_num);
    free((void*)buf);


    while ((spi_handle->sr & 0x05) != 0x04)
        ;
    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;
    return 0;
}

int spi_normal_send_dma(dmac_channel_number channel_num, uint8_t spi_bus, uint32_t chip_sel,
                        void *tx_buff, uint32_t tx_len, spi_transfer_width stw)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];

    uint32_t *buf = malloc((tx_len) * sizeof(uint32_t));
    int i;
    for(i = 0; i < tx_len; i++){
        switch(stw){
            case SPI_TRANS_SHORT:
                buf[i] = ((uint16_t *)tx_buff)[i];
            break;
            case SPI_TRANS_INT:
                buf[i] = ((uint32_t *)tx_buff)[i];
            break;
            break;
            case SPI_TRANS_CHAR:
            default:
                buf[i] = ((uint8_t *)tx_buff)[i];
            break;
        }
    }
    spi_handle->dmacr = 0x2;    /*enable dma transmit*/
    spi_handle->ssienr = 0x01;

    sysctl_dma_select(channel_num, SYSCTL_DMA_SELECT_SSI0_TX_REQ + spi_bus * 2);
    dmac_set_single_mode(channel_num, buf, (void *)(&spi_handle->dr[0]), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
                                DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, tx_len);
    spi_handle->ser = chip_sel;
    dmac_wait_done(channel_num);
    free((void*)buf);

    while ((spi_handle->sr & 0x05) != 0x04)
        ;
    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;
    return 0;
}

int spi_receive_data(uint8_t spi_bus, uint32_t chip_sel, uint8_t *cmd_buff, uint8_t cmd_len, uint8_t *rx_buff, uint32_t rx_len)
{
    uint32_t index, fifo_len;
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];

    spi_handle->ctrlr1 = rx_len - 1;
    spi_handle->ssienr = 0x01;
    while (cmd_len--)
        spi_handle->dr[0] = *cmd_buff++;
    spi_handle->ser = chip_sel;
    while (rx_len) {
        fifo_len = spi_handle->rxflr;
        fifo_len = fifo_len < rx_len ? fifo_len : rx_len;
        for (index = 0; index < fifo_len; index++)
            *rx_buff++ = spi_handle->dr[0];
        rx_len -= fifo_len;
    }
    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;
    return 0;
}

int spi_receive_data_dma(dmac_channel_number w_channel_num, dmac_channel_number r_channel_num,
                            uint8_t spi_bus, uint32_t chip_sel, uint8_t *cmd_buff, uint8_t cmd_len, uint8_t *rx_buff, uint32_t rx_len)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];

    uint32_t * write_cmd = malloc(sizeof(uint32_t) * (cmd_len + rx_len));
    size_t i;
    for (i = 0; i < cmd_len; i++)
        write_cmd[i] = cmd_buff[i];

    spi_handle->ctrlr1 = rx_len - 1;
    spi_handle->dmacr = 0x3;
    spi_handle->ssienr = 0x01;
    spi_handle->ser = chip_sel;

    sysctl_dma_select(w_channel_num, SYSCTL_DMA_SELECT_SSI0_TX_REQ + spi_bus * 2);
    sysctl_dma_select(r_channel_num, SYSCTL_DMA_SELECT_SSI0_RX_REQ + spi_bus * 2);


    dmac_set_single_mode(r_channel_num, (void *)(&spi_handle->dr[0]), write_cmd, DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT,
                            DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, rx_len);

    dmac_set_single_mode(w_channel_num, write_cmd, (void *)(&spi_handle->dr[0]), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
                            DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, cmd_len);

    dmac_wait_done(w_channel_num);
    dmac_wait_done(r_channel_num);

    for(i = 0; i < rx_len; i++){
        rx_buff[i] = write_cmd[i];
    }
    free(write_cmd);

    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;
    return 0;
}


int spi_special_receive_data(uint8_t spi_bus, uint32_t chip_sel, uint32_t *cmd_buff, uint8_t cmd_len, uint8_t *rx_buff, uint32_t rx_len)
{
    uint32_t index, fifo_len;
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];

    spi_handle->ctrlr1 = rx_len - 1;
    spi_handle->ssienr = 0x01;
    while (cmd_len--)
        spi_handle->dr[0] = *cmd_buff++;
    spi_handle->ser = chip_sel;
    while (rx_len) {
        fifo_len = spi_handle->rxflr;
        fifo_len = fifo_len < rx_len ? fifo_len : rx_len;
        for (index = 0; index < fifo_len; index++)
            *rx_buff++ = spi_handle->dr[0];
        rx_len -= fifo_len;
    }
    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;
    return 0;
}

int spi_special_receive_data_dma(dmac_channel_number w_channel_num, dmac_channel_number r_channel_num,
                                    uint8_t spi_bus, uint32_t chip_sel, uint32_t *cmd_buff, uint8_t cmd_len, uint8_t *rx_buff, uint32_t rx_len)
{
        configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
        volatile spi_t *spi_handle = spi[spi_bus];

        uint32_t * write_cmd = malloc(sizeof(uint32_t) * (cmd_len + rx_len));
        size_t i;
        for (i = 0; i < cmd_len; i++)
            write_cmd[i] = cmd_buff[i];

        spi_handle->ctrlr1 = rx_len - 1;
        spi_handle->dmacr = 0x3;
        spi_handle->ssienr = 0x01;
        spi_handle->ser = chip_sel;

        sysctl_dma_select(w_channel_num, SYSCTL_DMA_SELECT_SSI0_TX_REQ + spi_bus * 2);
        sysctl_dma_select(r_channel_num, SYSCTL_DMA_SELECT_SSI0_RX_REQ + spi_bus * 2);


        dmac_set_single_mode(r_channel_num, (void *)(&spi_handle->dr[0]), write_cmd, DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT,
                                DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, rx_len);

        dmac_set_single_mode(w_channel_num, write_cmd, (void *)(&spi_handle->dr[0]), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
                                DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, cmd_len);

        dmac_wait_done(w_channel_num);
        dmac_wait_done(r_channel_num);

        for(i = 0; i < rx_len; i++){
            rx_buff[i] = write_cmd[i];
        }
        free(write_cmd);
        spi_handle->ser = 0x00;
        spi_handle->ssienr = 0x00;
        return 0;

}


int spi_special_send_data(uint8_t spi_bus, uint32_t chip_sel, uint32_t *cmd_buff, uint8_t cmd_len, uint8_t *tx_buff, uint32_t tx_len)
{
    uint32_t index, fifo_len;
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);

    volatile spi_t *spi_handle = spi[spi_bus];

    spi_handle->ssienr = 0x01;
    while (cmd_len--)
        spi_handle->dr[0] = *cmd_buff++;
    fifo_len = 32 - spi_handle->txflr;
    fifo_len = fifo_len < tx_len ? fifo_len : tx_len;
    for (index = 0; index < fifo_len; index++)
        spi_handle->dr[0] = *tx_buff++;
    tx_len -= fifo_len;
    spi_handle->ser = chip_sel;
    while (tx_len) {
        fifo_len = 32 - spi_handle->txflr;
        fifo_len = fifo_len < tx_len ? fifo_len : tx_len;
        for (index = 0; index < fifo_len; index++)
            spi_handle->dr[0] = *tx_buff++;
        tx_len -= fifo_len;
    }
    while ((spi_handle->sr & 0x05) != 0x04)
        ;
    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;
    return 0;
}

int spi_special_send_data_dma(dmac_channel_number channel_num,uint8_t spi_bus, uint32_t chip_sel,
                                uint32_t *cmd_buff, uint8_t cmd_len, uint8_t *tx_buff, uint32_t tx_len)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];

    uint32_t *buf = malloc((cmd_len + tx_len) * sizeof(uint32_t));
    int i;
    for(i = 0; i < cmd_len; i++){
        buf[i] = cmd_buff[i];
    }

    for(i = 0; i < tx_len; i++){
        buf[cmd_len + i] = tx_buff[i];
    }
    spi_handle->dmacr = 0x2;    /*enable dma transmit*/
    spi_handle->ssienr = 0x01;

    sysctl_dma_select(channel_num, SYSCTL_DMA_SELECT_SSI0_TX_REQ + spi_bus * 2);
    dmac_set_single_mode(channel_num, buf, (void *)(&spi_handle->dr[0]), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
                                DMAC_MSIZE_4, DMAC_TRANS_WIDTH_32, cmd_len + tx_len);
    spi_handle->ser = chip_sel;
    dmac_wait_done(channel_num);
    free((void*)buf);

    while ((spi_handle->sr & 0x05) != 0x04)
        ;
    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;
    return 0;
}

int spi_fill_dma(dmac_channel_number channel_num,uint8_t spi_bus, uint32_t chip_sel,
                                uint32_t *cmd_buff, uint32_t cmd_len)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];

    spi_handle->dmacr = 0x2;    /*enable dma transmit*/
    spi_handle->ssienr = 0x01;

    sysctl_dma_select(channel_num, SYSCTL_DMA_SELECT_SSI0_TX_REQ + spi_bus * 2);
    dmac_set_single_mode(channel_num, cmd_buff, (void *)(&spi_handle->dr[0]), DMAC_ADDR_NOCHANGE, DMAC_ADDR_NOCHANGE,
                                DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, cmd_len);
    spi_handle->ser = chip_sel;
    dmac_wait_done(channel_num);

    while ((spi_handle->sr & 0x05) != 0x04)
        ;
    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;
    return 0;
}

void spi_set_tmod(uint8_t spi_bus, uint32_t tmod)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];
    uint8_t tmod_offset = 0;
    switch(spi_bus){
    case 0:
    case 1:
        tmod_offset = 8;
        break;
    case 2:
        configASSERT(!"Spi Bus 2 Not Support!");
        break;
    case 3:
    default:
        tmod_offset = 10;
        break;
    }

    set_bit(&spi_handle->ctrlr0, 3 << tmod_offset, tmod << tmod_offset);
}

void spi_set_frame_format(uint8_t spi_bus, uint32_t spi_frf)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];
    uint8_t frf_offset = 0;
    switch(spi_bus){
    case 0:
    case 1:
        frf_offset = 21;
        break;
    case 2:
        configASSERT(!"Spi Bus 2 Not Support!");
        break;
    case 3:
    default:
        frf_offset = 22;
        break;
    }

    set_bit(&spi_handle->ctrlr0, 3 << frf_offset, spi_frf << frf_offset);
}

int spi_get_frame_format(uint8_t spi_bus)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];
    uint8_t frf_offset = 0;
    switch(spi_bus){
    case 0:
    case 1:
        frf_offset = 21;
        break;
    case 2:
        configASSERT(!"Spi Bus 2 Not Support!");
        break;
    case 3:
    default:
        frf_offset = 22;
        break;
    }
    return ((spi_handle->ctrlr0 >> frf_offset) & 0x03);
}

/*
    SPI MODE0-3
    Serial Clock Polarity
    Serial Clock Phase
*/
void spi_set_work_mode(uint8_t spi_bus, spi_mode mode)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];
    set_bit(&spi_handle->ctrlr0, 0x3 << 6, mode << 6);
}

void spi_set_frame_size(uint8_t spi_bus, uint32_t dfs)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    volatile spi_t *spi_handle = spi[spi_bus];

    uint8_t dfs_offset;
    switch(spi_bus){
        case 0:
        case 1:
            dfs_offset = 16;
            break;
        case 2:
            configASSERT(!"Spi Bus 2 Not Support!");
            break;
        case 3:
        default:
            dfs_offset = 0;
            break;
    }
    int frame_format = spi_get_frame_format(spi_bus);
    switch (frame_format)
    {
    case SPI_FF_DUAL:
        configASSERT(dfs % 2 == 0);
        break;
    case SPI_FF_QUAD:
        configASSERT(dfs % 4 == 0);
        break;
    case SPI_FF_OCTAL:
        configASSERT(dfs % 8 == 0);
        break;
    default:
        break;
    }
    set_bit(&spi_handle->ctrlr0, 0x1F << dfs_offset, (dfs-1) << dfs_offset);
}

void spi_set_wait_cycles(uint8_t spi_bus, uint32_t wcycles)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    configASSERT(wcycles < (1 << 5));
    int frame_format = spi_get_frame_format(spi_bus);
    configASSERT(frame_format != SPI_FF_STANDARD);
    volatile spi_t *spi_handle = spi[spi_bus];

    set_bit(&spi_handle->spi_ctrlr0, 0x1F << 11, wcycles << 11);
}

void spi_set_inst_length(uint8_t spi_bus, uint32_t instruction_length)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    int frame_format = spi_get_frame_format(spi_bus);
    configASSERT(frame_format != SPI_FF_STANDARD);
    volatile spi_t *spi_handle = spi[spi_bus];

    uint32_t inst_l = 0;
    switch (instruction_length)
    {
    case 0:
        inst_l = 0;
        break;
    case 4:
        inst_l = 1;
        break;
    case 8:
        inst_l = 2;
        break;
    case 16:
        inst_l = 3;
        break;
    default:
        configASSERT("Invalid instruction length");
        break;
    }

    set_bit(&spi_handle->spi_ctrlr0, 0x3 << 8, inst_l << 8);
}

void spi_set_address_length(uint8_t spi_bus, uint32_t address_length)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    int frame_format = spi_get_frame_format(spi_bus);
    configASSERT(frame_format != SPI_FF_STANDARD);
    configASSERT(address_length % 4 == 0 && address_length <= 60);
    volatile spi_t *spi_handle = spi[spi_bus];
    uint32_t addr_l = address_length / 4;
    set_bit(&spi_handle->spi_ctrlr0, 0xF << 2, addr_l << 2);
}

void spi_set_trans_mode(uint8_t spi_bus, spi_addr_inst_trans_mode trans_mode)
{
    configASSERT(spi_bus < SPI_MAX_NUM && spi_bus != 2);
    int frame_format = spi_get_frame_format(spi_bus);
    configASSERT(frame_format != SPI_FF_STANDARD);
    volatile spi_t *spi_handle = spi[spi_bus];
    set_bit(&spi_handle->spi_ctrlr0, 0x3 << 0, trans_mode << 0);
}

