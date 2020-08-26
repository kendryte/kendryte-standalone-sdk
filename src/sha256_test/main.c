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
#include <stdlib.h>
#include <string.h>
#include <encoding.h>
#include "sha256.h"
#include "sleep.h"
#include "sysctl.h"

uint8_t hash[SHA256_HASH_LEN];
uint8_t compare1[] = {0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
                   0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};

uint8_t compare2[] = {0x58, 0xbe, 0xb6, 0xbb, 0x9b, 0x80, 0xb2, 0x12, 0xc3, 0xdb, 0xc1, 0xc1, 0x02, 0x0c, 0x69, 0x6f,
                   0xbf, 0xa3, 0xaa, 0xd8, 0xe8, 0xa4, 0xef, 0x4d, 0x38, 0x5e, 0x9b, 0x07, 0x32, 0xfc, 0x5d, 0x98};

uint8_t compare3[] = {0x6e, 0x65, 0xda, 0xd1, 0x7a, 0xa2, 0x3e, 0x72, 0x79, 0x8d, 0x50, 0x33, 0xa1, 0xae, 0xe5, 0x9e,
                   0xe3, 0x35, 0x2d, 0x3c, 0x49, 0x6c, 0x18, 0xfb, 0x71, 0xe3, 0xa5, 0x37, 0x22, 0x11, 0xfc, 0x6c};

uint8_t compare4[] = {0xcd, 0xc7, 0x6e, 0x5c, 0x99, 0x14, 0xfb, 0x92, 0x81, 0xa1, 0xc7, 0xe2, 0x84, 0xd7, 0x3e, 0x67,
                    0xf1, 0x80, 0x9a, 0x48, 0xa4, 0x97, 0x20, 0x0e, 0x04, 0x6d, 0x39, 0xcc, 0xc7, 0x11, 0x2c, 0xd0};
uint8_t data_buf[1000*1000];

int main(void)
{
    uint64_t cycle;
    uint8_t total_check_tag = 0;
    
    uint32_t i;
    printf("\n");
    cycle = read_cycle();
    sha256_hard_calculate((uint8_t *)"abc", 3, hash);
    for (i = 0; i < SHA256_HASH_LEN;)
    {
        if (hash[i] != compare1[i])
            total_check_tag = 1;
        printf("%02x", hash[i++]);
        if (!(i % 4))
            printf(" ");
    }
    printf("\n");

    sha256_hard_calculate((uint8_t *)"abcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghij", 60, hash);
    for (i = 0; i < SHA256_HASH_LEN;)
    {
        if (hash[i] != compare2[i])
            total_check_tag = 1;
        printf("%02x", hash[i++]);
        if (!(i % 4))
            printf(" ");
    }
    printf("\n");
    
    sha256_hard_calculate((uint8_t *)"abcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefgha", 65, hash);
    for (i = 0; i < SHA256_HASH_LEN;)
    {
        if (hash[i] != compare3[i])
            total_check_tag = 1;
        printf("%02x", hash[i++]);
        if (!(i % 4))
            printf(" ");
    }
    printf("\n");
    
    memset(data_buf, 'a', sizeof(data_buf));
    sha256_hard_calculate(data_buf, sizeof(data_buf), hash);
    for (i = 0; i < SHA256_HASH_LEN;)
    {
        if (hash[i] != compare4[i])
            total_check_tag = 1;
        printf("%02x", hash[i++]);
        if (!(i % 4))
            printf(" ");
    }
    printf("\n");

    sha256_context_t context;
    sha256_init(&context, sizeof(data_buf));
    sha256_update(&context, data_buf, 1111);
    sha256_update(&context, data_buf + 1111, sizeof(data_buf) - 1111);
    sha256_final(&context, hash);
    for (i = 0; i < SHA256_HASH_LEN;)
    {
        if (hash[i] != compare4[i])
            total_check_tag = 1;
        printf("%02x", hash[i++]);
        if (!(i % 4))
            printf(" ");
    }
    printf("\n");

    cycle = read_cycle() - cycle;
    if (total_check_tag == 1)
        printf("\nSHA256_TEST _TEST_FAIL_\n");
    else
        printf("\nSHA256_TEST _TEST_PASS_\n");
    printf("\nsha256 test time = %ld ms\n", cycle/(sysctl_clock_get_freq(SYSCTL_CLOCK_CPU)/1000));
    while(1);
    return 0;
}
