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

#include <stdio.h>
#include <string.h>
#include "aes.h"
#include "aes_cbc.h"
#include "gcm.h"
#include "sysctl.h"
#include "encoding.h"
#include "dmac.h"

//#define AES_DBUG
#ifdef AES_DBUG
#define AES_DBG(fmt, args...) printf(fmt, ##args)
#else
#define AES_DBG(fmt, args...)
#endif

#define CIPHER_MAX              3
#define AES_TEST_DATA_LEN       (1024 + 1 + 4UL)
#define AES_TEST_PADDING_LEN    ((AES_TEST_DATA_LEN + 15) / 16 *16)

enum _trans_data_mode
{
    AES_CPU = 0,
    AES_DMA = 1,
    AES_TRANS_MAX,
} ;

enum _test_mode
{
    AES_HARD = 0,
    AES_SOFT = 1,
    AES_TEST_MAX,
} ;

typedef enum _check_result
{
    AES_CHECK_PASS = 0,
    AES_CHECK_FAIL = 1,
    AES_CHECK_MAX,
} check_result_t;

char *cipher_name[CIPHER_MAX] =
{
    "aes-ecb-256",
    "aes-cbc-256",
    "aes-gcm-256",
};

uint8_t aes_key[] = {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c, 0x00, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
                    0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c, 0x00, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08};
size_t key_len = AES_256;

uint8_t aes_iv[] = {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88, 0x67, 0x30, 0x83, 0x08};
uint8_t iv_len = 16;
uint8_t iv_gcm_len = 12;

uint8_t aes_aad[] = {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
                    0xab, 0xad, 0xda, 0xd2};
uint8_t aad_len = 20;

uint8_t gcm_soft_tag[16];
uint8_t gcm_hard_tag[16];
uint8_t aes_hard_in_data[AES_TEST_PADDING_LEN];
uint8_t aes_soft_in_data[AES_TEST_PADDING_LEN];
uint8_t aes_hard_out_data[AES_TEST_PADDING_LEN];
uint8_t aes_soft_out_data[AES_TEST_PADDING_LEN];
uint64_t cycle[CIPHER_MAX][AES_TEST_MAX][AES_TRANS_MAX];
uint8_t get_time_flag;

void aes_soft_get_data (uint8_t *input_key,
    size_t key_len,
    uint8_t *iv,
    size_t iv_len,
    uint8_t *aes_aad,
    size_t aad_size,
    aes_cipher_mode_t cipher_mod,
    uint8_t *aes_data,
    size_t data_size)
{
    uint32_t i, temp_size;

    memset(aes_soft_in_data, 0, AES_TEST_PADDING_LEN);
    memcpy(aes_soft_in_data, aes_data, data_size);

    if (cipher_mod == AES_ECB)
    {
        if(get_time_flag)
            cycle[AES_ECB][AES_SOFT][AES_CPU] = read_cycle();
        i = 0;
        if (data_size >= 16)
        {
            for (i = 0; i < (data_size / 16); i++)
                AES_ECB_encrypt(&aes_soft_in_data[i * 16], input_key, &aes_soft_out_data[i * 16], 16);
        }
        temp_size = data_size % 16;
        if (temp_size)
            AES_ECB_encrypt(&aes_soft_in_data[i * 16], input_key, &aes_soft_out_data[i * 16], temp_size);
        if(get_time_flag)
            cycle[AES_ECB][AES_SOFT][AES_CPU] = read_cycle() - cycle[AES_ECB][AES_SOFT][AES_CPU];
    }
    else if (cipher_mod == AES_CBC)
    {
        if(get_time_flag)
            cycle[AES_CBC][AES_SOFT][AES_CPU] = read_cycle();
        AES_CBC_encrypt_buffer(aes_soft_out_data, aes_soft_in_data, data_size, input_key, iv);
        if(get_time_flag)
            cycle[AES_CBC][AES_SOFT][AES_CPU] = read_cycle() - cycle[AES_CBC][AES_SOFT][AES_CPU];
    }
    else if (cipher_mod == AES_GCM)
    {
        if(get_time_flag)
            cycle[AES_GCM][AES_SOFT][AES_CPU] = read_cycle();
        mbedtls_gcm_context ctx;
        mbedtls_cipher_id_t cipher = MBEDTLS_CIPHER_ID_AES;
        mbedtls_gcm_init(&ctx);
        // 128 bits, not bytes!
        mbedtls_gcm_setkey(&ctx, cipher, input_key, key_len*8);
        mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, data_size, iv, iv_len, aes_aad, aad_size, aes_data,
            aes_soft_out_data, 16, gcm_soft_tag);
        mbedtls_gcm_free(&ctx);
        if(get_time_flag)
            cycle[AES_GCM][AES_SOFT][AES_CPU] = read_cycle() - cycle[AES_GCM][AES_SOFT][AES_CPU];
    }
    else
    {
        printf("\n error cipher \n");
    }
}

void aes_cpu_get_data (uint8_t *input_key,
    size_t key_len,
    uint8_t *iv,
    size_t iv_len,
    uint8_t *aes_aad,
    size_t aad_size,
    aes_cipher_mode_t cipher_mod,
    uint8_t *aes_data,
    size_t data_size)
{
    cbc_context_t cbc_context;
    gcm_context_t gcm_context;
    cbc_context.input_key = input_key;
    cbc_context.iv = iv;
    gcm_context.gcm_aad = aes_aad;
    gcm_context.gcm_aad_len = aad_size;
    gcm_context.input_key = input_key;
    gcm_context.iv = iv;

    if (cipher_mod == AES_CBC)
    {
        if(get_time_flag)
            cycle[AES_CBC][AES_HARD][AES_CPU] = read_cycle();
        aes_cbc256_hard_encrypt(&cbc_context, aes_data, data_size, aes_hard_out_data);
        if(get_time_flag)
            cycle[AES_CBC][AES_HARD][AES_CPU] = read_cycle() - cycle[AES_CBC][AES_HARD][AES_CPU];
    }
    else if (cipher_mod == AES_ECB)
    {
        if(get_time_flag)
            cycle[AES_ECB][AES_HARD][AES_CPU] = read_cycle();
        aes_ecb256_hard_encrypt(input_key, aes_data, data_size, aes_hard_out_data);
        if(get_time_flag)
            cycle[AES_ECB][AES_HARD][AES_CPU] = read_cycle() - cycle[AES_ECB][AES_HARD][AES_CPU];
    }
    else
    {
        if(get_time_flag)
            cycle[AES_GCM][AES_HARD][AES_CPU] = read_cycle();
        aes_gcm256_hard_encrypt(&gcm_context, aes_data, data_size, aes_hard_out_data, gcm_hard_tag);
        if(get_time_flag)
            cycle[AES_GCM][AES_HARD][AES_CPU] = read_cycle() - cycle[AES_GCM][AES_HARD][AES_CPU];
    }
}

void aes_dma_get_data (uint8_t *input_key,
    size_t key_len,
    uint8_t *iv,
    size_t iv_len,
    uint8_t *aes_aad,
    size_t aad_size,
    aes_cipher_mode_t cipher_mod,
    uint8_t *aes_data,
    size_t data_size)
{
    cbc_context_t cbc_context;
    gcm_context_t gcm_context;
    cbc_context.input_key = input_key;
    cbc_context.iv = iv;
    gcm_context.gcm_aad = aes_aad;
    gcm_context.gcm_aad_len = aad_size;
    gcm_context.input_key = input_key;
    gcm_context.iv = iv;

    if (cipher_mod == AES_CBC)
    {
        if(get_time_flag)
            cycle[AES_CBC][AES_HARD][AES_DMA] = read_cycle();
        aes_cbc256_hard_encrypt_dma(DMAC_CHANNEL1, &cbc_context, aes_data, data_size, aes_hard_out_data);
        if(get_time_flag)
            cycle[AES_CBC][AES_HARD][AES_DMA] = read_cycle() - cycle[AES_CBC][AES_HARD][AES_DMA];
    }
    else if (cipher_mod == AES_ECB)
    {
        if(get_time_flag)
            cycle[AES_ECB][AES_HARD][AES_DMA] = read_cycle();
        aes_ecb256_hard_encrypt_dma(DMAC_CHANNEL1, input_key, aes_data, data_size, aes_hard_out_data);
        if(get_time_flag)
            cycle[AES_ECB][AES_HARD][AES_DMA] = read_cycle() - cycle[AES_ECB][AES_HARD][AES_DMA];
    }
    else
    {
        if(get_time_flag)
            cycle[AES_GCM][AES_HARD][AES_DMA] = read_cycle();
        aes_gcm256_hard_encrypt_dma(DMAC_CHANNEL1, &gcm_context, aes_data, data_size, aes_hard_out_data, gcm_hard_tag);
        if(get_time_flag)
            cycle[AES_GCM][AES_HARD][AES_DMA] = read_cycle() - cycle[AES_GCM][AES_HARD][AES_DMA];
    }
}

check_result_t aes_encrypt_compare_hard_soft(aes_cipher_mode_t cipher_mod, size_t data_size)
{
    size_t padding_len = data_size;
    uint32_t i;
    uint8_t check_tag = 0;

    if ((cipher_mod == AES_CBC) || (cipher_mod == AES_ECB))
    {
        padding_len = (data_size + 15) / 16 * 16;
    }
    for (i = 0; i < padding_len; i++)
    {
        if (aes_hard_out_data[i] != aes_soft_out_data[i])
        {
            AES_DBG("aes_hard_out_data[%d]:0x%02x  aes_soft_out_data:0x%02x\n", i, aes_hard_out_data[i], aes_soft_out_data[i]);
            check_tag = 1;
        }
    }
    if (check_tag == 1)
    {
        AES_DBG("\nciphertext error\n");
    }
    else
    {
        AES_DBG("ciphertext pass\n");
    }
    if (cipher_mod == AES_GCM)
    {
        check_tag = 0;
        for (i = 0; i < 16; i++)
        {
            if (gcm_soft_tag[i] != gcm_hard_tag[i])
            {
                AES_DBG("error tag : gcm_soft_tag:0x%02x    gcm_hard_tag:0x%02x\n", gcm_soft_tag[i], gcm_hard_tag[i]);
                check_tag = 1;
            }
        }
        if (check_tag == 1)
        {
            AES_DBG("tag error\n");
        }
        else
        {
            AES_DBG("tag OK\n");
        }
    }
    if(check_tag)
        return AES_CHECK_FAIL;
    else
        return AES_CHECK_PASS;
}

check_result_t aes_check_decrypt(uint8_t *input_key,
    size_t key_len,
    uint8_t *iv,
    size_t iv_len,
    uint8_t *aes_aad,
    size_t aad_size,
    aes_cipher_mode_t cipher_mod,
    uint8_t *aes_data,
    size_t data_size)
{
    uint32_t i;
    uint8_t check_tag = 0;
    size_t padding_len = data_size;
    if ((cipher_mod == AES_CBC) || (cipher_mod == AES_ECB))
    {
        padding_len = ((data_size + 15) / 16) * 16;
    }
    cbc_context_t cbc_context;
    gcm_context_t gcm_context;
    cbc_context.input_key = input_key;
    cbc_context.iv = iv;
    gcm_context.gcm_aad = aes_aad;
    gcm_context.gcm_aad_len = aad_size;
    gcm_context.input_key = input_key;
    gcm_context.iv = iv;

    if (cipher_mod == AES_CBC)
        aes_cbc256_hard_decrypt_dma(DMAC_CHANNEL1, &cbc_context, aes_hard_out_data, padding_len, aes_soft_out_data);
    else if (cipher_mod == AES_ECB)
        aes_ecb256_hard_decrypt_dma(DMAC_CHANNEL1, input_key, aes_hard_out_data, padding_len, aes_soft_out_data);
    else
        aes_gcm256_hard_decrypt_dma(DMAC_CHANNEL1, &gcm_context, aes_hard_out_data, padding_len, aes_soft_out_data, gcm_hard_tag);
    check_tag = 0;
    for (i = 0; i < data_size; i++)
    {
        if (aes_data[i] != aes_soft_out_data[i])
        {
            AES_DBG("aes_data[%d]:0x%02x  aes_soft_out_data:0x%02x\n", i, aes_data[i], aes_soft_out_data[i]);
            check_tag = 1;
        }
    }
    if (check_tag == 1)
    {
        AES_DBG("\nplaintext error\n");
        return AES_CHECK_FAIL;
    }
    else
    {
        AES_DBG("plaintext OK\n");
        return AES_CHECK_PASS;
    }
}

check_result_t aes_check (uint8_t *input_key,
    size_t key_len,
    uint8_t *iv,
    size_t iv_len,
    uint8_t *aes_aad,
    size_t aad_size,
    aes_cipher_mode_t cipher_mod,
    uint8_t *aes_data,
    size_t data_size)
{
    check_result_t ret = AES_CHECK_PASS;

    memset(aes_soft_in_data, 0, AES_TEST_PADDING_LEN);
    memset(aes_soft_out_data, 0, AES_TEST_PADDING_LEN);
    memset(aes_hard_out_data, 0, AES_TEST_PADDING_LEN);

    aes_dma_get_data(input_key, key_len, iv, iv_len, aes_aad, aad_size, cipher_mod, aes_data, data_size);
    aes_soft_get_data(input_key, key_len, iv, iv_len, aes_aad, aad_size, cipher_mod, aes_data, data_size);
    ret |= aes_encrypt_compare_hard_soft(cipher_mod, data_size);

    memset(aes_hard_out_data, 0, AES_TEST_PADDING_LEN);
    aes_cpu_get_data(input_key, key_len, iv, iv_len, aes_aad, aad_size, cipher_mod, aes_data, data_size);
    ret |= aes_encrypt_compare_hard_soft(cipher_mod, data_size);
    memset(aes_soft_out_data, 0, AES_TEST_PADDING_LEN);
    ret |= aes_check_decrypt(input_key, key_len, iv, iv_len, aes_aad, aad_size, cipher_mod, aes_data, data_size);
    return ret;
}

check_result_t aes_check_all_byte(aes_cipher_mode_t cipher)
{
    uint32_t check_tag = 0;
    uint32_t index = 0;
    size_t data_len = 0;
    memset(aes_hard_in_data, 0, AES_TEST_PADDING_LEN);
    if (cipher == AES_GCM)
        iv_len = iv_gcm_len;
    for (index = 0; index < (AES_TEST_DATA_LEN < 256 ? AES_TEST_DATA_LEN : 256); index++)
    {
        aes_hard_in_data[index] = index;
        data_len++;

        AES_DBG("[%s] test num: %ld \n", cipher_name[cipher], data_len);
        if (aes_check(aes_key, key_len, aes_iv, iv_len, aes_aad, aad_len, cipher, aes_hard_in_data, data_len)
            == AES_CHECK_FAIL)
            check_tag = 1;
    }

    memset(aes_hard_in_data, 0, AES_TEST_PADDING_LEN);
    get_time_flag = 1;
    data_len = AES_TEST_DATA_LEN;
    AES_DBG("[%s] test num: %ld \n", cipher_name[cipher], data_len);
    for (index = 0; index < data_len; index++)
        aes_hard_in_data[index] = index % 256;
    if (aes_check(aes_key, key_len, aes_iv, iv_len, aes_aad, aad_len, cipher, aes_hard_in_data, data_len)
        == AES_CHECK_FAIL)
        check_tag = 1;
    get_time_flag = 0;
    if(check_tag)
        return AES_CHECK_FAIL;
    else
        return AES_CHECK_PASS;
}

check_result_t aes_check_all_key(aes_cipher_mode_t cipher)
{
    size_t data_len = 0;
    uint32_t index = 0;
    uint32_t i = 0;
    uint32_t check_tag = 0;

    memset(aes_hard_in_data, 0, AES_TEST_PADDING_LEN);
    if (cipher == AES_GCM)
        iv_len = iv_gcm_len;
    data_len = AES_TEST_DATA_LEN;
    for (index = 0; index < data_len; index++)
        aes_hard_in_data[index] = index;
    for (i = 0; i < (256 / key_len); i++)
    {
        for (index = i * key_len; index < (i * key_len) + key_len; index++)
            aes_key[index - (i * key_len)] = index;
        if (aes_check(aes_key, key_len, aes_iv, iv_len, aes_aad, aad_len, cipher, aes_hard_in_data, data_len)
            == AES_CHECK_FAIL)
            check_tag = 1;
    }
    if(check_tag)
        return AES_CHECK_FAIL;
    else
        return AES_CHECK_PASS;
}

check_result_t aes_check_all_iv(aes_cipher_mode_t cipher)
{
    size_t data_len = 0;
    uint32_t index = 0;
    uint32_t i = 0;
    uint8_t check_tag = 0;

    memset(aes_hard_in_data, 0, AES_TEST_PADDING_LEN);
    if (cipher == AES_GCM)
        iv_len = iv_gcm_len;
    data_len = AES_TEST_DATA_LEN;
    for (index = 0; index < data_len; index++)
        aes_hard_in_data[index] = index;
    for (i = 0; i < (256 / iv_len); i++)
    {
        for (index = i * iv_len; index < (i * iv_len) + iv_len; index++)
            aes_iv[index - (i * iv_len)] = index;
        if (aes_check(aes_key, key_len, aes_iv, iv_len, aes_aad, aad_len, cipher, aes_hard_in_data, data_len)
            == AES_CHECK_FAIL)
            check_tag = 1;
    }
    if(check_tag)
        return AES_CHECK_FAIL;
    else
        return AES_CHECK_PASS;
}

int main(void)
{
    aes_cipher_mode_t cipher;
    printf("begin test %d\n", get_time_flag);

    for (cipher = AES_ECB; cipher < AES_CIPHER_MAX; cipher++)
    {
        printf("[%s] test all byte ... \n", cipher_name[cipher]);
        if (AES_CHECK_FAIL == aes_check_all_byte(cipher))
        {
            printf("aes %s check_all_byte fail\n", cipher_name[cipher]);
            return -1;
        }
        printf("[%s] test all key ... \n", cipher_name[cipher]);
        if (AES_CHECK_FAIL == aes_check_all_key(cipher))
        {
            printf("aes %s check_all_key fail\n", cipher_name[cipher]);
            return -1;
        }
        printf("[%s] test all iv ... \n", cipher_name[cipher]);
        if (AES_CHECK_FAIL == aes_check_all_iv(cipher))
        {
            printf("aes %s check_all_iv fail\n", cipher_name[cipher]);
            return -1;
        }
        printf("[%s] [%ld bytes] cpu time = %ld us, dma time = %ld us, soft time = %ld us\n", cipher_name[cipher],
                AES_TEST_DATA_LEN,
                cycle[cipher][AES_HARD][AES_CPU]/(sysctl_clock_get_freq(SYSCTL_CLOCK_CPU)/1000000),
                cycle[cipher][AES_HARD][AES_DMA]/(sysctl_clock_get_freq(SYSCTL_CLOCK_CPU)/1000000),
                cycle[cipher][AES_SOFT][AES_CPU]/(sysctl_clock_get_freq(SYSCTL_CLOCK_CPU)/1000000));
    }
    printf("aes-256 test pass\n");
    while (1)
        ;
    return 0;
}

