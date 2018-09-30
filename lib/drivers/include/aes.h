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
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _aes_cipher_mode
{
    AES_ECB = 0,
    AES_CBC = 1,
    AES_GCM = 2,
    CIPHER_MAX,
} aes_cipher_mode_t;

typedef enum _aes_kmode
{
    AES_128 = 16,
    AES_192 = 24,
    AES_256 = 32,
} aes_kmode_t;

typedef enum _aes_encrypt_sel
{
    AES_HARD_ENCRYPTION = 0,
    AES_HARD_DECRYPTION = 1,
} aes_encrypt_sel_t;

typedef struct _aes_mode_ctl
{
    /* set the first bit and second bit 00:ecb; 01:cbc,10：aes_gcm */
    uint32_t cipher_mode : 3;
    /* [4:3]:00:128; 01:192; 10:256;11:reserved*/
    uint32_t kmode : 2;
    uint32_t endian : 6;
    uint32_t stream_mode : 3;
    uint32_t reserved : 18;
} __attribute__((packed, aligned(4))) aes_mode_ctl_t;

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
    aes_mode_ctl_t mode_ctl;
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

typedef struct _aes_param
{
    uint8_t *input_data;
    size_t input_data_len;
    uint8_t *input_key;
    size_t input_key_len;
    uint8_t *iv;
    uint8_t iv_len;
    uint8_t* gcm_add;
    size_t gcm_add_len;
    aes_cipher_mode_t cipher_mode;
    uint8_t *output_data;
    uint8_t *gcm_tag;
} aes_param_t;

void aes_hard_decrypt(aes_param_t *param);
void aes_hard_encrypt(aes_param_t *param);

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_AES_H */
