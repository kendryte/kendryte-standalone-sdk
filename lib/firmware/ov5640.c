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
#include "ov5640.h"
#include "ov5640cfg.h"
#include <stdio.h>
#include "ov2640.h"
#include "dvp.h"
#include "plic.h"

extern void mdelay(uint32_t ms);

void hal_delay(uint32_t delay)
{
    mdelay(delay);
}

uint8_t ov5640_wr_reg(uint16_t reg,uint8_t data)
{
    dvp_sccb_write(OV5640_ADDR, reg, data);
    return 0;
}

uint8_t ov5640_rd_reg(uint16_t reg)
{
    return dvp_sccb_read(OV5640_ADDR, reg);
}

uint8_t ov5640_init(void)
{
    uint16_t i = 0;
    uint16_t reg = 0;

    reg = ov5640_rd_reg(OV5640_CHIPIDH);
    reg <<= 8;
    reg |= ov5640_rd_reg(OV5640_CHIPIDL);
    printf("ID: %X \r\n", reg);
    if(reg != OV5640_ID)
    {
        printf("ID: %d \r\n", reg);
        return 1;
    }

    ov5640_wr_reg(0x3103,0X11); /*system clock from pad, bit[1]*/
    ov5640_wr_reg(0X3008,0X82);
    hal_delay(10);

    for(i = 0; i<sizeof(ov5640_init_reg_tbl) / 4; i++)
    {
        ov5640_wr_reg(ov5640_init_reg_tbl[i][0], ov5640_init_reg_tbl[i][1]);
    }

    hal_delay(50);
    /* Test for flash light*/
    ov5640_flash_lamp(1);
    hal_delay(50);
    ov5640_flash_lamp(0);

    return 0x00;
}

void ov5640_flash_lamp(uint8_t sw)
{
    ov5640_wr_reg(0x3016, 0X02);
    ov5640_wr_reg(0x301C, 0X02);
    if(sw)
        ov5640_wr_reg(0X3019, 0X02);
    else
        ov5640_wr_reg(0X3019, 0X00);
}

