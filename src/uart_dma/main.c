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
#include "fpioa.h"
#include <string.h>
#include "uart.h"
#include "gpiohs.h"
#include "sysctl.h"
#include <unistd.h>

#define RECV_LENTH  4

#define CLOSLIGHT   0x55555555
#define OPENLIGHT   0xAAAAAAAA

#define UART_NUM    UART_DEVICE_3

volatile uint32_t s_count = 0;
volatile uint32_t s_recv_cnt = 0;
volatile uint32_t g_send_count = 0;
uint8_t recv_buf[128];

int release_cmd(char *cmd)
{
    switch(*((int *)cmd)){
        case CLOSLIGHT:
        gpiohs_set_pin(3, GPIO_PV_LOW);
        break;
        case OPENLIGHT:
        gpiohs_set_pin(3, GPIO_PV_HIGH);
        break;
    }
    return 0;
}

void io_mux_init(void)
{
    fpioa_set_function(4, FUNC_UART1_RX + UART_NUM * 2);
    fpioa_set_function(5, FUNC_UART1_TX + UART_NUM * 2);
    fpioa_set_function(24, FUNC_GPIOHS3);
}

int main()
{
    io_mux_init();
    plic_init();
    sysctl_enable_irq();

    gpiohs_set_drive_mode(3, GPIO_DM_OUTPUT);
    gpio_pin_value_t value = GPIO_PV_HIGH;
    gpiohs_set_pin(3, value);

    uart_init(UART_NUM);
    uart_configure(UART_NUM, 115200, 8, UART_STOP_1, UART_PARITY_NONE);

    char *hel = {"hello world!\n"};
    uart_send_data_dma(UART_NUM, DMAC_CHANNEL0, (uint8_t *)hel, strlen(hel));

    char recv = 0;
    int rec_flag = 0;
    char cmd[8];
    int i = 0;
    while (1)
    {
        uart_receive_data_dma(UART_NUM, DMAC_CHANNEL1, (uint8_t *)&recv, 1);    /*block wait for receive*/
        switch(rec_flag)
        {
            case 0:
                if(recv == 0x55)
                    rec_flag = 1;
                break;
            case 1:
                if(recv == 0xAA)
                    rec_flag = 2;
                else if(recv != 0x55)
                    rec_flag = 0;
                break;
            case 2:
                cmd[i++] = recv;
                if(i >= RECV_LENTH)
                {
                    i = 0;
                    release_cmd(cmd);
                    rec_flag = 0;
                }
                break;
        }
    }
}

