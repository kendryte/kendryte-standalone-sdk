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
#ifndef _DRIVER_SPI_H
#define _DRIVER_SPI_H

#include <stdint.h>
#include <stddef.h>
#include "dmac.h"

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
typedef struct _spi
{
    /* SPI Control Register 0                                    (0x00)*/
    volatile uint32_t ctrlr0;
    /* SPI Control Register 1                                    (0x04)*/
    volatile uint32_t ctrlr1;
    /* SPI Enable Register                                       (0x08)*/
    volatile uint32_t ssienr;
    /* SPI Microwire Control Register                            (0x0c)*/
    volatile uint32_t mwcr;
    /* SPI Slave Enable Register                                 (0x10)*/
    volatile uint32_t ser;
    /* SPI Baud Rate Select                                      (0x14)*/
    volatile uint32_t baudr;
    /* SPI Transmit FIFO Threshold Level                         (0x18)*/
    volatile uint32_t txftlr;
    /* SPI Receive FIFO Threshold Level                          (0x1c)*/
    volatile uint32_t rxftlr;
    /* SPI Transmit FIFO Level Register                          (0x20)*/
    volatile uint32_t txflr;
    /* SPI Receive FIFO Level Register                           (0x24)*/
    volatile uint32_t rxflr;
    /* SPI Status Register                                       (0x28)*/
    volatile uint32_t sr;
    /* SPI Interrupt Mask Register                               (0x2c)*/
    volatile uint32_t imr;
    /* SPI Interrupt Status Register                             (0x30)*/
    volatile uint32_t isr;
    /* SPI Raw Interrupt Status Register                         (0x34)*/
    volatile uint32_t risr;
    /* SPI Transmit FIFO Overflow Interrupt Clear Register       (0x38)*/
    volatile uint32_t txoicr;
    /* SPI Receive FIFO Overflow Interrupt Clear Register        (0x3c)*/
    volatile uint32_t rxoicr;
    /* SPI Receive FIFO Underflow Interrupt Clear Register       (0x40)*/
    volatile uint32_t rxuicr;
    /* SPI Multi-Master Interrupt Clear Register                 (0x44)*/
    volatile uint32_t msticr;
    /* SPI Interrupt Clear Register                              (0x48)*/
    volatile uint32_t icr;
    /* SPI DMA Control Register                                  (0x4c)*/
    volatile uint32_t dmacr;
    /* SPI DMA Transmit Data Level                               (0x50)*/
    volatile uint32_t dmatdlr;
    /* SPI DMA Receive Data Level                                (0x54)*/
    volatile uint32_t dmardlr;
    /* SPI Identification Register                               (0x58)*/
    volatile uint32_t idr;
    /* SPI DWC_ssi component version                             (0x5c)*/
    volatile uint32_t ssic_version_id;
    /* SPI Data Register 0-36                                    (0x60 -- 0xec)*/
    volatile uint32_t dr[36];
    /* SPI RX Sample Delay Register                              (0xf0)*/
    volatile uint32_t rx_sample_delay;
    /* SPI SPI Control Register                                  (0xf4)*/
    volatile uint32_t spi_ctrlr0;
    /* reserved                                                  (0xf8)*/
    volatile uint32_t resv;
    /* SPI XIP Mode bits                                         (0xfc)*/
    volatile uint32_t xip_mode_bits;
    /* SPI XIP INCR transfer opcode                              (0x100)*/
    volatile uint32_t xip_incr_inst;
    /* SPI XIP WRAP transfer opcode                              (0x104)*/
    volatile uint32_t xip_wrap_inst;
    /* SPI XIP Control Register                                  (0x108)*/
    volatile uint32_t xip_ctrl;
    /* SPI XIP Slave Enable Register                             (0x10c)*/
    volatile uint32_t xip_ser;
    /* SPI XIP Receive FIFO Overflow Interrupt Clear Register    (0x110)*/
    volatile uint32_t xrxoicr;
    /* SPI XIP time out register for continuous transfers        (0x114)*/
    volatile uint32_t xip_cnt_time_out;
    volatile uint32_t endian;
} __attribute__((packed, aligned(4))) spi_t;
/* clang-format on */


typedef enum _spi_mode
{
    SPI_MODE_0,
    SPI_MODE_1,
    SPI_MODE_2,
    SPI_MODE_3,
} spi_mode_t;

typedef enum _spi_frame_format
{
    SPI_FF_STANDARD,
    SPI_FF_DUAL,
    SPI_FF_QUAD,
    SPI_FF_OCTAL
} spi_frame_format_t;

typedef enum _spi_addr_inst_trans_mode
{
    SPI_AITM_STANDARD,
    SPI_AITM_ADDR_STANDARD,
    SPI_AITM_AS_FRAME_FORMAT
} spi_addr_inst_trans_mode_t;

typedef enum _spi_transfer_mode
{
    SPI_TMOD_TRANS_RECV,
    SPI_TMOD_TRANS,
    SPI_TMOD_RECV,
    SPI_TMOD_EEROM
} spi_transfer_mode_t;


typedef enum _spi_transfer_width
{
    SPI_TRANS_CHAR  = 0x0,
    SPI_TRANS_SHORT = 0x1,
    SPI_TRANS_INT   = 0x2,
} spi_transfer_width_t;


extern volatile spi_t *const spi[4];


/**
 * @brief       Spi initialize
 *
 * @param[in]   spi_bus     Spi bus number
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_init(uint8_t spi_bus);

/**
 * @brief       Spi master mode configuration
 *
 * @param[in]   spi_bus             Spi bus number
 * @param[in]   mode                Spi mode
 * @param[in]   frame_format        Spi frame format
 * @param[in]   data_bit_length     Spi data bit length
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_master_config(uint8_t spi_bus, spi_mode_t mode, spi_frame_format_t frame_format,
                           size_t data_bit_length);

/**
 * @brief       Spi transmit configuration
 *
 * @param[in]   spi_bus                 Spi bus number
 * @param[in]   instruction_length      Instruction length
 * @param[in]   address_length          Address length
 * @param[in]   wait_cycles             Wait cycles
 * @param[in]   trans_mode              Spi transfer mode
 *
 */
void spi_trans_config(uint8_t spi_bus, size_t instruction_length, size_t address_length,
                           size_t wait_cycles, spi_addr_inst_trans_mode_t trans_mode);

/**
 * @brief       Spi send data
 *
 * @param[in]   spi_bus         Spi bus number
 * @param[in]   chip_sel        Spi chip select
 * @param[in]   cmd_buff        Spi command buffer point
 * @param[in]   cmd_len         Spi command length
 * @param[in]   tx_buff         Spi transmit buffer point
 * @param[in]   tx_len          Spi transmit buffer length
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_send_data(uint8_t spi_bus, uint32_t chip_sel, uint8_t *cmd_buff, size_t cmd_len, uint8_t *tx_buff, size_t tx_len);


/**
 * @brief       Spi receive data
 *
 * @param[in]   spi_bus             Spi bus number
 * @param[in]   chip_sel            Spi chip select
 * @param[in]   cmd_buff            Spi command buffer point
 * @param[in]   cmd_len             Spi command length
 * @param[in]   rx_buff             Spi receive buffer point
 * @param[in]   rx_len              Spi receive buffer length
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_receive_data(uint8_t spi_bus, uint32_t chip_sel, uint8_t *cmd_buff, size_t cmd_len, uint8_t *rx_buff, size_t rx_len);


/**
 * @brief       Spi special receive data
 *
 * @param[in]   spi_bus         Spi bus number
 * @param[in]   chip_sel        Spi chip select
 * @param[in]   cmd_buff        Spi command buffer point
 * @param[in]   cmd_len         Spi command length
 * @param[in]   rx_buff         Spi receive buffer point
 * @param[in]   rx_len          Spi receive buffer length
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_quad_receive_data(uint8_t spi_bus, uint32_t chip_sel, uint32_t *cmd_buff, size_t cmd_len, uint8_t *rx_buff, size_t rx_len);

/**
 * @brief       Spi special send data
 *
 * @param[in]   spi_bus         Spi bus number
 * @param[in]   chip_sel        Spi chip select
 * @param[in]   cmd_buff        Spi command buffer point
 * @param[in]   cmd_len         Spi command length
 * @param[in]   tx_buff         Spi transmit buffer point
 * @param[in]   tx_len          Spi transmit buffer length
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_quad_send_data(uint8_t spi_bus, uint32_t chip_sel, uint32_t *cmd_buff, size_t cmd_len, uint8_t *tx_buff, size_t tx_len);

/**
 * @brief       Spi set transfer mode
 *
 * @param[in]   spi_bus     Spi bus number
 * @param[in]   tmod        Spi tmod
 *
 */
void spi_set_tmod(uint8_t spi_bus, uint32_t tmod);

/**
 * @brief       Spi set frame format
 *
 * @param[in]   spi_bus     Spi bus number
 * @param[in]   spi_frf     Spi frame format
 *
 */
void spi_set_frame_format(uint8_t spi_bus, uint32_t spi_frf);

/**
 * @brief       Spi get frame format
 *
 * @param[in]   spi_bus     Spi bus number
 *
 * @return      frame foramt
 */
int spi_get_frame_format(uint8_t spi_bus);

/**
 * @brief       Spi set work mode
 *
 * @param[in]   spi_bus     Spi bus number
 * @param[in]   mode        Spi work mode
 *
 */
void spi_set_work_mode(uint8_t spi_bus, spi_mode_t mode);

/**
 * @brief       Spi set frame size
 *
 * @param[in]   spi_bus     Spi bus number
 * @param[in]   dfs         Spi frame size
 *
 */
void spi_set_frame_size(uint8_t spi_bus, uint32_t dfs);

/**
 * @brief       Spi set wait cycles
 *
 * @param[in]   spi_bus     Spi bus number
 * @param[in]   wcycles     Spi wait cycles
 *
 */
void spi_set_wait_cycles(uint8_t spi_bus, uint32_t wcycles);

/**
 * @brief       Spi set instruction length
 *
 * @param[in]   spi_bus                 Spi bus number
 * @param[in]   instruction_length      Spi instruction length
 *
 */
void spi_set_inst_length(uint8_t spi_bus, uint32_t instruction_length);

/**
 * @brief       Spi set address length
 *
 * @param[in]   spi_bus             Spi bus number
 * @param[in]   address_length      Spi address length
 *
 */
void spi_set_address_length(uint8_t spi_bus, uint32_t address_length);

/**
 * @brief       Spi set transfer mode
 *
 * @param[in]   spi_bus         Spi bus number
 * @param[in]   trans_mode      Spi tansfer mode
 *
 */
void spi_set_trans_mode(uint8_t spi_bus, spi_addr_inst_trans_mode_t trans_mode);

/**
 * @brief       Spi send data by dma
 *
 * @param[in]   channel_num     Dmac channel number
 * @param[in]   spi_bus         Spi bus number
 * @param[in]   chip_sel        Spi chip select
 * @param[in]   cmd_buff        Spi command buffer point
 * @param[in]   cmd_len         Spi command length
 * @param[in]   tx_buff         Spi transmit buffer point
 * @param[in]   tx_len          Spi transmit buffer length
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_send_data_dma(dmac_channel_number_t channel_num, uint8_t spi_bus, uint32_t chip_sel,
                        uint8_t *cmd_buff, size_t cmd_len, uint8_t *tx_buff, size_t tx_len);


/**
 * @brief       Spi receive data by dma
 *
 * @param[in]   w_channel_num       Dmac write channel number
 * @param[in]   r_channel_num       Dmac read channel number
 * @param[in]   spi_bus             Spi bus number
 * @param[in]   chip_sel            Spi chip select
 * @param[in]   cmd_buff            Spi command buffer point
 * @param[in]   cmd_len             Spi command length
 * @param[in]   rx_buff             Spi receive buffer point
 * @param[in]   rx_len              Spi receive buffer length
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_receive_data_dma(dmac_channel_number_t channel_num_w, dmac_channel_number_t channel_num_r,
                            uint8_t spi_bus, uint32_t chip_sel, uint8_t *cmd_buff, size_t cmd_len, uint8_t *rx_buff, size_t rx_len);


/**
 * @brief       Spi special send data by dma
 *
 * @param[in]   channel_num     Dmac channel number
 * @param[in]   spi_bus         Spi bus number
 * @param[in]   chip_sel        Spi chip select
 * @param[in]   cmd_buff        Spi command buffer point
 * @param[in]   cmd_len         Spi command length
 * @param[in]   tx_buff         Spi transmit buffer point
 * @param[in]   tx_len          Spi transmit buffer length
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_quad_send_data_dma(dmac_channel_number_t channel_num,uint8_t spi_bus, uint32_t chip_sel,
                                uint32_t *cmd_buff, size_t cmd_len, uint8_t *tx_buff, size_t tx_len);

/**
 * @brief       Spi special receive data by dma
 *
 * @param[in]   w_channel_num       Dmac write channel number
 * @param[in]   r_channel_num       Dmac read channel number
 * @param[in]   spi_bus             Spi bus number
 * @param[in]   chip_sel            Spi chip select
 * @param[in]   cmd_buff            Spi command buffer point
 * @param[in]   cmd_len             Spi command length
 * @param[in]   rx_buff             Spi receive buffer point
 * @param[in]   rx_len              Spi receive buffer length
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_quad_receive_data_dma(dmac_channel_number_t channel_num_w, dmac_channel_number_t channel_num_r,
                                        uint8_t spi_bus, uint32_t chip_sel, uint32_t *cmd_buff, size_t cmd_len, uint8_t *rx_buff, size_t rx_len);

/**
 * @brief       Spi fill dma
 *
 * @param[in]   channel_num     Dmac channel number
 * @param[in]   spi_bus         Spi bus number
 * @param[in]   chip_sel        Spi chip select
 * @param[in]   cmd_buff        Spi command buffer point
 * @param[in]   cmd_len         Spi command length
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_fill_dma(dmac_channel_number_t channel_num,uint8_t spi_bus, uint32_t chip_sel, uint32_t *cmd_buff, size_t cmd_len);

/**
 * @brief       Spi normal send by dma
 *
 * @param[in]   channel_num     Dmac channel number
 * @param[in]   spi_bus         Spi bus number
 * @param[in]   chip_sel        Spi chip select
 * @param[in]   tx_buff         Spi transmit buffer point
 * @param[in]   tx_len          Spi transmit buffer length
 * @param[in]   stw             Spi transfer width
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int spi_normal_send_dma(dmac_channel_number_t channel_num, uint8_t spi_bus, uint32_t chip_sel,
                        void *tx_buff, size_t tx_len, spi_transfer_width_t stw);

/**
 * @brief       Spi normal send by dma
 *
 * @param[in]   spi_bus         Spi bus number
 * @param[in]   spi_clk         Spi clock rate
 *
 * @return      The real spi clock rate
 */
uint32_t spi_set_clk_rate(uint8_t spi_bus, uint32_t spi_clk);

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_SPI_H */
