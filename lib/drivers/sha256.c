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
#include "sha256.h"
#include "syscalls.h"
#include "sysctl.h"

volatile sha256_t* const sha256 = (volatile sha256_t*)SHA256_BASE_ADDR;

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define _BYTESWAP(x) ((ROTR((x), 8) & 0xff00ff00L) | (ROTL((x), 8) & 0x00ff00ffL))
#define _BYTESWAP64(x) __byteswap64(x)

static inline uint64_t __byteswap64(uint64_t x)
{
    uint32_t a = x >> 32;
    uint32_t b = (uint32_t)x;

    return ((uint64_t)_BYTESWAP(b) << 32) | (uint64_t)_BYTESWAP(a);
}
static const uint8_t padding[64] =
{
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};

int sha256_init(uint8_t dma_en, uint32_t input_size, SHA256Context_t* sc)
{
    sysctl_clock_tnable(SYSCTL_CLOCK_SHA);
    sysctl_reset(SYSCTL_RESET_SHA);
    input_size = (input_size + 64) / 64;
    if (dma_en)
        sha256->sha_input_ctrl |= 1;
    else
        sha256->sha_input_ctrl &= ~0x01;

    sha256->sha_data_num = input_size;

    sha256->sha_status |= 1 << 16; /*0 for little endian, 1 for big endian*/
    sha256->sha_status |= 1; /*enable sha256*/

    sc->totalLength = 0LL;
    sc->hash[0] = 0x6a09e667L;
    sc->hash[1] = 0xbb67ae85L;
    sc->hash[2] = 0x3c6ef372L;
    sc->hash[3] = 0xa54ff53aL;
    sc->hash[4] = 0x510e527fL;
    sc->hash[5] = 0x9b05688cL;
    sc->hash[6] = 0x1f83d9abL;
    sc->hash[7] = 0x5be0cd19L;
    sc->bufferLength = 0L;
    return 1;
}

void sha256_update(SHA256Context_t* sc, const void* vdata, uint32_t len)
{
    const uint8_t* data = vdata;
    uint32_t bufferBytesLeft;
    uint32_t bytesToCopy;
    uint32_t i;

    while (len)
    {
        bufferBytesLeft = 64L - sc->bufferLength;

        bytesToCopy = bufferBytesLeft;
        if (bytesToCopy > len)
            bytesToCopy = len;

        memcpy(&sc->buffer.bytes[sc->bufferLength], data, bytesToCopy);

        sc->totalLength += bytesToCopy * 8L;

        sc->bufferLength += bytesToCopy;
        data += bytesToCopy;
        len -= bytesToCopy;

        if (sc->bufferLength == 64L)
        {
            for (i = 0; i < 16; i++)
            {
                while (sha256->sha_input_ctrl & (1 << 8))
                    ;
                sha256->sha_data_in1 = sc->buffer.words[i];
            }
            sc->bufferLength = 0L;
        }
    }
}

void sha256_final(SHA256Context_t* sc, uint8_t hash[SHA256_HASH_SIZE])
{
    uint32_t bytesToPad;
    uint64_t lengthPad;
    int i;

    bytesToPad = 120L - sc->bufferLength;
    if (bytesToPad > 64L)
        bytesToPad -= 64L;
    lengthPad = _BYTESWAP64(sc->totalLength);
    sha256_update(sc, padding, bytesToPad);
    sha256_update(sc, &lengthPad, 8L);

    while (!(sha256->sha_status & 0x01))
        ;

    if (hash)
    {
        for (i = 0; i < SHA256_HASH_WORDS; i++)
        {
            *((uint32_t*)hash) = sha256->sha_result[SHA256_HASH_WORDS - i - 1];
            hash += 4;
        }
    }
}
