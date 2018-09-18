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
#include <stdint.h>
#include "encoding.h"
#include "aes.h"
#include "sysctl.h"

volatile aes_t* const aes = (volatile aes_t*)AES_BASE_ADDR;

void aes_clkinit()
{
    sysctl_clock_enable(SYSCTL_CLOCK_AES);
    sysctl_reset(SYSCTL_RESET_AES);
}

int aes_init(uint8_t* key_addr, uint8_t key_length, uint8_t* aes_iv,
    uint8_t iv_length, uint8_t* aes_aad, enum aes_cipher_mod cipher_mod,
    enum aes_encrypt_sel encrypt_sel, uint32_t add_size, uint32_t data_size)
{
    int i, remainder, num, cnt;
    uint32_t u32data;
    uint8_t u8data[4] = {0};

    if ((cipher_mod == AES_ECB) || (cipher_mod == AES_CBC))
        data_size = ((data_size + 15) / 16) * 16;
    aes->aes_endian |= 1;

    /* write key Low byte alignment*/
    num = key_length / 4;
    for (i = 0; i < num; i++)
        aes->aes_key[i] = *((uint32_t*)(&key_addr[key_length - (4 * i) - 4]));
    remainder = key_length % 4;
    if (remainder)
    {
        switch (remainder)
        {
        case 1:
            u8data[0] = key_addr[0];
            break;
        case 2:
            u8data[0] = key_addr[0];
            u8data[1] = key_addr[1];
            break;
        case 3:
            u8data[0] = key_addr[0];
            u8data[1] = key_addr[1];
            u8data[2] = key_addr[2];
            break;
        default:
            break;
        }
        aes->aes_key[num] = *((uint32_t*)(&u8data[0]));
    }

    /*write iv Low byte alignment*/
    num = iv_length / 4;
    for (i = 0; i < num; i++)
        aes->aes_iv[i] = *((uint32_t*)(&aes_iv[iv_length - (4 * i) - 4]));
    remainder = iv_length % 4;
    if (remainder)
    {
        switch (remainder)
        {
        case 1:
            u8data[0] = aes_iv[0];
            break;
        case 2:
            u8data[0] = aes_iv[0];
            u8data[1] = aes_iv[1];
            break;
        case 3:
            u8data[0] = aes_iv[0];
            u8data[1] = aes_iv[1];
            u8data[2] = aes_iv[2];
            break;
        default:
            break;
        }
        aes->aes_iv[num] = *((uint32_t*)(&u8data[0]));
    }

    aes->mode_ctl.cipher_mode = cipher_mod;

    /*
     * [1:0],set the first bit and second bit 00:ecb; 01:cbc;
     * 10,11：aes_gcm
     */
    aes->encrypt_sel = encrypt_sel;
    aes->gb_aad_end_adr = add_size - 1;
    aes->gb_pc_end_adr = data_size - 1;
    aes->gb_aes_en |= 1;

    /* write aad */
    if (cipher_mod == AES_GCM)
    {
        num = add_size / 4;
        for (i = 0; i < num; i++)
        {
            u32data = *((uint32_t*)(&aes_aad[i * 4]));
            while (!aes_get_data_in_flag())
                ;
            aes_write_aad(u32data);
        }
        cnt = 4 * num;
        remainder = add_size % 4;
        if (remainder)
        {
            switch (remainder)
            {
            case 1:
                u8data[0] = aes_aad[cnt];
                break;
            case 2:
                u8data[0] = aes_aad[cnt];
                u8data[1] = aes_aad[cnt + 1];
                break;
            case 3:
                u8data[0] = aes_aad[cnt];
                u8data[1] = aes_aad[cnt + 1];
                u8data[2] = aes_aad[cnt + 2];
                break;
            default:
                return 0;
            }
            u32data = *((uint32_t*)(&u8data[0]));
            while (!aes_get_data_in_flag())
                ;
            aes_write_aad(u32data);
        }
    }

    return 1;
}

int aes_write_aad(uint32_t aad_data)
{
    aes->aes_aad_data = aad_data;
    return 0;
}

int aes_write_text(uint32_t text_data)
{
    aes->aes_text_data = text_data;
    return 0;
}

int aes_write_tag(uint32_t* tag)
{
    aes->gcm_in_tag[0] = tag[3];
    aes->gcm_in_tag[1] = tag[2];
    aes->gcm_in_tag[2] = tag[1];
    aes->gcm_in_tag[3] = tag[0];
    return 0;
}

int aes_get_data_in_flag(void)
{
    /* data can in flag 1: data ready 0: data not ready */
    return aes->data_in_flag;
}

int aes_get_data_out_flag(void)
{
    /* data can output flag 1: data ready 0: data not ready */
    return aes->data_out_flag;
}

int aes_get_tag_in_flag(void)
{
    /* data can output flag 1: data ready 0: data not ready */
    return aes->tag_in_flag;
}

uint32_t aes_read_out_data(void)
{
    return aes->aes_out_data;
}

int aes_check_tag(void)
{
    return aes->tag_chk;
}

int aes_get_tag(uint8_t* l_tag)
{
    uint32_t u32tag;
    uint8_t i = 0;

    u32tag = aes->gcm_out_tag[3];
    l_tag[i++] = (uint8_t)((u32tag >> 24) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag >> 16) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag >> 8) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag)&0xff);

    u32tag = aes->gcm_out_tag[2];
    l_tag[i++] = (uint8_t)((u32tag >> 24) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag >> 16) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag >> 8) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag)&0xff);

    u32tag = aes->gcm_out_tag[1];
    l_tag[i++] = (uint8_t)((u32tag >> 24) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag >> 16) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag >> 8) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag)&0xff);

    u32tag = aes->gcm_out_tag[0];
    l_tag[i++] = (uint8_t)((u32tag >> 24) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag >> 16) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag >> 8) & 0xff);
    l_tag[i++] = (uint8_t)((u32tag)&0xff);
    return 1;
}

int aes_clear_chk_tag(void)
{
    aes->tag_clear = 0;
    return 0;
}

int aes_process_less_80_bytes(uint8_t* aes_in_data,
    uint8_t* aes_out_data,
    uint32_t data_size,
    enum aes_cipher_mod cipher_mod)
{
    int padding_size;
    int num, i, remainder, cnt;
    uint32_t u32data;
    uint8_t u8data[4] = {0};
    /* Fill 128 bits    (16byte) */
    padding_size = ((data_size + 15) / 16) * 16;

    /* write text */
    num = data_size / 4;
    for (i = 0; i < num; i++)
    {
        u32data = *((uint32_t*)(&aes_in_data[i * 4]));
        while (!aes_get_data_in_flag())
            ;
        aes_write_text(u32data);
    }
    cnt = 4 * num;
    remainder = data_size % 4;
    if (remainder)
    {
        switch (remainder)
        {
        case 1:
            u8data[0] = aes_in_data[cnt];
            break;
        case 2:
            u8data[0] = aes_in_data[cnt];
            u8data[1] = aes_in_data[cnt + 1];
            break;
        case 3:
            u8data[0] = aes_in_data[cnt];
            u8data[1] = aes_in_data[cnt + 1];
            u8data[2] = aes_in_data[cnt + 2];
            break;
        default:
            return 0;
        }
        u32data = *((uint32_t*)(&u8data[0]));
        while (!aes_get_data_in_flag())
            ;
        aes_write_text(u32data);
    }
    if ((cipher_mod == AES_ECB) || (cipher_mod == AES_CBC))
    {
        /* use 0 to Fill 128 bits */
        num = (padding_size - data_size) / 4;
        for (i = 0; i < num; i++)
        {
            while (!aes_get_data_in_flag())
                ;
            aes_write_text(0);
        }
        /* get data */
        num = padding_size / 4;
    }
    /* get data */
    for (i = 0; i < num; i++)
    {
        while (!aes_get_data_out_flag())
            ;
        *((uint32_t*)(&aes_out_data[i * 4])) = aes_read_out_data();
    }
    if ((cipher_mod == AES_GCM) && (remainder))
    {
        while (!aes_get_data_out_flag())
            ;

        *((uint32_t*)(&u8data[0])) = aes_read_out_data();
        switch (remainder)
        {
        case 1:
            aes_out_data[num * 4] = u8data[0];
            break;
        case 2:
            aes_out_data[num * 4] = u8data[0];
            aes_out_data[(i * 4) + 1] = u8data[1];
            break;
        case 3:
            aes_out_data[num * 4] = u8data[0];
            aes_out_data[(i * 4) + 1] = u8data[1];
            aes_out_data[(i * 4) + 2] = u8data[2];
            break;
        default:
            return 0;
        }
    }
    return 1;
}

int aes_process(uint8_t* aes_in_data,
    uint8_t* aes_out_data,
    uint32_t data_size,
    enum aes_cipher_mod cipher_mod)
{
    uint32_t i, temp_size;

    i = 0;
    if (data_size >= 80)
    {
        for (i = 0; i < (data_size / 80); i++)
            aes_process_less_80_bytes(&aes_in_data[i * 80], &aes_out_data[i * 80], 80, cipher_mod);
    }
    temp_size = data_size % 80;
    if (temp_size)
        aes_process_less_80_bytes(&aes_in_data[i * 80], &aes_out_data[i * 80], temp_size, cipher_mod);
    return 1;
}

int aes_check_gcm_tag(uint32_t* aes_gcm_tag)
{
    /* check tag */
    while (!aes_get_tag_in_flag())
        ;
    aes_write_tag(aes_gcm_tag);
    while (!aes_check_tag())
        ;
    if (aes_check_tag() == 2)
    {
        aes_clear_chk_tag();
        return 1;
    }
    else
    {
        aes_clear_chk_tag();
        return 0;
    }
}
