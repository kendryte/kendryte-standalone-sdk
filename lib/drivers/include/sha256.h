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
#ifndef _SHA256_H
#define _SHA256_H

#include <stdint.h>
#include "encoding.h"
#include "platform.h"

#define DISABLE_SHA_DMA 0
#define ENABLE_SHA_DMA  1

/**
 * @brief       AES
 *
 */
typedef struct _sha256
{
    uint32_t sha_result[8];
    uint32_t sha_data_in1;
    uint32_t sha_data_in2;
    uint32_t sha_data_num; /*1 unit represents 64 bytes*/
    uint32_t sha_status;
    uint32_t reserved0;
    uint32_t sha_input_ctrl;
} __attribute__((packed, aligned(4))) sha256_t;

#define SHA256_HASH_SIZE 32

/* Hash size in 32-bit words */
#define SHA256_HASH_WORDS 8

struct _SHA256Context
{
    uint64_t totalLength;
    uint32_t hash[SHA256_HASH_WORDS];
    uint32_t bufferLength;
    union
    {
        uint32_t words[16];
        uint8_t bytes[64];
    } buffer;
#ifdef RUNTIME_ENDIAN
    int littleEndian;
#endif /* RUNTIME_ENDIAN */
};

typedef struct _SHA256Context SHA256Context_t;

/**
 * @brief       Sha256 initialize
 *
 * @param[in]   dma_en          Dma enable flag
 * @param[in]   input_size      Input size
 * @param[in]   sc              Sha256 Context point
 *
 * @return      Result
 *     - 0      Success
 *     - Other  Fail
 */
int sha256_init(uint8_t dma_en, uint32_t input_size, SHA256Context_t *sc);

/**
 * @brief       Sha256 update
 *
 * @param[in]   sc          Sha256 Context point
 * @param[in]   data        Input data point
 * @param[in]   len         Input data size
 *
 */
void sha256_update(SHA256Context_t *sc, const void *data, uint32_t len);

/**
 * @brief       Sha256 final
 *
 * @param[in]   sc          Sha256 Context point
 * @param[out]  hash        Sha256 result
 *
 */
void sha256_final(SHA256Context_t *sc, uint8_t hash[SHA256_HASH_SIZE]);

#endif
