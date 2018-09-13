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
#ifndef _DRIVER_AES_H
#define _DRIVER_AES_H

#include <stdint.h>
#include "env/encoding.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _aes_mode_ctl
{
    /* set the first bit and second bit 00:ecb; 01:cbc,10：aes_gcm */
    uint32_t cipher_mode : 3;
    /* [4:3]:00:128; 01:192; 10:256;11:reserved*/
    uint32_t kmode : 2;
    uint32_t endian : 6;
    uint32_t stream_mode : 3;
    uint32_t reserved : 18;
} __attribute__((packed, aligned(4))) aes_mode_ctl;

/**
 * @brief       AES
 */
typedef struct _aes
{
    uint32_t aes_key[4];
    /* 0: encrption ; 1: dencrption */
    uint32_t encrypt_sel;
    /**
     * [1:0], Set the first bit and second bit 00:ecb; 01:cbc;
     * 10,11：aes_gcm
    */
    aes_mode_ctl mode_ctl;
    uint32_t aes_iv[4];
    /* aes interrupt enable */
    uint32_t aes_endian;
    /* aes interrupt flag */
    uint32_t aes_finish;
    /* gcm add data begin address */
    uint32_t dma_sel;
    /* gcm add data end address */
    uint32_t gb_aad_end_adr;
    /* gcm plantext/ciphter text data begin address */
    uint32_t gb_pc_ini_adr;
    /* gcm plantext/ciphter text data end address */
    uint32_t gb_pc_end_adr;
    /* gcm plantext/ciphter text data */
    uint32_t aes_text_data;
    /* AAD data */
    uint32_t aes_aad_data;
    /**
     * [1:0],00:check not finish; 01: check fail; 10: check success;11:
     * reversed
     */
    uint32_t tag_chk;
    /* data can input flag 1: data can input; 0 : data cannot input */
    uint32_t data_in_flag;
    /* gcm input tag for compare with the calculate tag */
    uint32_t gcm_in_tag[4];
    /* gcm plantext/ciphter text data */
    uint32_t aes_out_data;
    uint32_t gb_aes_en;
    /* data can output flag 1: data ready 0: data not ready */
    uint32_t data_out_flag;
    /* allow tag input when use GCM */
    uint32_t tag_in_flag;
    uint32_t tag_clear;
    uint32_t gcm_out_tag[4];
    uint32_t aes_key_ext[4];
} __attribute__((packed, aligned(4))) aes_t;

enum aes_cipher_mod
{
    AES_ECB = 0,
    AES_CBC = 1,
    AES_GCM = 2,
};

enum aes_kmode
{
    AES_128 = 0,
    AES_192 = 1,
    AES_256 = 2,
};

enum aes_encrypt_sel
{
    AES_ENCRYPTION = 0,
    AES_DECRYPTION = 1,
};

/**
 * @brief       Aes initialize
 *
 * @param[in]   key_addr        Key address
 * @param[in]   key_length      Key length
 * @param[in]   aes_iv          Aes iv
 * @param[in]   iv_length       Iv length
 * @param[in]   aes_aad         Aes aad
 * @param[in]   cipher_mod      Aes cipher mode
 * @param[in]   encrypt_sel      Aes encrypt select
 * @param[in]   add_size        Aad size
 * @param[in]   data_size       Data size
 *
 * @return      result
 *     - 1      Success
 *     - Other  Fail
 */
int aes_init(uint8_t* key_addr, uint8_t key_length, uint8_t* aes_iv,
    uint8_t iv_length, uint8_t* aes_aad, enum aes_cipher_mod cipher_mod,
    enum aes_encrypt_sel encrypt_sel, uint32_t add_size, uint32_t data_size);

/**
 * @brief       Aes write aad data
 *
 * @param[in]   aad_data        Aes aad data
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int aes_write_aad(uint32_t aad_data);

/**
 * @brief       Aes write text data
 *
 * @param[in]   text_data       Aes aad data
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int aes_write_text(uint32_t text_data);

/**
 * @brief       Aes write tag
 *
 * @param[in]   text_data       Aes tag point
 *
 * @return      result
 *     - 0      Success
 *     - Other  Fail
 */
int aes_write_tag(uint32_t* tag);

/**
 * @brief       Aes get data in flag
 *
 * @return      Data in flag
 */
int aes_get_data_in_flag(void);

/**
 * @brief       Aes get data out flag
 *
 * @return      Data out flag
 */
int aes_get_data_out_flag(void);

/**
 * @brief       Aes get tag in flag
 *
 * @return      Tag in flag
 */
int aes_get_tag_in_flag(void);

/**
 * @brief       Aes read out data
 *
 * @return      Out data
 */
uint32_t aes_read_out_data(void);

/**
 * @brief       Aes check tag
 *
 * @return      Tag check result
 *     - 0      Check not finish
 *     - 1      Check fail
 *     - 2      Check success
 */
int aes_check_tag(void);

/**
 * @brief       Aes get gcm out tag
 *
 * @param[out]   l_tag      gcm out tag
 *
 * @return      Tag check result
 *     - 1      Success
 *     - Other  Fail
 */
int aes_get_tag(uint8_t* l_tag);

/**
 * @brief       Aes clear check tag
 *
 * @return      Tag check result
 *     - 0      Success
 *     - Other  Fail
 */
int aes_clear_chk_tag(void);

/**
 * @brief       Aes process
 *
 * @param[in]   aes_in_data         Aes in data
 * @param[in]   aes_out_data        Aes out data
 * @param[in]   data_size           Aes data size
 * @param[in]   cipher_mod          Aes cipher mode
 *
 * @return      Tag check result
 *     - 1      Success
 *     - Other  Fail
 */
int aes_process(uint8_t* aes_in_data,
    uint8_t* aes_out_data,
    uint32_t data_size,
    enum aes_cipher_mod cipher_mod);

/**
 * @brief       Aes check gcm tag
 *
 * @param[in]   aes_gcm_tag     Aes gcm tag
 *
 * @return      Tag check result
 *     - 1      Success
 *     - Other  Fail
 */
int aes_check_gcm_tag(uint32_t* aes_gcm_tag);

/**
 * @brief      Aes clock initialize
 */
void aes_clkinit();

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_AES_H */
