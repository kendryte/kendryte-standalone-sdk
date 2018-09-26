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
#include <stdlib.h>
#include <stdint.h>
#include "plic.h"
#include "sysctl.h"
#include "uart.h"
#include "common.h"
#include "atomic.h"

#define __UART_BRATE_CONST  16

typedef struct _uart_devinfo_t
{
    uart_t *uart;
    const uart_info_t *uart_info;
    uint32_t uart_status;
} uart_devinfo_t;

volatile uart_t* const  uart[3] =
{
    (volatile uart_t*)UART1_BASE_ADDR,
    (volatile uart_t*)UART2_BASE_ADDR,
    (volatile uart_t*)UART3_BASE_ADDR
};

#define RING_BUFF_LEN 64U

typedef struct _ring_buff_t
{
    size_t head;
    size_t tail;
    size_t length;
    char ring_buff[RING_BUFF_LEN];
} ring_buff_t;

ring_buff_t *ring_recv[3] = {NULL, NULL, NULL};

static int write_ringbuff(uint8_t channel, uint8_t rdata)
{
    ring_buff_t *rb = ring_recv[channel];

    if (rb->length >= RING_BUFF_LEN)
    {
        return -1;
    }
    rb->ring_buff[rb->tail] = rdata;
    rb->tail = (rb->tail + 1) % RING_BUFF_LEN;
    atomic_add(&rb->length, 1);
    return 0;
}

static int read_ringbuff(uint8_t channel, char *rdata, size_t len)
{
    ring_buff_t *rb = ring_recv[channel];
    size_t cnt = 0;
    while ((len--) && rb->length)
    {
        *(rdata++) = rb->ring_buff[rb->head];
        rb->head = (rb->head + 1) % RING_BUFF_LEN;
        atomic_add(&rb->length, -1);
        cnt++;
    }
    return cnt;
}

static int on_irq_apbuart_recv(void *param)
{
    int i = 0;
    for (i = 0; i < 3; i++)
    {
        if (param == uart[i])
            break;
    }

    while (uart[i]->LSR & 1)
    {
        write_ringbuff(i, ((uint8_t)(uart[i]->RBR & 0xff)));
    }
    return 0;
}

static int uartapb_putc(uint8_t channel, char c)
{
    while (!(uart[channel]->LSR & (1u << 6)))
        continue;
    uart[channel]->THR = c;
    return 0;
}

int uartapb_getc(uint8_t channel)
{
    while (!(uart[channel]->LSR & 1))
        continue;

    return (char)(uart[channel]->RBR & 0xff);
}


int uart_read(uint8_t channel, char* buffer, size_t len)
{
    return read_ringbuff(channel, buffer, len);
}

int uart_write(uint8_t channel, const char* buffer, size_t len)
{
    int write = 0;
    while (write < len)
    {
        uartapb_putc(channel, *buffer++);
        write++;
    }

    return write;
}

void uart_config(uint8_t channel, size_t baud_rate, size_t data_width, uart_stopbit_t stopbit, uart_parity_t parity)
{

    configASSERT(data_width >= 5 && data_width <= 8);
    if (data_width == 5)
    {
        configASSERT(stopbit != UART_STOP_2);
    }
    else
    {
        configASSERT(stopbit != UART_STOP_1_5);
    }

    uint32_t stopbit_val = stopbit == UART_STOP_1 ? 0 : 1;
    uint32_t parity_val;
    switch (parity)
    {
        case UART_PARITY_NONE:
            parity_val = 0;
            break;
        case UART_PARITY_ODD:
            parity_val = 1;
            break;
        case UART_PARITY_EVEN:
            parity_val = 3;
            break;
        default:
            configASSERT(!"Invalid parity");
            break;
    }

    uint32_t freq = sysctl_clock_get_freq(SYSCTL_CLOCK_APB0);
    uint32_t u16Divider = (freq + __UART_BRATE_CONST * baud_rate / 2) /
        (__UART_BRATE_CONST * baud_rate);

    /* Set UART registers */
    uart[channel]->TCR &= ~(1u);
    uart[channel]->TCR &= ~(1u << 3);
    uart[channel]->TCR &= ~(1u << 4);
    uart[channel]->TCR |= (1u << 2);
    uart[channel]->TCR &= ~(1u << 1);
    uart[channel]->DE_EN &= ~(1u);

    uart[channel]->LCR |= 1u << 7;
    uart[channel]->DLL = u16Divider & 0xFF;
    uart[channel]->DLH = u16Divider >> 8;
    uart[channel]->LCR = 0;
    uart[channel]->LCR = (data_width - 5) | (stopbit_val << 2) | (parity_val << 3);
    uart[channel]->LCR &= ~(1u << 7);
    uart[channel]->MCR &= ~3;
    uart[channel]->IER = 1;     /*RX INT enable*/
}

void uartapb_init(uint8_t channel)
{
    sysctl_clock_enable(SYSCTL_CLOCK_UART1 + channel);

    ring_buff_t *rb = malloc(sizeof(ring_buff_t));
    rb->head = 0;
    rb->tail = 0;
    rb->length = 0;
    ring_recv[channel] = rb;
    plic_irq_register(IRQN_UART1_INTERRUPT + channel, on_irq_apbuart_recv, (void *)uart[channel]);
    plic_set_priority(IRQN_UART1_INTERRUPT + channel, 1);
    plic_irq_enable(IRQN_UART1_INTERRUPT);
}
