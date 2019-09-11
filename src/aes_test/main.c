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
#include "sysctl.h"
#include "dmac.h"
#include "utils.h"

uint8_t aes_key[] = {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c, 0x00, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08};
size_t key_len = AES_128;

uint8_t aes_iv[] = {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88, 0x67, 0x30, 0x83, 0x08};
uint8_t iv_len = 16;

uint8_t aes_data[] = {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c, 0x00, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08};
size_t data_len = 15;

uint8_t aes_out_data[16];
uint8_t aes_decrypt_data[16];

int main(void)
{
	uint8_t *aes_out_data_nochche = (uint8_t *)IO_CACHE_EXCHANGE(aes_out_data);
	uint8_t *aes_decrypt_data_nochche = (uint8_t *)IO_CACHE_EXCHANGE(aes_decrypt_data);
	printf("hello AES \n");
    cbc_context_t cbc_context;

    cbc_context.input_key = aes_key;
    cbc_context.iv = aes_iv;
    aes_cbc128_hard_encrypt_dma(DMAC_CHANNEL0, &cbc_context, aes_data, data_len, aes_out_data_nochche);
    size_t padding_len = ((data_len + 15) / 16) * 16;
    aes_cbc128_hard_decrypt_dma(DMAC_CHANNEL0, &cbc_context, aes_out_data_nochche, padding_len, aes_decrypt_data_nochche);
    int i = 0;
    for(i=0; i<data_len; i++)
    {
        if(aes_decrypt_data_nochche[i] != aes_data[i])
        {
            printf("aes_decrypt_data[%d] = %x aes_data[%d] = %x\n", i, aes_decrypt_data_nochche[i], i, aes_data[i]);
            return 1;
        }
    }
    printf("test pass\n");
    while (1)
        ;
    return 0;
}

