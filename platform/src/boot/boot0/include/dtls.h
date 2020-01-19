
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __DTLS_H__
#define __DTLS_H__

#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "cap_ms_c_hdr.h"
#include "iomem.h"

#define ARRAY_SIZE(n)       (sizeof (n) / sizeof ((n)[0]))

#define bsm_readreg(a)      readreg(a)
#define bsm_writereg(a, v)  writereg(a, v)

#include "bsm.h"

// Mapping from [track][attempt] to selected fwid.
typedef struct {
    uint8_t map[4][4];
} bsm_fwid_map_t;

// board.c
int get_chip_type(void);
int cap_board_type(void);
int get_cpld_id(void);
uint32_t board_qspi_frequency(void);
uint8_t board_qspi_read_delay(void);
const bsm_fwid_map_t *board_bsm_fwid_map(void);
uint8_t board_bsm_wdt_disable(void);
uint8_t board_reset_on_panic(void);
int board_get_part(const char *name, intptr_t *addrp, uint32_t *sizep);
int board_get_bfl_log2_secsize();

// cpld.c
uint8_t cpld_read(int reg);

// gpio.c
void gpio_init(void);
int gpio_bit(int pin);
uint32_t gpio_read_bits(void);

// fwsel.c
enum {
    FW_MAIN_A = BSM_FWID_MAINA,
    FW_MAIN_B = BSM_FWID_MAINB,
    FW_GOLD   = BSM_FWID_GOLD,
    FW_DIAG   = BSM_FWID_DIAG
};
int get_pri_fw(void);

// log.c
void logf(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void logf_printf(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

// panic.c
struct xcpt_regs {
    uint64_t elr;       // +000
    uint64_t esr;       // +008
    uint64_t sp;        // +010
    uint64_t x[31];     // +018...117
};
void panic(const char *s) __attribute__((noreturn));
void xcpt_panic(struct xcpt_regs *regs) __attribute__((noreturn));

// printf.c
void printf(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void snprintf(char *buf, size_t len, const char *fmt, ...) \
        __attribute__ ((format (printf, 3, 4)));
void vprintf(const char *fmt, va_list ap);
void vsnprintf(char *buf, size_t len, const char *fmt, va_list ap);

// qspi.c
void qspi_init(void);

// uart.c
void uart_wait_idle(void);
void uart_init(void);
void putchar(int c);
void puts(const char *s);

// uboot.c
int is_uboot_valid(intptr_t image_addr, uint32_t part_size);

// wdt.c
void wdt_enable_chip_reset(int wdt);
void wdt_start(int wdt, int to);
void wdt_pause(int wdt, int en);
void wdt_kick(int wdt);
void wdt_sys_reset(void) __attribute__((noreturn));

#endif
