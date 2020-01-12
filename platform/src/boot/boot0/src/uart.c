
/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include "dtls.h"
#include "iomem.h"
#include "defs.h"
#include "cap_uart.h"

static inline void
uart_writereg(int reg, int val)
{
    writereg(UART0_BASE + reg, val);
}

static inline uint32_t
uart_readreg(int reg)
{
    return readreg(UART0_BASE + reg);
}

void
uart_wait_idle(void)
{
    while ((uart_readreg(UART_LSR) & 0x40) == 0) {
        ; /* spin */
    }
}

void
uart_init(void)
{
    uint32_t div;

    div = 2 * UART_CLK / UART_BAUD / 16;
    div = (div >> 1) + (div & 0x1);

    uart_wait_idle();

    uart_writereg(UART_LCR, 0x80);
    uart_writereg(UART_DLL, div & 0xff);
    uart_writereg(UART_DLH, div >> 8);
    uart_writereg(UART_LCR, 0x03);
    uart_writereg(UART_FCR, 0x03);
}

void
_putchar(int c)
{
    while ((uart_readreg(UART_LSR) & 0x20) == 0) {
        ; /* spin */
    }
    uart_writereg(UART_THR, c);
}

void
putchar(int c)
{
    if (c == '\n') {
        _putchar('\r');
    }
    _putchar(c);
}

void
puts(const char *s)
{
    for (int c = *s++; c != '\0'; c = *s++) {
        putchar(c);
    }
    putchar('\n');
}
