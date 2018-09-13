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
#ifndef _DRIVER_I2S_H
#define _DRIVER_I2S_H

#include <stdint.h>
#include "platform.h"
#include "io.h"
#include "dmac.h"

#ifdef __cplusplus
extern "C" {
#endif


#define I2S0_IN_D0  90
#define I2S0_SCLK   88
#define I2S0_WS    89

enum i2s_device_num_t
{
    I2S_DEVICE_0 = 0,
    I2S_DEVICE_1 = 1,
    I2S_DEVICE_2 = 2,
    I2S_DEVICE_MAX
};

enum i2s_channel_num_t
{
    CHANNEL_0 = 0,
    CHANNEL_1 = 1,
    CHANNEL_2 = 2,
    CHANNEL_3 = 3
};

enum i2s_transmit_t
{
    TRANSMITTER = 0,
    RECEIVER = 1
};

enum i2s_work_mode_t
{
    STANDARD_MODE = 1,
    RIGHT_JUSTIFYING_MODE = 2,
    LEFT_JUSTIFYING_MODE = 4
};

enum sclk_gating_cycles_t
{
    /* Clock gating is diable */
    NO_CLOCK_GATING = 0x0,
    /* Gating after 12 sclk cycles */
    CLOCK_CYCLES_12 = 0x1,
    /* Gating after 16 sclk cycles */
    CLOCK_CYCLES_16 = 0x2,
    /* Gating after 20 sclk cycles */
    CLOCK_CYCLES_20 = 0x3,
    /* Gating after 24 sclk cycles */
    CLOCK_CYCLES_24 = 0x4
};

enum word_select_cycles_t
{
    /* 16 sclk cycles */
    SCLK_CYCLES_16 = 0x0,
    /* 24 sclk cycles */
    SCLK_CYCLES_24 = 0x1,
    /* 32 sclk cycles */
    SCLK_CYCLES_32 = 0x2

};

enum word_length_t
{
    /* Ignore the word length */
    IGNORE_WORD_LENGTH = 0x0,
    /* 12-bit data resolution of the receiver */
    RESOLUTION_12_BIT = 0x1,
    /* 16-bit data resolution of the receiver */
    RESOLUTION_16_BIT = 0x2,
    /* 20-bit data resolution of the receiver */
    RESOLUTION_20_BIT = 0x3,
    /* 24-bit data resolution of the receiver */
    RESOLUTION_24_BIT = 0x4,
    /* 32-bit data resolution of the receiver */
    RESOLUTION_32_BIT = 0x5
};

enum fifo_threshold_t
{
    /* Interrupt trigger when FIFO level is 1 */
    TRIGGER_LEVEL_1 = 0x0,
    /* Interrupt trigger when FIFO level is 2 */
    TRIGGER_LEVEL_2 = 0x1,
    /* Interrupt trigger when FIFO level is 3 */
    TRIGGER_LEVEL_3 = 0x2,
    /* Interrupt trigger when FIFO level is 4 */
    TRIGGER_LEVEL_4 = 0x3,
    /* Interrupt trigger when FIFO level is 5 */
    TRIGGER_LEVEL_5 = 0x4,
    /* Interrupt trigger when FIFO level is 6 */
    TRIGGER_LEVEL_6 = 0x5,
    /* Interrupt trigger when FIFO level is 7 */
    TRIGGER_LEVEL_7 = 0x6,
    /* Interrupt trigger when FIFO level is 8 */
    TRIGGER_LEVEL_8 = 0x7,
    /* Interrupt trigger when FIFO level is 9 */
    TRIGGER_LEVEL_9 = 0x8,
    /* Interrupt trigger when FIFO level is 10 */
    TRIGGER_LEVEL_10 = 0x9,
    /* Interrupt trigger when FIFO level is 11 */
    TRIGGER_LEVEL_11 = 0xa,
    /* Interrupt trigger when FIFO level is 12 */
    TRIGGER_LEVEL_12 = 0xb,
    /* Interrupt trigger when FIFO level is 13 */
    TRIGGER_LEVEL_13 = 0xc,
    /* Interrupt trigger when FIFO level is 14 */
    TRIGGER_LEVEL_14 = 0xd,
    /* Interrupt trigger when FIFO level is 15 */
    TRIGGER_LEVEL_15 = 0xe,
    /* Interrupt trigger when FIFO level is 16 */
    TRIGGER_LEVEL_16 = 0xf
};


struct i2s_ier_t
{
    /* Bit 0 is ien, 0 for disable i2s and 1 for enable i2s */
    uint32_t ien : 1;
    /* Bits [31:1] is reserved */
    uint32_t resv : 31;
} __attribute__((packed, aligned(4)));

union ier_u
{
    struct i2s_ier_t ier;
    uint32_t reg_data;
};

struct i2s_irer_t
{
    /* Bit 0 is receiver block  enable,
     * 0 for receiver disable
     * 1 for receiver enable
     */
    uint32_t rxen : 1;
    /* Bits [31:1] is reserved */
    uint32_t resv : 31;
} __attribute__((packed, aligned(4)));

union irer_u
{
    struct i2s_irer_t irer;
    uint32_t reg_data;
};

struct i2s_iter_t
{
    uint32_t txen : 1;
    /* Bit 0 is transmitter block  enable,
     * 0 for transmitter disable
     * 1 for transmitter enable
     */
    uint32_t resv : 31;
    /* Bits [31:1] is reserved */
} __attribute__((packed, aligned(4)));

union iter_u
{
    struct i2s_iter_t iter;
    uint32_t reg_data;
};

struct i2s_cer_t
{
    uint32_t clken : 1;
    /* Bit 0 is clock generation enable/disable,
     * 0 for clock generation disable,
     * 1 for clock generation enable
     */
    uint32_t resv : 31;
    /* Bits [31:1] is reserved */
} __attribute__((packed, aligned(4)));

union cer_u
{
    struct i2s_cer_t cer;
    uint32_t reg_data;
};

struct i2s_ccr_t
{
    /* Bits [2:0] is used to program  the gating of sclk,
     * 0x0 for clock gating is diable,
     * 0x1 for gating after 12 sclk cycles
     * 0x2 for gating after 16 sclk cycles
     * 0x3 for gating after 20 sclk cycles
     * 0x4 for gating after 24 sclk cycles
     */
    uint32_t clk_gate : 3;
    /* Bits [4:3] used program  the number of sclk cycles for which the
     * word select line stayd in the left aligned or right aligned mode.
     * 0x0 for 16sclk cycles, 0x1 for 24 sclk cycles 0x2 for 32 sclk
     * cycles
     */
    uint32_t clk_word_size : 2;
    /* Bit[5:7] is alignment mode setting.
     * 0x1 for standard i2s format
     * 0x2 for right aligned format
     * 0x4 for left aligned format
     */
    uint32_t align_mode : 3;
    /* Bit[8] is DMA transmit enable control */
    uint32_t dma_tx_en : 1;
    /* Bit[9] is DMA receive enable control */
    uint32_t dma_rx_en : 1;
    uint32_t dma_divide_16 : 1;
    /* Bit[10] split 32bit data to two 16 bit data and filled in left
     * and right channel. Used with dma_tx_en or dma_rx_en
     */
    uint32_t sign_expand_en : 1;
    uint32_t resv : 20;
    /* Bits [31:11] is reseved */
} __attribute__((packed, aligned(4)));

union ccr_u
{
    struct i2s_ccr_t ccr;
    uint32_t reg_data;
};

struct i2s_rxffr_t
{
    uint32_t rxffr : 1;
    /* Bit 0 is receiver FIFO reset,
     * 0 for does not flush RX FIFO, 1 for flush RX FIFO
     */
    uint32_t resv : 31;
    /* Bits [31:1] is reserved */
} __attribute__((packed, aligned(4)));

union rxffr_u
{
    struct i2s_rxffr_t rxffr;
    uint32_t reg_data;
};

struct i2s_lrbrthr
{
    uint32_t fifo : 16;
    /* Bits [15:0] if used data receive or transmit */
    uint32_t resv : 16;
};

union lrbthr_u
{
    struct i2s_lrbrthr buffer;
    uint32_t reg_data;
};

struct i2s_rthr_t
{
    /* Bits  [15:0] is right stereo data transmitted serially
     * from transmit channel input
     */
    uint32_t rthrx : 16;
    /* Bits [31:16] is reserved */
    uint32_t resv : 16;
} __attribute__((packed, aligned(4)));

union rthr_u
{
    struct i2s_rthr_t rthr;
    uint32_t reg_data;
};

struct i2s_rer_t
{
    /* Bit 0 is receive channel enable/disable, 0 for receive channel disable,
     *1 for receive channel enable
     */
    uint32_t rxchenx : 1;
    /* Bits [31:1] is reseved */
    uint32_t resv : 31;
} __attribute__((packed, aligned(4)));

union rer_u
{
    struct i2s_rer_t rer;
    uint32_t reg_data;
};

struct i2s_ter_t
{
    /* Bit 0 is transmit channel enable/disable, 0 for transmit channel disable,
     * 1 for transmit channel enable
     */
    uint32_t txchenx : 1;
    /* Bits [31:1] is reseved */
    uint32_t resv : 31;
} __attribute__((packed, aligned(4)));

union ter_u
{
    struct i2s_ter_t ter;
    uint32_t reg_data;
};

struct i2s_rcr_tcr_t
{
    /* Bits [2:0] is used to program desired data resolution of
     * receiver/transmitter,
     * 0x0 for ignore the word length
     * 0x1 for 12-bit data resolution of the receiver/transmitter,
     * 0x2 for 16-bit data resolution of the receiver/transmitter,
     * 0x3 for 20-bit data resolution of the receiver/transmitter,
     * 0x4 for 24-bit data resolution of the receiver/transmitter,
     * 0x5 for 32-bit data resolution of the receiver/transmitter
     */
    uint32_t wlen : 3;
    /* Bits [31:3] is reseved */
    uint32_t resv : 29;
} __attribute__((packed, aligned(4)));

union rcr_tcr_u {
    struct i2s_rcr_tcr_t rcr_tcr;
    uint32_t reg_data;
};

struct i2s_isr_t {
    /* Bit 0 is status of receiver data avaliable interrupt
     * 0x0 for RX FIFO trigger level not reached
     * 0x1 for RX FIFO trigger level is reached
     */
    uint32_t rxda : 1;
    /* Bit 1 is status of data overrun interrupt for rx channel
     * 0x0 for RX FIFO write valid
     * 0x1 for RX FIFO write overrun
     */
    uint32_t rxfo : 1;
    /* Bits [3:2] is reserved */
    uint32_t resv1 : 2;
    /* Bit 4 is status of transmit empty triger interrupt
     * 0x0 for TX FIFO triiger level is reach
     * 0x1 for TX FIFO trigger level is not reached
     */
    uint32_t txfe : 1;
    /* BIt 5 is status of data overrun interrupt for the TX channel
     * 0x0 for TX FIFO write valid
     * 0x1 for TX FIFO write overrun
     */
    uint32_t txfo : 1;
    /* BIts [31:6] is reserved */
    uint32_t resv2 : 26;
} __attribute__((packed, aligned(4)));

union isr_u {
    struct i2s_isr_t isr;
    uint32_t reg_data;
};

struct i2s_imr_t
{
    /* Bit 0 is mask RX FIFO data available interrupt
     * 0x0 for unmask RX FIFO data available interrupt
     * 0x1 for mask  RX FIFO data available interrupt
     */
    uint32_t rxdam : 1;
    /* Bit 1 is mask RX FIFO overrun interrupt
     * 0x0 for unmask RX FIFO overrun interrupt
     * 0x1 for mask RX FIFO  overrun interrupt
     */
    uint32_t rxfom : 1;
    /* Bits [3:2] is reserved */
    uint32_t resv1 : 2;
    /* Bit 4 is mask TX FIFO empty interrupt,
     * 0x0 for unmask TX FIFO empty interrupt,
     * 0x1 for mask TX FIFO empty interrupt
     */
    uint32_t txfem : 1;
    /* BIt 5 is mask TX FIFO overrun interrupt
     * 0x0 for mask TX FIFO overrun interrupt
     * 0x1 for unmash TX FIFO overrun interrupt
     */
    uint32_t txfom : 1;
    /* Bits [31:6] is reserved */
    uint32_t resv2 : 26;
} __attribute__((packed, aligned(4)));

union imr_u
{
    struct i2s_imr_t imr;
    uint32_t reg_data;
};

struct i2s_ror_t
{
    /* Bit 0 is read this bit to clear RX FIFO data overrun interrupt
     * 0x0 for RX FIFO write valid,
     *0x1 for RX FIFO write overrun
     */
    uint32_t rxcho : 1;
    /* Bits [31:1] is reserved */
    uint32_t resv : 31;
} __attribute__((packed, aligned(4)));

union ror_u
{
    struct i2s_ror_t ror;
    uint32_t reg_data;
};

struct i2s_tor_t
{
    /* Bit 0 is read this bit to clear TX FIFO data overrun interrupt
     * 0x0 for TX FIFO write valid,
     *0x1 for TX FIFO write overrun
     */
    uint32_t txcho : 1;
    /* Bits [31:1] is reserved */
    uint32_t resv : 31;
} __attribute__((packed, aligned(4)));

union tor_u
{
    struct i2s_tor_t tor;
    uint32_t reg_data;
};

struct i2s_rfcr_t
{
    /* Bits [3:0] is used program the trigger level in the RX FIFO at
     * which the receiver data available interrupt generate,
     * 0x0 for interrupt trigger when FIFO level is 1,
     * 0x2 for interrupt trigger when FIFO level is 2,
     * 0x3 for interrupt trigger when FIFO level is 4,
     * 0x4 for interrupt trigger when FIFO level is 5,
     * 0x5 for interrupt trigger when FIFO level is 6,
     * 0x6 for interrupt trigger when FIFO level is 7,
     * 0x7 for interrupt trigger when FIFO level is 8,
     * 0x8 for interrupt trigger when FIFO level is 9,
     * 0x9 for interrupt trigger when FIFO level is 10,
     * 0xa for interrupt trigger when FIFO level is 11,
     * 0xb for interrupt trigger when FIFO level is 12,
     * 0xc for interrupt trigger when FIFO level is 13,
     * 0xd for interrupt trigger when FIFO level is 14,
     * 0xe for interrupt trigger when FIFO level is 15,
     * 0xf for interrupt trigger when FIFO level is 16
     */
    uint32_t rxchdt : 4;
    /* Bits [31:4] is reserved */
    uint32_t rsvd_rfcrx : 28;
} __attribute__((packed, aligned(4)));

union rfcr_u
{
    struct i2s_rfcr_t rfcr;
    uint32_t reg_data;
};

struct i2s_tfcr_t
{
    /* Bits [3:0] is used program the trigger level in the TX FIFO at
     * which the receiver data available interrupt generate,
     * 0x0 for interrupt trigger when FIFO level is 1,
     * 0x2 for interrupt trigger when FIFO level is 2,
     * 0x3 for interrupt trigger when FIFO level is 4,
     * 0x4 for interrupt trigger when FIFO level is 5,
     * 0x5 for interrupt trigger when FIFO level is 6,
     * 0x6 for interrupt trigger when FIFO level is 7,
     * 0x7 for interrupt trigger when FIFO level is 8,
     * 0x8 for interrupt trigger when FIFO level is 9,
     * 0x9 for interrupt trigger when FIFO level is 10,
     * 0xa for interrupt trigger when FIFO level is 11,
     * 0xb for interrupt trigger when FIFO level is 12,
     * 0xc for interrupt trigger when FIFO level is 13,
     * 0xd for interrupt trigger when FIFO level is 14,
     * 0xe for interrupt trigger when FIFO level is 15,
     * 0xf for interrupt trigger when FIFO level is 16
     */
    uint32_t txchet : 4;
    /* Bits [31:4] is reserved */
    uint32_t rsvd_tfcrx : 28;
} __attribute__((packed, aligned(4)));

union tfcr_u
{
    struct i2s_tfcr_t tfcr;
    uint32_t reg_data;
};

struct i2s_rff_t
{
    /* Bit  0 is receiver channel FIFO reset,
     * 0x0 for does not flush an individual RX FIFO,
     * 0x1 for flush an indiviadual RX FIFO
     */
    uint32_t rxchfr : 1;
    /*< Bits [31:1] is reserved ,write only */
    uint32_t rsvd_rffx : 31;
} __attribute__((packed, aligned(4)));

union rff_u
{
    struct i2s_rff_t rff;
    uint32_t reg_data;
};

struct i2s_tff_t
{
    /* Bit  0 is transmit channel FIFO reset,
     * 0x0 for does not flush an individual TX FIFO,
     * 0x1 for flush an indiviadual TX FIFO
     */
    uint32_t rtxchfr : 1;
    /*< Bits [31:1] is reserved ,write only */
    uint32_t rsvd_rffx : 31;
} __attribute__((packed, aligned(4)));

union tff_u
{
    struct i2s_tff_t tff;
    uint32_t reg_data;
};

struct i2s_channel_t
{
    /* Left  Receive or Left Transmit Register      (0x20) */
    volatile uint32_t left_rxtx;
    /* Right Receive or Right Transmit Register     (0x24) */
    volatile uint32_t right_rxtx;
    /* Receive Enable Register                      (0x28) */
    volatile uint32_t rer;
    /* Transmit Enable Register                     (0x2c) */
    volatile uint32_t ter;
    /* Receive Configuration Register               (0x30) */
    volatile uint32_t rcr;
    /* Transmit Configuration Register              (0x34) */
    volatile uint32_t tcr;
    /* Interrupt Status Register                    (0x38) */
    volatile uint32_t isr;
    /* Interrupt Mask Register                      (0x3c) */
    volatile uint32_t imr;
    /* Receive Overrun Register                     (0x40) */
    volatile uint32_t ror;
    /* Transmit Overrun Register                    (0x44) */
    volatile uint32_t tor;
    /* Receive FIFO Configuration Register          (0x48) */
    volatile uint32_t rfcr;
    /* Transmit FIFO Configuration Register         (0x4c) */
    volatile uint32_t tfcr;
    /* Receive FIFO Flush Register                  (0x50) */
    volatile uint32_t rff;
    /* Transmit FIFO Flush Register                 (0x54) */
    volatile uint32_t tff;
    /* reserved                                (0x58-0x5c) */
    volatile uint32_t reserved1[2];
} __attribute__((packed, aligned(4)));

/****is* i2s.api/dw_i2s_portmap
 * NAME
 *  i2s_t
 * DESCRIPTION
 *  This is the structure used for accessing the i2s register
 *  portmap.
 * EXAMPLE
 *  struct i2s_t *portmap;
 *  portmap = (struct dw_i2s_portmap *) DW_APB_I2S_BASE;
 * SOURCE
 */
struct i2s_t
{
    /* I2S Enable Register                          (0x00) */
    volatile uint32_t ier;
    /* I2S Receiver Block Enable Register           (0x04) */
    volatile uint32_t irer;
    /* I2S Transmitter Block Enable Register        (0x08) */
    volatile uint32_t iter;
    /* Clock Enable Register                        (0x0c) */
    volatile uint32_t cer;
    /* Clock Configuration Register                 (0x10) */
    volatile uint32_t ccr;
    /* Receiver Block FIFO Reset Register           (0x04) */
    volatile uint32_t rxffr;
    /* Transmitter Block FIFO Reset Register        (0x18) */
    volatile uint32_t txffr;
    /* reserved                                     (0x1c) */
    volatile uint32_t reserved1;
    volatile struct i2s_channel_t channel[4];
    /* reserved                               (0x118-0x1bc) */
    volatile uint32_t reserved2[40];
    /*  Receiver Block DMA Register                 (0x1c0) */
    volatile uint32_t rxdma;
    /* Reset Receiver Block DMA Register            (0x1c4) */
    volatile uint32_t rrxdma;
    /* Transmitter Block DMA Register               (0x1c8) */
    volatile uint32_t txdma;
    /* Reset Transmitter Block DMA Register         (0x1cc) */
    volatile uint32_t rtxdma;
    /* reserved                               (0x1d0-0x1ec) */
    volatile uint32_t reserved3[8];
    /* Component Parameter Register 2               (0x1f0) */
    volatile uint32_t i2s_comp_param_2;
    /* Component Parameter Register 1               (0x1f4) */
    volatile uint32_t i2s_comp_param_1;
    /* I2S Component Version Register               (0x1f8) */
    volatile uint32_t i2s_comp_version_1;
    /* I2S Component Type Register                  (0x1fc) */
    volatile uint32_t i2s_comp_type;
} __attribute__((packed, aligned(4)));

/**
 * @brief      I2S object instance
 */
extern volatile struct i2s_t *const i2s[3];

/**
 * @brief       i2s device
 *
 * @param[in]   device_num      the device of i2s
 *
 */
void i2s_device_enable(enum i2s_device_num_t device_num);

/**
 * @brief       Enable or disable i2s device
 *
 * @param[in]   device_num      The device of i2s
 * @param[in]   enable          Enable flag 0:disable, 1:enable
 */
void i2s_dev_enable(enum i2s_device_num_t device_num, uint32_t enable);

/**
 * @brief       Set I2S recive channel enable or disable
 *
 * @param[in]   device_num      which of device
 * @param[in]   channel_num     The channel number
 * @param[in]   enable          The enable or disable
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_receive_channel_enable(enum i2s_device_num_t device_num,
                   enum i2s_channel_num_t channel_num, uint32_t enable);

/**
 * @brief       Set I2S transmit channel enable or disable
 *
 * @param[in]   device_num      which of device
 * @param[in]   channel_num     The channel number
 * @param[in]   enable          The enable or disable
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_transmit_channel_enable(enum i2s_device_num_t device_num,
                enum i2s_channel_num_t channel_num, uint32_t enable);

/**
 * @brief       Read pcm data  from channel_num channel
 *
 * @param[in]   device_num      which of device
 * @param[in]   channel_num     The channel number
 * @param[in]   buf             save read data
 * @param[in]   length          the length to read form i2s
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int32_t i2s_receive_data(enum i2s_device_num_t device_num,
      enum i2s_channel_num_t channel_num, uint64_t *buf,
      uint32_t length);

/**
 * @brief       Read pcm data from dma
 *
 * @param[in]   device_num      which of device
 * @param[in]   buf             save read data
 * @param[in]   length          the length to read form i2s
 * @param[in]   channel_num     The dma channel number
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int32_t i2s_receive_data_dma(enum i2s_device_num_t device_num, uint32_t *buf,
    uint32_t length, dmac_channel_number channel_num);

/**
 * @brief       Read pcm data from dma
 *
 * @param[in]   device_num      which of device
 * @param[in]   buf             save read data
 * @param[in]   length          the length to read form i2s
 * @param[in]   channel_num     The dma channel number
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int32_t i2s_recv_dma(enum i2s_device_num_t device_num, uint32_t *buf,
    uint32_t length, dmac_channel_number channel_num);

/**
 * @brief       Mask or unmask interrupt
 *
 * @param[in]   channel_num         The channel number
 * @param[in]   rx_available        The receive available interrupt
 * @param[in]   rx_overrun_int      The receive overrun interrupt
 * @param[in]   tx_empty_int        The transmit empty interrupt
 * @param[in]   tx_overrun_int      The transmit overrun interrupt
 * @param[in]   device_num          which of device
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_set_mask_interrupt(enum i2s_device_num_t device_num,
               enum i2s_channel_num_t channel_num,
               uint32_t rx_available, uint32_t rx_overrun_int,
               uint32_t tx_empty_int, uint32_t tx_overrun_int);
/**
 * @brief       Set transmit threshold
 *
 * @param[in]   threshold       The threshold data
 * @param[in]   channel_num     The channel number
 * @param[in]   device_num      which of device
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_set_tx_threshold(enum i2s_device_num_t device_num,
             enum fifo_threshold_t threshold,
             enum i2s_channel_num_t channel_num);

/**
 * @brief       Set receive threshold
 *
 * @param[in]   threshold       The threshold data
 * @param[in]   channel_num     The channel number
 * @param[in]   device_num      which of device
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_set_rx_threshold(enum i2s_device_num_t device_num,
             enum fifo_threshold_t threshold,
             enum i2s_channel_num_t channel_num);

/**
 * @brief       Configure I2s master mode word select size and clock gating param
 *
 * @param[in]   device_num              which of device
 * @param[in]   word_select_size        clock cycle
 * @param[in]   gating_cycles           The sclk gating cycles

 * @param[in]   word_mode               work mode standard,left justify,
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_master_configure(enum i2s_device_num_t device_num,
    enum word_select_cycles_t word_select_size,
    enum sclk_gating_cycles_t gating_cycles,
    enum i2s_work_mode_t word_mode);
/**
 * @brief       Set rx fifo word length
 *
 * @param[in]   device_num      which of device
 * @param[in]   word_length     word length
 * @param[in]   channel_num     The channel number
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_set_rx_word_length(enum i2s_device_num_t device_num,
               enum word_length_t word_length,
               enum i2s_channel_num_t channel_num);

/**
 * @brief       set tx fifo word length
 *
 * @param[in]   device_num      which of device
 * @param[in]   word_length     word length
 * @param[in]   channel_num     The channel number
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_set_tx_word_length(enum i2s_device_num_t device_num,
               enum word_length_t word_length,
               enum i2s_channel_num_t channel_num);

/**
 * @brief       i2s fpioa set and clock configure
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_fpioa_sysctl(void);

/**
 * @brief       i2s receive enable
 *
 * @param[in]   device_num      The device number
 * @param[in]   channel_num     The channel number
 */
void i2s_receive_enable(enum i2s_device_num_t device_num,
            enum i2s_channel_num_t channel_num);

/**
 * @brief       i2s transimit enable
 *
 * @param[in]   device_num      The device number
 * @param[in]   channel_num     The channel number
 */
void i2s_transimit_enable(enum i2s_device_num_t device_num,
              enum i2s_channel_num_t channel_num);

/**
 * @brief       I2S receive channel configure
 *
 * @param[in]   device_num              The device number
 * @param[in]   channel_num             The channel number
 * @param[in]   word_length             The word length
 * @param[in]   word_select_size        The word select size
 * @param[in]   trigger_level           The trigger level
 */
void i2s_rx_channel_configure(enum i2s_device_num_t device_num,
    enum i2s_channel_num_t channel_num,
    enum word_length_t word_length,
    enum word_select_cycles_t word_select_size,
    enum fifo_threshold_t trigger_level,
    enum i2s_work_mode_t word_mode);

/**
 * @brief       I2S transmit channel  enable
 *
 * @param[in]   device_num              The device number
 * @param[in]   channel_num             The channel number
 * @param[in]   word_length             The word length
 * @param[in]   word_select_size        The word select size
 * @param[in]   trigger_level           The trigger level
 */
void i2s_tx_channel_configure(enum i2s_device_num_t device_num,
    enum i2s_channel_num_t channel_num,
    enum word_length_t word_length,
    enum word_select_cycles_t word_select_size,
    enum fifo_threshold_t trigger_level,
    enum i2s_work_mode_t word_mode);

/**
 * @brief       disable block
 *
 * @param[in]   device_num      The device number
 * @param[in]   rxtx_mode       The rxtx mode
 */
void i2s_disable_block(enum i2s_device_num_t device_num,
    enum i2s_transmit_t rxtx_mode);

/**
 * @brief       Enable I2S transmit DMA
 *
 * @param[in]   device_num      The device number
 * @param[in]   enable          The enable
 *
 * @return     result
 *       0     Success
 *       Other Fail
 */
int i2s_transmit_dma_enable(enum i2s_device_num_t device_num, uint32_t enable);

/**
 * @brief       Enable I2S receive DMA
 *
 * @param[in]   device_num      The device number
 * @param[in]   enable          The enable
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_receive_dma_enable(enum i2s_device_num_t device_num, uint32_t enable);

/**
 * @brief       Split I2S transmit DMA from 32bit to two 16bit left and right
 *
 * @param[in]   device_num      The device number
 * @param[in]   enable          The enable
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_transmit_dma_divide(enum i2s_device_num_t device_num, uint32_t enable);

/**
 * @brief       Write pcm data to channel_num channel
 *
 * @param[in]   device_num      The i2s number
 * @param[in]   channel_num     The channel number
 * @param[in]   pcm             32bit (16 bits left and 16bits right)pcm data
 * @param[in]   device_num      which of device
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_transmit_data(enum i2s_device_num_t device_num,
    enum i2s_channel_num_t channel_num, uint8_t *pcm, uint32_t length, uint8_t single_length);

/**
 * @brief       I2s init
 *
 * @param[in]   device_num          The device number
 * @param[in]   rxtx_mode           I2s work mode
 * @param[in]   channel_mask        Channel mask to which channel work
 *
 */
void i2s_init(enum i2s_device_num_t device_num, enum i2s_transmit_t rxtx_mode, uint32_t channel_mask);

/**
 * @brief       Write pcm data to channel_num channel by dma
 *
 * @param[in]   device_num          which of device
 * @param[in]   pcm                 Send data
 * @param[in]   length              Send data length
 * @param[in]   single_length       Send data width
 * @param[in]   channel_num         dmac channel
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_transmit_data_dma(enum i2s_device_num_t device_num,
    void *pcm, uint32_t length, uint8_t single_length, dmac_channel_number channel_num);

/**
 * @brief       Write pcm data to channel_num channel by dma, first wait dmac done
 *
 * @param[in]   device_num      which of device
 * @param[in]   pcm             Send data
 * @param[in]   length          Send data length
 * @param[in]   channel_num     dmac channel
 *
 */
void i2s_send_data_dma(enum i2s_device_num_t device_num,
    void *pcm, uint32_t length, dmac_channel_number channel_num);

/**
 * @brief       Write pcm data to channel_num channel by dma
 *
 * @param[in]   device_num          which of device
 * @param[in]   channel_num         which of device
 * @param[in]   buf                 Send data
 * @param[in]   length              Send data length
 * @param[in]   frame               I2s frame number
 * @param[in]   bits_per_sample     I2s sample bits
 * @param[in]   track_num           track num
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int i2s_play(enum i2s_device_num_t device_num,dmac_channel_number channel_num,
    uint8_t *buf, size_t length, uint32_t frame, uint32_t bits_per_sample, uint8_t track_num);

/**
 * @brief       Write pcm data by dma from i2s to i2s
 *
 * @param[in]   device_src_num      I2s receive
 * @param[in]   device_dest_num     I2s transfer
 * @param[in]   length              Data length
 * @param[in]   channel_num         Dmac channel
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int32_t i2s_special_dma(enum i2s_device_num_t device_src_num, enum i2s_device_num_t device_dest_num,
    uint32_t length, dmac_channel_number channel_num);

#ifdef __cplusplus
}
#endif

#endif
