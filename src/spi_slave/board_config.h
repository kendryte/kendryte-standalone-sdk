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
#ifndef _BOARD_CONFIG_
#define _BOARD_CONFIG_

#include "sysctl.h"

#define  BOARD_KD233        1
#define  BOARD_LICHEEDAN    0

#if ((BOARD_KD233 + BOARD_LICHEEDAN) != 1)
    #error board only choose one
#endif

#if BOARD_KD233
    /* power domain set */
    #define POWER_BANK0_SELECT  SYSCTL_POWER_V18
    #define POWER_BANK1_SELECT  SYSCTL_POWER_V18
    #define POWER_BANK2_SELECT  SYSCTL_POWER_V18
    #define POWER_BANK3_SELECT  SYSCTL_POWER_V33
    #define POWER_BANK4_SELECT  SYSCTL_POWER_V33
    #define POWER_BANK5_SELECT  SYSCTL_POWER_V33
    #define POWER_BANK6_SELECT  SYSCTL_POWER_V33
    #define POWER_BANK7_SELECT  SYSCTL_POWER_V33
#elif BOARD_LICHEEDAN
    /* power domain set */
    #define POWER_BANK0_SELECT  SYSCTL_POWER_V33
    #define POWER_BANK1_SELECT  SYSCTL_POWER_V33
    #define POWER_BANK2_SELECT  SYSCTL_POWER_V33
    #define POWER_BANK3_SELECT  SYSCTL_POWER_V33
    #define POWER_BANK4_SELECT  SYSCTL_POWER_V33
    #define POWER_BANK5_SELECT  SYSCTL_POWER_V33
    #define POWER_BANK6_SELECT  SYSCTL_POWER_V18
    #define POWER_BANK7_SELECT  SYSCTL_POWER_V18
#endif

#endif