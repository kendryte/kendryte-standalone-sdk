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
#ifndef _DRIVER_OTP_H
#define _DRIVER_OTP_H

#include <stdint.h>

/* clang-format off */
#define OTP_COMMON_DATA_ADDR    0x00000000U
#define OTP_SYSTEM_DATA_ADDR    0x00003AD0U
#define OTP_BISR_DATA_ADDR      0x00003DD0U
#define OTP_BLOCK_CTL_ADDR      0x00003FD0U
#define OTP_WIRED_REG_ADDR      0x00003FE0U
#define OTP_AES_KEY_ADDR        0x00003FF0U

#define OTP_BUSY_FLAG           0x00000001U
#define OTP_BYPASS_FLAG         0x00000002U
#define OTP_TEST_FLAG           0x00000004U
/* clang-format on */

typedef enum _otp_status_t
{
    OTP_OK = 0,
    OTP_ERROR_TIMEOUT,  /* operation timeout*/
    OTP_ERROR_ADDRESS,  /* invalid address*/
    OTP_ERROR_WRITE,    /* write error*/
    OTP_ERROR_BLANK,    /* blank check error*/
    OTP_ERROR_BISR,     /* bisr error*/
    OTP_ERROR_TESTDEC,  /* testdec error*/
    OTP_ERROR_WRTEST,   /* wrtest error*/
    OTP_ERROR_KEYCOMP,  /* key is wrong*/
    OTP_ERROR_PARAM,    /* param error*/
    OTP_ERROR_NULL,     /* undefine error*/
    OTP_BLOCK_NORMAL,   /* block can be written*/
    OTP_BLOCK_PROTECTED,/* block can not be written*/
    OTP_FUNC_ENABLE,    /* function available*/
    OTP_FUNC_DISABLE,   /* function unavailable*/
    OTP_FLAG_SET,       /* flag set*/
    OTP_FLAG_UNSET,     /* flag unset*/
} otp_status_t;

typedef enum _otp_data_block_t
{
    COMMON_DATA_BLOCK1 = 0,
    COMMON_DATA_BLOCK2,
    COMMON_DATA_BLOCK3,
    COMMON_DATA_BLOCK4,
    COMMON_DATA_BLOCK5,
    COMMON_DATA_BLOCK6,
    COMMON_DATA_BLOCK7,
    COMMON_DATA_BLOCK8,
    COMMON_DATA_BLOCK9,
    COMMON_DATA_BLOCK10,
    COMMON_DATA_BLOCK11,
    COMMON_DATA_BLOCK12,
    COMMON_DATA_BLOCK13,
    COMMON_DATA_BLOCK14,
    COMMON_DATA_BLOCK15,
    DATA_BLOCK_RESERVE,
    SYSTEM_DATA_BLOCK1,
    SYSTEM_DATA_BLOCK2,
    SYSTEM_DATA_BLOCK3,
    SYSTEM_DATA_BLOCK4,
    SYSTEM_DATA_BLOCK5,
    SYSTEM_DATA_BLOCK6,
    SYSTEM_DATA_BLOCK7,
    SYSTEM_DATA_BLOCK8,
    SYSTEM_DATA_BLOCK9,
    SYSTEM_DATA_BLOCK10,
    SYSTEM_DATA_BLOCK11,
    SYSTEM_DATA_BLOCK12,
    SYSTEM_DATA_BLOCK13,
    SYSTEM_DATA_BLOCK14,
    SYSTEM_DATA_BLOCK15,
    SYSTEM_DATA_BLOCK16,
    SYSTEM_DATA_BLOCK17,
    SYSTEM_DATA_BLOCK18,
    SYSTEM_DATA_BLOCK19,
    SYSTEM_DATA_BLOCK20,
    SYSTEM_DATA_BLOCK21,
    SYSTEM_DATA_BLOCK22,
    SYSTEM_DATA_BLOCK23,
    SYSTEM_DATA_BLOCK24,
    SYSTEM_DATA_BLOCK25,
    SYSTEM_DATA_BLOCK26,
    SYSTEM_DATA_BLOCK27,
    SYSTEM_DATA_BLOCK28,
    SYSTEM_DATA_BLOCK29,
    SYSTEM_DATA_BLOCK30,
    SYSTEM_DATA_BLOCK31,
    SYSTEM_DATA_BLOCK32,
    SYSTEM_DATA_BLOCK33,
    SYSTEM_DATA_BLOCK34,
    SYSTEM_DATA_BLOCK35,
    SYSTEM_DATA_BLOCK36,
    SYSTEM_DATA_BLOCK37,
    SYSTEM_DATA_BLOCK38,
    SYSTEM_DATA_BLOCK39,
    SYSTEM_DATA_BLOCK40,
    SYSTEM_DATA_BLOCK41,
    SYSTEM_DATA_BLOCK42,
    SYSTEM_DATA_BLOCK43,
    SYSTEM_DATA_BLOCK44,
    SYSTEM_DATA_BLOCK45,
    SYSTEM_DATA_BLOCK46,
    SYSTEM_DATA_BLOCK47,
    SYSTEM_DATA_BLOCK48,
    DATA_BLOCK_MAX = 64,
} otp_data_block_t;

typedef enum _otp_func_reg_t
{
    BLANK_TEST_DISABLE = 0,
    RAM_BISR_DISABLE,
    AES_WRITE_DISABLE,
    AES_VERIFY_DISABLE,
    JTAG_DISABLE,
    TEST_EN_DISABLE = 6,
    ISP_DISABLE,
    OTP_FUNC_FIRMWARE_CIPHER_DISABLE,
    FUNC_REG_MAX = 64,
} otp_func_reg_t;

typedef struct _otp_t
{
    volatile uint32_t otp_ceb;
    volatile uint32_t otp_test_mode;
    volatile uint32_t otp_mode;
    volatile uint32_t gb_otp_en;
    volatile uint32_t dat_in_finish;
    volatile uint32_t otp_bisr_fail;
    volatile uint32_t test_step;
    volatile uint32_t otp_pwrrdy;
    volatile uint32_t otp_last_dat;
    volatile uint32_t otp_data;
    volatile uint32_t otp_pwr_mode;
    volatile uint32_t otp_in_dat;
    volatile uint32_t otp_apb_adr;
    volatile uint32_t td_result;
    volatile uint32_t data_acp_flag;
    volatile uint32_t otp_adr_in_flag;
    volatile uint32_t wr_result;
    volatile uint32_t otp_thershold;
    volatile uint32_t bisr_finish;
    volatile uint32_t key_cmp_result;
    volatile uint32_t otp_cmp_key;
    volatile uint32_t cmp_result_rdy;
    volatile uint32_t otp_cle;
    volatile uint32_t data_blk_ctrl;
    volatile uint32_t otp_wrg_adr_flag;
    volatile uint32_t pro_wrong;
    volatile uint32_t otp_status;
    volatile uint32_t otp_pro_adr;
    volatile uint32_t blank_finish;
    volatile uint32_t bisr2otp_en;
    volatile uint32_t otp_cpu_ctrl;
    volatile uint32_t otp_web_cpu;
    volatile uint32_t otp_rstb_cpu;
    volatile uint32_t otp_seltm_cpu;
    volatile uint32_t otp_readen_cpu;
    volatile uint32_t otp_pgmen_cpu;
    volatile uint32_t otp_dle_cpu;
    volatile uint32_t otp_din_cpu;
    volatile uint32_t otp_cpumpen_cpu;
    volatile uint32_t otp_cle_cpu;
    volatile uint32_t otp_ceb_cpu;
    volatile uint32_t otp_adr_cpu;
    volatile uint32_t otp_dat_cpu;
    volatile uint32_t otp_data_rdy;
    volatile uint32_t block_flag_high;
    volatile uint32_t block_flag_low;
    volatile uint32_t reg_flag_high;
    volatile uint32_t reg_flag_low;
} __attribute__((packed, aligned(4))) otp_t;

/**
 * @brief       Init OTP
 *
 * @note        The otp clock frequency is 12.5M by default
 *
 * @param[in]  div	bus_clk / otp_clk
 */
void otp_init(uint8_t div);

/**
 * @brief       Enable otp test mode
 */
void otp_test_enable(void);

/**
 * @brief       Disable otp test mode
 */
void otp_test_disable(void);

/**
 * @brief       Enable key output to aes
 */
void otp_key_output_enable(void);

/**
 * @brief       Disable key output to aes
 */
void otp_key_output_disable(void);

/**
 * @brief       Get the wrong address when programming fails
 *
 * @return      The wrong address
 */
uint32_t otp_wrong_address_get(void);

/**
 * @brief       Get OTP status
 *
 * @param[in]   flag        status flag
 *
 * @return      Results of the operation
 */
otp_status_t otp_status_get(uint32_t flag);

/**
 * @brief       Perform the blank check operation
 *
 * @return      Results of the operation
 */
otp_status_t otp_blank_check(void);

/**
 * @brief       Perform the testdec operation
 *
 * @return      Results of the operation
 */
otp_status_t otp_testdec(void);

/**
 * @brief       Perform the wrtest operation
 *
 * @return      Results of the operation
 */
otp_status_t otp_wrtest(void);

/**
 * @brief       Write data
 *
 * @param[in]   addr            Start programming address(bit)
 * @param[in]   data_buf        Need to write the data point
 * @param[in]   length          Need to write the data length(bit)
 *
 * @return      Results of the operation
 */
otp_status_t otp_write_data(uint32_t addr, uint8_t *data_buf, uint32_t length);

/**
 * @brief       Read data
 *
 * @param[in]   addr            Start read address(bit).
 * @param[in]   data_buf        Need to read the data point
 * @param[in]   length          Need to read the data length(bit)
 *
 * @return     Results of the operation
 */
otp_status_t otp_read_data(uint32_t addr, uint8_t *data_buf, uint32_t length);

/**
 * @brief       Write the key
 *
 * @param[in]   data_buf        The key data,length is 128 bits(4 words)
 *
 * @return      Results of the operation
 */
otp_status_t otp_key_write(uint8_t *data_buf);

/**
 * @brief       Compare the key
 *
 * @param[in]   data_buf        The key data,length is 128 bits(4 words)
 *
 * @return      Results of the operation
 */
otp_status_t otp_key_compare(uint8_t *data_buf);

/**
 * @brief       Data block write protect
 *
 * @param[in]   block       Need to write a protected data block
 *
 * @return      Results of the operation
 */
otp_status_t otp_data_block_protect_set(otp_data_block_t block);

/**
 * @brief       Disable the specified function
 *
 * @param[in]   reg     Need to disable the function
 *
 * @return      Results of the operation
 */
otp_status_t otp_func_reg_disable_set(otp_func_reg_t func);

/**
 * @brief       Get the data block status
 *
 * @param[in]   block       The specified data block
 *
 * @return      Results of the operation
 */
otp_status_t otp_data_block_protect_get(otp_data_block_t block);

/**
 * @brief       Get the function status
 *
 * @param[in]   reg     The specified function
 *
 * @return      Results of the operation
 */
otp_status_t otp_func_reg_disable_get(otp_func_reg_t func);

/**
 * @brief       Refresh the data block status
 *
 * @param[in]   block       The specified data block
 *
 * @return      Results of the operation
 */
otp_status_t otp_data_block_protect_refresh(otp_data_block_t block);

/**
 * @brief       Write data(bypass mode)
 *
 * @param[in]   addr            Start programming address(bit)
 * @param[in]   data_buf        Need to write the data point
 * @param[in]   length          Need to write the data length(bit)
 *
 * @return     Results of the operation
 */
otp_status_t otp_soft_write(uint32_t addr, uint8_t *data_buf, uint32_t length);

/**
 * @brief       Read data(bypass mode)
 *
 * @param[in]   addr            Start read address(bit).Be sure to align 16 bits
 * @param[in]   data_buf        Need to read the data point
 * @param[in]   length          Need to read the data length(half word/16bits)
 *
 * @return      Results of the operation
 */
otp_status_t otp_soft_read(uint32_t addr, uint8_t *data_buf, uint32_t length);

#endif
