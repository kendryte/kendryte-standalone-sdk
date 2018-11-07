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
#include "utils.h"
#include "atomic.h"

#define __UART_BRATE_CONST  16

volatile uart_t* const  uart[3] =
{
    (volatile uart_t*)UART1_BASE_ADDR,
    (volatile uart_t*)UART2_BASE_ADDR,
    (volatile uart_t*)UART3_BASE_ADDR
};

#define UART_INTERRUPT_SEND                 0x02U
#define UART_INTERRUPT_RECEIVE              0x04U
#define UART_INTERRUPT_CHARACTER_TIMEOUT    0x0CU

typedef struct _uart_interrupt_context
{
    plic_irq_callback_t callback;
    void *ctx;
} uart_interrupt_context_t;

typedef struct _uart_context
{
    uart_interrupt_context_t uart_receive_context;
    uart_interrupt_context_t uart_send_context;
    uint32_t uart_num;
    uint32_t send_fifo_intterupt : 2;
    uint32_t receive_fifo_intterupt : 2;
    uint32_t reserved:28;
} uart_context_t;

uart_context_t g_uart_context[3];

typedef struct _uart_dma_context
{
    uint8_t *buffer;
    size_t buf_len;
    uint32_t *malloc_buffer;
    uart_interrupt_mode_t int_mode;
    dmac_channel_number_t dmac_channel;
    uart_device_number_t uart_num;
    uart_interrupt_context_t uart_int_context;
} uart_dma_context_t;

uart_dma_context_t uart_send_dma_context[3];
uart_dma_context_t uart_recv_dma_context[3];

volatile int g_write_count = 0;

static int uart_irq_callback(void *param)
{
    uart_context_t *uart_context = (uart_context_t *)param;
    uint32_t v_channel = uart_context->uart_num;
    uint8_t v_int_status = uart[v_channel]->IIR & 0xF;

    if(v_int_status == UART_INTERRUPT_SEND && g_write_count != 0)
    {
        if(uart_context->uart_send_context.callback != NULL)
            uart_context->uart_send_context.callback(uart_context->uart_send_context.ctx);
    }
    else if(v_int_status == UART_INTERRUPT_RECEIVE || v_int_status == UART_INTERRUPT_CHARACTER_TIMEOUT)
    {
        if(uart_context->uart_receive_context.callback != NULL)
            uart_context->uart_receive_context.callback(uart_context->uart_receive_context.ctx);
    }
    return 0;
}

static int uartapb_putc(uart_device_number_t channel, char c)
{
    while (uart[channel]->LSR & (1u << 5))
        continue;
    uart[channel]->THR = c;
    return 0;
}

int uartapb_getc(uart_device_number_t channel)
{
    while (!(uart[channel]->LSR & 1))
        continue;

    return (char)(uart[channel]->RBR & 0xff);
}

int uart_dma_callback(void *ctx)
{
    uart_dma_context_t *v_uart_dma_context = (uart_dma_context_t *)ctx;
    dmac_channel_number_t dmac_channel = v_uart_dma_context->dmac_channel;
    dmac_free_irq(dmac_channel);

    if(v_uart_dma_context->int_mode == UART_RECEIVE)
    {
        size_t v_buf_len = v_uart_dma_context->buf_len;
        uint8_t *v_buffer = v_uart_dma_context->buffer;
        uint32_t *v_recv_buffer = v_uart_dma_context->malloc_buffer;
        for(size_t i = 0; i < v_buf_len; i++)
        {
            v_buffer[i] = v_recv_buffer[i];
        }
    }
    free(v_uart_dma_context->malloc_buffer);
    if(v_uart_dma_context->uart_int_context.callback)
        v_uart_dma_context->uart_int_context.callback(v_uart_dma_context->uart_int_context.ctx);
    return 0;
}

int uart_receive_data(uart_device_number_t channel, char *buffer, size_t buf_len)
{
    size_t i = 0;
    for(i = 0;i < buf_len; i++)
    {
        if(uart[channel]->LSR & 1)
            buffer[i] = (char)(uart[channel]->RBR & 0xff);
        else
            break;
    }
    return i;
}

void uart_receive_data_dma(uart_device_number_t uart_channel, dmac_channel_number_t dmac_channel, uint8_t *buffer, size_t buf_len)
{
    uint32_t *v_recv_buf = malloc(buf_len * sizeof(uint32_t));
        configASSERT(v_recv_buf!=NULL);
    dmac_channel_disable(dmac_channel);
    dmac_wait_idle(dmac_channel);
    sysctl_dma_select((sysctl_dma_channel_t)dmac_channel, SYSCTL_DMA_SELECT_UART1_RX_REQ + uart_channel * 2);
    dmac_set_single_mode(dmac_channel, (void *)(&uart[uart_channel]->RBR), v_recv_buf, DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT,
        DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, buf_len);
    dmac_wait_done(dmac_channel);
    for(uint32_t i = 0; i < buf_len; i++)
    {
        buffer[i] = (uint8_t)(v_recv_buf[i] & 0xff);
    }
}

void uart_receive_data_dma_irq(uart_device_number_t uart_channel, dmac_channel_number_t dmac_channel,
                                     uint8_t *buffer, size_t buf_len, plic_irq_callback_t uart_callback,
                                     void *ctx, uint32_t priority)
{
    uint32_t *v_recv_buf = malloc(buf_len * sizeof(uint32_t));
        configASSERT(v_recv_buf!=NULL);

    uart_recv_dma_context[uart_channel].dmac_channel = dmac_channel;
    uart_recv_dma_context[uart_channel].uart_num = uart_channel;
    uart_recv_dma_context[uart_channel].malloc_buffer = v_recv_buf;
    uart_recv_dma_context[uart_channel].buffer = buffer;
    uart_recv_dma_context[uart_channel].buf_len = buf_len;
    uart_recv_dma_context[uart_channel].int_mode = UART_RECEIVE;
    uart_recv_dma_context[uart_channel].uart_int_context.callback = uart_callback;
    uart_recv_dma_context[uart_channel].uart_int_context.ctx = ctx;

    dmac_set_irq(dmac_channel, uart_dma_callback, &uart_recv_dma_context[uart_channel], priority);
    sysctl_dma_select((sysctl_dma_channel_t)dmac_channel, SYSCTL_DMA_SELECT_UART1_RX_REQ + uart_channel * 2);
    dmac_set_single_mode(dmac_channel, (void *)(&uart[uart_channel]->RBR), v_recv_buf, DMAC_ADDR_NOCHANGE, DMAC_ADDR_INCREMENT,
        DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, buf_len);
}

int uart_send_data(uart_device_number_t channel, const char *buffer, size_t buf_len)
{
    g_write_count = 0;
    while (g_write_count < buf_len)
    {
        uartapb_putc(channel, *buffer++);
        g_write_count++;
    }
    return g_write_count;
}

void uart_send_data_dma(uart_device_number_t uart_channel, dmac_channel_number_t dmac_channel, const uint8_t *buffer, size_t buf_len)
{
    uint32_t *v_send_buf = malloc(buf_len * sizeof(uint32_t));
    configASSERT(v_send_buf!=NULL);
    for(uint32_t i = 0; i < buf_len; i++)
        v_send_buf[i] = buffer[i];
    dmac_channel_disable(dmac_channel);
    dmac_wait_idle(dmac_channel);
    sysctl_dma_select((sysctl_dma_channel_t)dmac_channel, SYSCTL_DMA_SELECT_UART1_TX_REQ + uart_channel * 2);
    dmac_set_single_mode(dmac_channel, v_send_buf, (void *)(&uart[uart_channel]->THR), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
        DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, buf_len);
    dmac_wait_done(dmac_channel);
    free((void *)v_send_buf);
}

void uart_send_data_dma_irq(uart_device_number_t uart_channel, dmac_channel_number_t dmac_channel,
                            const uint8_t *buffer, size_t buf_len, plic_irq_callback_t uart_callback,
                            void *ctx, uint32_t priority)
{
    uint32_t *v_send_buf = malloc(buf_len * sizeof(uint32_t));
    configASSERT(v_send_buf!=NULL);

    uart_send_dma_context[uart_channel].dmac_channel = dmac_channel;
    uart_send_dma_context[uart_channel].uart_num = uart_channel;
    uart_send_dma_context[uart_channel].malloc_buffer = v_send_buf;
    uart_send_dma_context[uart_channel].buffer = (uint8_t *)buffer;
    uart_send_dma_context[uart_channel].buf_len = buf_len;
    uart_send_dma_context[uart_channel].int_mode = UART_SEND;
    uart_send_dma_context[uart_channel].uart_int_context.callback = uart_callback;
    uart_send_dma_context[uart_channel].uart_int_context.ctx = ctx;

    for(uint32_t i = 0; i < buf_len; i++)
        v_send_buf[i] = buffer[i];
    dmac_set_irq(dmac_channel, uart_dma_callback, &uart_send_dma_context[uart_channel], priority);
    sysctl_dma_select((sysctl_dma_channel_t)dmac_channel, SYSCTL_DMA_SELECT_UART1_TX_REQ + uart_channel * 2);
    dmac_set_single_mode(dmac_channel, v_send_buf, (void *)(&uart[uart_channel]->THR), DMAC_ADDR_INCREMENT, DMAC_ADDR_NOCHANGE,
        DMAC_MSIZE_1, DMAC_TRANS_WIDTH_32, buf_len);
}

void uart_config(uart_device_number_t channel, uint32_t baud_rate, uart_bitwidth_t data_width, uart_stopbit_t stopbit, uart_parity_t parity)
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
    uart[channel]->IER |= 0x80; /* THRE */
    g_uart_context[channel].receive_fifo_intterupt = UART_RECEIVE_FIFO_1;
    g_uart_context[channel].send_fifo_intterupt = UART_SEND_FIFO_8;
    uart[channel]->FCR = UART_RECEIVE_FIFO_1 << 6 | UART_SEND_FIFO_8 << 4 | 0x1 << 3 | 0x1;
}

void uart_init(uart_device_number_t channel)
{
    sysctl_clock_enable(SYSCTL_CLOCK_UART1 + channel);
}

void uart_set_send_trigger(uart_device_number_t channel, uart_send_trigger_t trigger)
{
    g_uart_context[channel].send_fifo_intterupt = trigger;
    uart[channel]->FCR = g_uart_context[channel].receive_fifo_intterupt << 6 | g_uart_context[channel].send_fifo_intterupt | 0x1;
}

void uart_set_receive_trigger(uart_device_number_t channel, uart_receive_trigger_t trigger)
{
    g_uart_context[channel].receive_fifo_intterupt = trigger;
    uart[channel]->FCR = g_uart_context[channel].receive_fifo_intterupt << 6 | g_uart_context[channel].send_fifo_intterupt | 0x1;
}

void uart_set_irq(uart_device_number_t channel, uart_interrupt_mode_t interrupt_mode, plic_irq_callback_t uart_callback, void *ctx, uint32_t priority)
{
    if(interrupt_mode == UART_SEND)
    {
        uart[channel]->IER |= 0x2;
        g_uart_context[channel].uart_send_context.callback = uart_callback;
        g_uart_context[channel].uart_send_context.ctx = ctx;
    }
    else if(interrupt_mode == UART_RECEIVE)
    {
        uart[channel]->IER |= 0x1;
        g_uart_context[channel].uart_receive_context.callback = uart_callback;
        g_uart_context[channel].uart_receive_context.ctx = ctx;
    }
    g_uart_context[channel].uart_num = channel;
    plic_irq_disable(IRQN_UART1_INTERRUPT + channel);
    plic_set_priority(IRQN_UART1_INTERRUPT + channel, priority);
    plic_irq_register(IRQN_UART1_INTERRUPT + channel, uart_irq_callback, &g_uart_context[channel]);
    plic_irq_enable(IRQN_UART1_INTERRUPT + channel);
}

void uart_free_irq(uart_device_number_t channel, uart_interrupt_mode_t interrupt_mode)
{
    if(interrupt_mode == UART_SEND)
    {
        uart[channel]->IER &= ~(0x2);
        g_uart_context[channel].uart_send_context.callback = NULL;
        g_uart_context[channel].uart_send_context.ctx = NULL;
    }
    else if(interrupt_mode == UART_RECEIVE)
    {
        uart[channel]->IER &= ~(0x1);
        g_uart_context[channel].uart_receive_context.callback = NULL;
        g_uart_context[channel].uart_receive_context.ctx = NULL;
    }
    if(uart[channel]->IER == 0)
    {
        plic_irq_disable(IRQN_UART1_INTERRUPT + channel);
        plic_irq_deregister(IRQN_UART1_INTERRUPT + channel);
    }
}

