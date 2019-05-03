
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __DTLS_H__
#define __DTLS_H__

#include <stdarg.h>

#ifndef NULL
#define NULL 0
#endif

// main.c
void main(void) __attribute__((noreturn));

// panic.c
void panic(const char *s) __attribute__((noreturn));

// efuse.c
int is_secure_boot_enabled(void);

// esecure.c
void esec_mark_boot_success(void);

// uart.c
void uart_init(void);
void uart_write(const char *s);

#endif
