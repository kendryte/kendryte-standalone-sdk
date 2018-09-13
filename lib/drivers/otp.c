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
#include "otp.h"
#include "platform.h"
#include "sysctl.h"

/* clang-format off */
#define DELAY_TIMEOUT       0xFFFFFFU
#define WRTEST_NUM          0xA5U
/* clang-format on */

volatile struct otp_t* const otp = (volatile struct otp_t*)OTP_BASE_ADDR;

void otp_init(uint8_t div)
{
    sysctl_clock_enable(SYSCTL_CLOCK_OTP);
    otp->otp_cpu_ctrl = 0;
    otp->otp_thershold = div;
    otp->data_blk_ctrl = 0;
    otp->gb_otp_en = 0;
    otp->otp_pwr_mode = 0;
    otp->otp_web_cpu = 1;
    otp->otp_rstb_cpu = 1;
    otp->otp_seltm_cpu = 0;
    otp->otp_readen_cpu = 0;
    otp->otp_pgmen_cpu = 0;
    otp->otp_dle_cpu = 0;
    otp->otp_din_cpu = 0;
    otp->otp_cpumpen_cpu = 0;
    otp->otp_cle_cpu = 0;
    otp->otp_ceb_cpu = 1;
    otp->otp_adr_cpu = 0;
    otp->otp_dat_cpu = 0;
}

void otp_test_enable(void)
{
    otp->otp_cpu_ctrl = 0xCAAC;
}

void otp_test_disable(void)
{
    otp->otp_cpu_ctrl = 0;
}

void otp_key_output_enable(void)
{
    otp->gb_otp_en = 1;
}

void otp_key_output_disable(void)
{
    otp->gb_otp_en = 0;
}

enum otp_status_t otp_status_get(uint32_t flag)
{
    if (otp->otp_status & flag)
        return OTP_FLAG_SET;
    return OTP_FLAG_UNSET;
}

static enum otp_status_t otp_bisr_write(void)
{
    uint32_t time_out = 0;

    otp->otp_cle = 1;
    otp->otp_mode = 0x02;
    otp->otp_test_mode = 0x30;
    otp->test_step = 0;
    otp->otp_ceb = 0;
    while (otp->bisr_finish == 0)
    {
        time_out++;
        if (time_out >= DELAY_TIMEOUT)
            return OTP_ERROR_TIMEOUT;
    }
    otp->bisr_finish = 0;
    if (otp->pro_wrong)
        return OTP_ERROR_BISR;
    return OTP_OK;
}

static enum otp_status_t otp_bisr_read(void)
{
    uint32_t time_out = 0;

    otp->otp_cle = 1;
    otp->otp_mode = 0x02;
    otp->otp_test_mode = 0x30;
    otp->test_step = 1;
    otp->otp_ceb = 0;
    while (otp->bisr_finish == 0)
    {
        time_out++;
        if (time_out >= DELAY_TIMEOUT)
            return OTP_ERROR_TIMEOUT;
    }
    otp->bisr_finish = 0;
    return OTP_OK;
}

enum otp_status_t otp_blank_check(void)
{
    enum otp_status_t status;
    uint32_t time_out = 0;

    if (otp_func_reg_disable_get(BLANK_TEST_DISABLE) == OTP_FUNC_DISABLE)
        return OTP_FUNC_DISABLE;

    otp->otp_cle = 1;
    otp->otp_mode = 0x02;
    otp->otp_test_mode = 0x24;
    otp->blank_finish = 0;
    otp->otp_ceb = 0;
    while (otp->blank_finish == 0)
    {
        time_out++;
        if (time_out >= DELAY_TIMEOUT)
            return OTP_ERROR_TIMEOUT;
    }
    if (otp->otp_bisr_fail)
        return OTP_ERROR_BLANK;

    status = otp_bisr_write();
    if (status != OTP_OK)
        return status;
    status = otp_bisr_read();
    if (status != OTP_OK)
        return status;
    status = otp_func_reg_disable_set(BLANK_TEST_DISABLE);
    if (status != OTP_OK)
        return status;
    return OTP_OK;
}

enum otp_status_t otp_testdec(void)
{
    uint32_t time_out = 0;

    otp->otp_cle = 1;
    otp->otp_mode = 0x02;
    otp->otp_test_mode = 0x21;
    otp->otp_ceb = 0;
    while (otp->td_result == 0)
    {
        time_out++;
        if (time_out >= DELAY_TIMEOUT)
            return OTP_ERROR_TIMEOUT;
    }
    if (otp->td_result == 0x01)
    {
        otp->td_result = 0;
        return OTP_ERROR_TESTDEC;
    }
    return OTP_OK;
}

enum otp_status_t otp_wrtest(void)
{
    uint16_t addr, data, i, j;
    uint32_t time_out;

    otp->otp_cle = 1;
    otp->otp_mode = 0x02;
    otp->otp_test_mode = 0x01;
    otp->test_step = 0;
    otp->data_acp_flag = 0;
    otp->otp_ceb = 0;
    addr = 0;
    for (i = 0; i < 128; i++)
    {
        data = WRTEST_NUM;
        for (j = 0; j < 8; j++)
        {
            if ((addr == 1023) || (data & 0x01))
            {
                time_out = 0;
                while (otp->otp_adr_in_flag == 0)
                {
                    time_out++;
                    if (time_out >= DELAY_TIMEOUT)
                        return OTP_ERROR_TIMEOUT;
                }
                otp->otp_apb_adr = addr;
                if (addr == 1023)
                {
                    otp->otp_in_dat = data & 0x01;
                    otp->otp_last_dat = 0x01;
                }
                else
                    otp->otp_in_dat = 0x01;
                otp->dat_in_finish = 0x01;
                time_out = 0;
                while (otp->data_acp_flag == 0)
                {
                    time_out++;
                    if (time_out >= DELAY_TIMEOUT)
                        return OTP_ERROR_TIMEOUT;
                }
                if (otp->data_acp_flag == 0x01)
                {
                    otp->data_acp_flag = 0;
                    return OTP_ERROR_WRITE;
                }
                otp->data_acp_flag = 0;
            }
            data >>= 1;
            addr++;
        }
    }
    time_out = 0;
    while ((otp->wr_result & 0x04) == 0)
    {
        time_out++;
        if (time_out >= DELAY_TIMEOUT)
            return OTP_ERROR_TIMEOUT;
    }
    otp->wr_result &= 0xFFFFFFFB;

    otp->otp_cle = 1;
    otp->otp_mode = 0x02;
    otp->otp_test_mode = 0x01;
    otp->test_step = 1;
    otp->data_acp_flag = 0;
    otp->otp_ceb = 0;
    addr = 0;
    for (i = 0; i < 128; i++)
    {
        time_out = 0;
        while (otp->otp_adr_in_flag == 0)
        {
            time_out++;
            if (time_out >= DELAY_TIMEOUT)
                return OTP_ERROR_TIMEOUT;
            if (otp->wr_result == 0x01)
            {
                otp->wr_result = 0;
                return OTP_ERROR_WRTEST;
            }
        }
        otp->otp_in_dat = WRTEST_NUM;
        otp->otp_apb_adr = addr;
        if (i == 127)
            otp->otp_last_dat = 0x01;
        addr += 8;
    }
    time_out = 0;
    while ((otp->wr_result & 0x03) == 0)
    {
        time_out++;
        if (time_out >= DELAY_TIMEOUT)
            return OTP_ERROR_TIMEOUT;
    }
    if ((otp->wr_result & 0x03) == 0x01)
    {
        otp->wr_result = 0;
        return OTP_ERROR_WRTEST;
    }
    return OTP_OK;
}

static enum otp_status_t otp_write_byte(uint32_t addr, uint8_t* data_buf, uint32_t length)
{
    enum otp_status_t status;
    uint8_t data, index;
    uint32_t time_out;

    otp->otp_cle = 0;
    otp->otp_mode = 1;
    otp->data_acp_flag = 0;
    otp->otp_wrg_adr_flag = 0;
    otp->otp_ceb = 0;
    index = 0;
    addr *= 8;
    length *= 8;
    data = *data_buf++;
    while (length--)
    {
        if ((length == 0) || (data & 0x01))
        {
            time_out = 0;
            while (otp->otp_adr_in_flag == 0)
            {
                time_out++;
                if (time_out >= DELAY_TIMEOUT)
                    return OTP_ERROR_TIMEOUT;
            }
            otp->otp_apb_adr = addr;
            if (length == 0)
            {
                otp->otp_in_dat = data & 0x01;
                otp->otp_last_dat = 1;
            }
            else
                otp->otp_in_dat = 1;
            otp->dat_in_finish = 1;
            time_out = 0;
            while (otp->data_acp_flag == 0)
            {
                time_out++;
                if (time_out >= DELAY_TIMEOUT)
                    return OTP_ERROR_TIMEOUT;
            }
            if (otp->otp_wrg_adr_flag == 1)
                return OTP_ERROR_ADDRESS;
            if (otp->data_acp_flag == 1)
                return OTP_ERROR_WRITE;
            otp->data_acp_flag = 0;
        }
        data >>= 1;
        addr++;
        index++;
        if (index == 8)
        {
            index = 0;
            data = *data_buf++;
        }
    }

    status = otp_bisr_write();
    if (status != OTP_OK)
        return status;
    status = otp_bisr_read();
    if (status != OTP_OK)
        return status;
    return OTP_OK;
}

static enum otp_status_t otp_read_byte(uint32_t addr, uint8_t* data_buf, uint32_t length)
{
    uint32_t time_out;

    otp->otp_cle = 0;
    otp->otp_mode = 0;
    otp->otp_wrg_adr_flag = 0;
    otp->otp_ceb = 0;
    addr *= 8;
    while (length--)
    {
        time_out = 0;
        while (otp->otp_adr_in_flag == 0)
        {
            time_out++;
            if (time_out >= DELAY_TIMEOUT)
                return OTP_ERROR_TIMEOUT;
        }
        if (length == 0)
            otp->otp_last_dat = 1;
        otp->otp_apb_adr = addr;
        time_out = 0;
        while (otp->otp_data_rdy == 0)
        {
            time_out++;
            if (time_out >= DELAY_TIMEOUT)
                return OTP_ERROR_TIMEOUT;
        }
        if (otp->otp_wrg_adr_flag == 0x01)
            return OTP_ERROR_ADDRESS;
        *data_buf++ = otp->otp_data;
        addr += 8;
    }
    return OTP_OK;
}

enum otp_status_t otp_write_data(uint32_t addr, uint8_t* data_buf, uint32_t length)
{
    enum otp_status_t status;

    if (addr >= OTP_BISR_DATA_ADDR)
        return OTP_ERROR_ADDRESS;
    length = length <= OTP_BISR_DATA_ADDR - addr ? length : OTP_BISR_DATA_ADDR - addr;

    status = otp_write_byte(addr, data_buf, length);
    if (status == OTP_ERROR_ADDRESS)
        status = OTP_BLOCK_PROTECTED;
    return status;
}

enum otp_status_t otp_read_data(uint32_t addr, uint8_t* data_buf, uint32_t length)
{
    enum otp_status_t status;

    if (addr >= OTP_BISR_DATA_ADDR)
        return OTP_ERROR_ADDRESS;
    length = length <= OTP_BISR_DATA_ADDR - addr ? length : OTP_BISR_DATA_ADDR - addr;

    status = otp_read_byte(addr, data_buf, length);
    if (status == OTP_ERROR_ADDRESS)
        status = OTP_ERROR_NULL;
    return status;
}

enum otp_status_t otp_key_write(uint8_t* data_buf)
{
    enum otp_status_t status;

    status = otp_write_byte(OTP_AES_KEY_ADDR, data_buf, 16);
    if (status == OTP_ERROR_ADDRESS)
        status = OTP_FUNC_DISABLE;
    return status;
}

enum otp_status_t otp_key_compare(uint8_t* data_buf)
{
    uint32_t time_out = 0;

    otp->key_cmp_result = 0;
    otp->otp_cmp_key = ((uint32_t)data_buf[0] << 24) | ((uint32_t)data_buf[1] << 16) | ((uint32_t)data_buf[2] << 8) | (uint32_t)data_buf[3];
    otp->otp_cmp_key = ((uint32_t)data_buf[4] << 24) | ((uint32_t)data_buf[5] << 16) | ((uint32_t)data_buf[6] << 8) | (uint32_t)data_buf[7];
    otp->otp_cmp_key = ((uint32_t)data_buf[8] << 24) | ((uint32_t)data_buf[9] << 16) | ((uint32_t)data_buf[10] << 8) | (uint32_t)data_buf[11];
    otp->otp_cmp_key = ((uint32_t)data_buf[12] << 24) | ((uint32_t)data_buf[13] << 16) | ((uint32_t)data_buf[14] << 8) | (uint32_t)data_buf[15];
    while (otp->key_cmp_result == 0)
    {
        time_out++;
        if (time_out >= DELAY_TIMEOUT)
            return OTP_ERROR_TIMEOUT;
    }
    if (otp->key_cmp_result == 0x01)
        return OTP_ERROR_KEYCOMP;
    else if (otp->key_cmp_result == 0x03)
        return OTP_FUNC_DISABLE;
    return OTP_OK;
}

enum otp_status_t otp_data_block_protect_set(enum otp_data_block_t block)
{
    enum otp_status_t status;
    uint8_t value;

    if (block >= DATA_BLOCK_MAX)
        return OTP_ERROR_PARAM;
    otp->data_blk_ctrl = 0x01;
    value = 0x03 << ((block % 4) * 2);
    status = otp_write_byte(OTP_BLOCK_CTL_ADDR + block / 4, &value, 1);
    otp->data_blk_ctrl = 0;
    return status;
}

enum otp_status_t otp_func_reg_disable_set(enum otp_func_reg_t func)
{
    enum otp_status_t status;
    uint8_t value;

    if (func >= FUNC_REG_MAX)
        return OTP_ERROR_PARAM;
    otp->data_blk_ctrl = 0x01;
    value = 0x03 << ((func % 4) * 2);
    status = otp_write_byte(OTP_WIRED_REG_ADDR + func / 4, &value, 1);
    otp->data_blk_ctrl = 0;
    return status;
}

enum otp_status_t otp_data_block_protect_get(enum otp_data_block_t block)
{
    if (block < DATA_BLOCK_MAX / 2)
    {
        if (otp->block_flag_low & (0x01 << block))
            return OTP_BLOCK_PROTECTED;
    }
    else if (block < DATA_BLOCK_MAX)
    {
        if (otp->block_flag_high & (0x01 << (block - DATA_BLOCK_MAX / 2)))
            return OTP_BLOCK_PROTECTED;
    }
    else
        return OTP_ERROR_PARAM;
    return OTP_BLOCK_NORMAL;
}

enum otp_status_t otp_func_reg_disable_get(enum otp_func_reg_t func)
{
    if (func < FUNC_REG_MAX / 2)
    {
        if (otp->reg_flag_low & (0x01 << func))
            return OTP_FUNC_DISABLE;
    }
    else if (func < FUNC_REG_MAX)
    {
        if (otp->reg_flag_high & (0x01 << (func - FUNC_REG_MAX / 2)))
            return OTP_FUNC_DISABLE;
    }
    else
        return OTP_ERROR_PARAM;
    return OTP_FUNC_ENABLE;
}

enum otp_status_t otp_data_block_protect_refresh(enum otp_data_block_t block)
{
    uint8_t value;

    if (block < DATA_BLOCK_MAX)
        return otp_read_byte(OTP_BLOCK_CTL_ADDR + block / 4, &value, 1);
    else
        return OTP_ERROR_PARAM;
}

enum otp_status_t otp_soft_write(uint32_t addr, uint8_t* data_buf, uint32_t length)
{
    uint8_t data, index, count;

    otp->otp_ceb_cpu = 1;
    otp->otp_cle_cpu = 0;
    otp->otp_seltm_cpu = 0;
    otp->otp_readen_cpu = 0;
    otp->otp_dle_cpu = 0;
    otp->otp_web_cpu = 1;
    otp->otp_cpumpen_cpu = 0;
    otp->otp_pgmen_cpu = 0;
    otp->otp_rstb_cpu = 1;

    otp->otp_ceb_cpu = 0;
    otp->otp_rstb_cpu = 0;
    otp->otp_rstb_cpu = 1;

    index = 0;
    addr *= 8;
    length *= 8;
    data = *data_buf++;
    while (length)
    {
        otp->otp_adr_cpu = addr;
        otp->otp_din_cpu = data & 0x01;
        otp->otp_dle_cpu = 1;
        otp->otp_web_cpu = 0;
        otp->otp_web_cpu = 1;
        otp->otp_dle_cpu = 0;
        count = 20;
        while (count--)
        {
            otp->otp_pgmen_cpu = 1;
            otp->otp_cpumpen_cpu = 1;
            otp->otp_web_cpu = 0;
            otp->otp_web_cpu = 1;
            otp->otp_cpumpen_cpu = 0;
            otp->otp_pgmen_cpu = 0;
            if (otp->otp_dat_cpu == 0)
                break;
        }
        if (otp->otp_dat_cpu & 0x01)
            break;
        data >>= 1;
        addr++;
        index++;
        if (index == 8)
        {
            index = 0;
            data = *data_buf++;
        }
        length--;
    }
    otp->otp_ceb_cpu = 1;
    if (length)
        return OTP_ERROR_WRITE;
    return OTP_OK;
}

enum otp_status_t otp_soft_read(uint32_t addr, uint8_t* data_buf, uint32_t length)
{
    otp->otp_ceb_cpu = 1;
    otp->otp_dle_cpu = 0;
    otp->otp_cle_cpu = 0;
    otp->otp_pgmen_cpu = 0;
    otp->otp_web_cpu = 1;
    otp->otp_readen_cpu = 0;
    otp->otp_rstb_cpu = 1;

    otp->otp_ceb_cpu = 0;
    otp->otp_rstb_cpu = 0;
    otp->otp_rstb_cpu = 1;

    while (length)
    {
        otp->otp_adr_cpu = addr;
        otp->otp_readen_cpu = 1;
        *data_buf++ = otp->otp_dat_cpu;
        otp->otp_readen_cpu = 0;
        addr += 8;
        length--;
    }
    otp->otp_ceb_cpu = 1;
    if (length)
        return OTP_ERROR_WRITE;
    return OTP_OK;
}

uint32_t otp_wrong_address_get(void)
{
    return otp->otp_pro_adr;
}
