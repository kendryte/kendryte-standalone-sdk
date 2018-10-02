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
#include <string.h>
#include "sysctl.h"
#include "sha256.h"
#include "utils.h"

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define BYTESWAP(x) ((ROTR((x), 8) & 0xff00ff00L) | (ROTL((x), 8) & 0x00ff00ffL))
#define BYTESWAP64(x) byteswap64(x)

volatile sha256_t* const sha256 = (volatile sha256_t*)SHA256_BASE_ADDR;
static const uint8_t padding[64] =
{
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00
};

static inline uint64_t byteswap64(uint64_t x)
{
    uint32_t a = (uint32_t)(x >> 32);
    uint32_t b = (uint32_t)x;
    return ((uint64_t)BYTESWAP(b) << 32) | (uint64_t)BYTESWAP(a);
}

void sha256_init(sha256_context_t *context, size_t buf_len)
{
    sysctl_clock_enable(SYSCTL_CLOCK_SHA);
    sysctl_reset(SYSCTL_RESET_SHA);

    sha256->sha_data_num = (uint32_t)((buf_len + SHA256_BLOCK_LEN + 8) / SHA256_BLOCK_LEN);

    sha256->sha_input_ctrl &= (~0x1);
    sha256->sha_status |= SHA256_BIG_ENDIAN;
    sha256->sha_status |= ENABLE_SHA;
    context->total_length = 0LL;
    context->buffer_length = 0L;
}

void sha256_update(sha256_context_t *context, const void *data_buf, size_t buf_len)
{
    const uint8_t* data = data_buf;
    size_t buffer_bytes_left;
    size_t bytes_to_copy;
    uint32_t i;

    while (buf_len)
    {
        buffer_bytes_left = SHA256_BLOCK_LEN - context->buffer_length;
        bytes_to_copy = buffer_bytes_left;
        if (bytes_to_copy > buf_len)
            bytes_to_copy = buf_len;
        memcpy(&context->buffer.bytes[context->buffer_length], data, bytes_to_copy);
        context->total_length += bytes_to_copy * 8;
        context->buffer_length += bytes_to_copy;
        data += bytes_to_copy;
        buf_len -= bytes_to_copy;
        if (context->buffer_length == SHA256_BLOCK_LEN)
        {
            for (i = 0; i < 16; i++)
            {
                while (sha256->sha_input_ctrl & (1 << 8))
                    ;
                sha256->sha_data_in1 = context->buffer.words[i];
            }
            context->buffer_length = 0L;
        }
    }
}

void sha256_final(sha256_context_t *context, uint8_t *output)
{
    size_t bytes_to_pad;
    size_t length_pad;
    uint32_t i;

    bytes_to_pad = 120 - context->buffer_length;
    if (bytes_to_pad > 64)
        bytes_to_pad -= 64;
    length_pad = BYTESWAP64(context->total_length);
    sha256_update(context, padding, bytes_to_pad);
    sha256_update(context, &length_pad, 8);
    while (!(sha256->sha_status & 0x01))
        ;
    if (output)
    {
        for (i = 0; i < SHA256_HASH_WORDS; i++)
        {
            *((uint32_t*)output) = sha256->sha_result[SHA256_HASH_WORDS - i - 1];
            output += 4;
        }
    }
}

void sha256_hard_calculate(const uint8_t *data, size_t data_len, uint8_t *output)
{
    sha256_context_t sha;
    sha256_init(&sha, data_len);
    sha256_update(&sha, data, data_len);
    sha256_final(&sha, output);
}

