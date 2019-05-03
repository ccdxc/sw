
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdint.h>
#include "dtls.h"
#include "iomem.h"
#include "defs.h"
#include "uart.h"

static inline void uart_writereg(int reg, int val)
{
    writereg(UART0_BASE + reg, val);
}

static inline uint32_t
uart_readreg(int reg)
{
    return readreg(UART0_BASE + reg);
}

void
uart_init(void)
{
    uint32_t div;

    div = 2 * UART_CLK / UART_BAUD / 16;
    div = (div >> 1) + (div & 0x1);

    uart_writereg(UART_LCR, 0x80);
    uart_writereg(UART_DLL, div & 0xff);
    uart_writereg(UART_DLH, div >> 8);
    uart_writereg(UART_LCR, 0x03);
    uart_writereg(UART_FCR, 0x03);
}

static void
_uart_putchar(int c)
{
    while ((uart_readreg(UART_LSR) & 0x20) == 0) {
        ; /* spin */
    }
    uart_writereg(UART_THR, c);
}

static void
uart_putchar(int c)
{
    if (c == '\n') {
        _uart_putchar('\r');
    }
    _uart_putchar(c);
}

void
uart_write(const char *s)
{
    for (int c = *s++; c != '\0'; c = *s++) {
        uart_putchar(c);
    }
}
