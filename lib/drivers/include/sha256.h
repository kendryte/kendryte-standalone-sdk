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
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ENABLE_SHA          1
#define SHA256_BIG_ENDIAN   (0x1 << 16)

#define SHA256_HASH_LEN    32
#define SHA256_HASH_WORDS   8
#define SHA256_BLOCK_LEN   64L

typedef struct _sha256
{
    uint32_t sha_result[8];
    uint32_t sha_data_in1;
    uint32_t sha_data_in2;
    uint32_t sha_data_num;
    uint32_t sha_status;
    uint32_t reserved0;
    uint32_t sha_input_ctrl;
} __attribute__((packed, aligned(4))) sha256_t;

typedef struct _sha256_context
{
    size_t total_length;
    size_t buffer_length;
    union
    {
        uint32_t words[16];
        uint8_t bytes[64];
    } buffer;
} sha256_context_t;

/**
 * @brief       Init SHA256 calculation context
 *
 * @param[in]   context SHA256 context object
 *
 */
void sha256_init(sha256_context_t *context);

/**
 * @brief       Called repeatedly with chunks of the message to be hashed
 *
 * @param[in]   context SHA256 context object
 * @param[in]   data_buf    data chunk to be hashed
 * @param[in]   buf_len    length of data chunk
 *
 */
void sha256_update(sha256_context_t *context, const void *data_buf, size_t buf_len);

/**
 * @brief       Finish SHA256 hash process, output the result.
 *
 * @param[in]   context SHA256 context object
 * @param[out]  output  The buffer where SHA256 hash will be output
 *
 */
void sha256_final(sha256_context_t *context, uint8_t *output);

/**
 * @brief       Simple SHA256 hash once.
 *
 * @param[in]   data      Data will be hashed
 * @param[in]   data_len  Data length
 * @param[out]  output    Output buffer
 *
 */
void sha256_hard_calculate(const uint8_t *data, size_t data_len, uint8_t *output);

#ifdef __cplusplus
}
#endif

#endif

