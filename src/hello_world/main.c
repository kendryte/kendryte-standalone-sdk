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
#include <bsp.h>
#include <sysctl.h>
#include "iomem_malloc.h"

int core1_function(void *ctx)
{
    uint64_t core = current_coreid();
    printf("Core %ld Hello world\n", core);
    while(1);
}

int main(void)
{
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    uint64_t core = current_coreid();
    int data;
    printf("Core %ld Hello world\n", core);
    register_core1(core1_function, NULL);

    /* Clear stdin buffer before scanf */
    sys_stdin_flush();
	extern unsigned int _iodata;

	unsigned char *t = (unsigned char *)((uintptr_t)&_iodata + 4);
	memcpy(t, "hello malloc", 15);
	//memcpy(_iomem_spi_dest, "hello world", 12);

	printf("_iomem_spi_array%s  %p\n", t, t);
    uint8_t *test = (uint8_t *)iomem_malloc(16);
    memcpy(test, "hello world", 12);
    printf("%s %p\n", test, test);
    iomem_free(test);
    while(1)
        continue;
    return 0;
}
