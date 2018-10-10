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
#include "sleep.h"
#include "bsp.h"
int main()
{
    uint64_t core_id = current_coreid();
    if (core_id == 0)
    {
        printf("Core 0 Hello, world!\n");
    }
    else
    {
        msleep(100);
        printf("Core 1 Hello, world!\n");
    }
    while (1)
        ;
    return 0;
}
