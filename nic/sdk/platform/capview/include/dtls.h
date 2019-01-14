
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __DTLS_H__
#define __DTLS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <gmpxx.h>
#include "cvdb.h"

/*
 * main.c
 */
extern void *init_sbrk;

/*
 * cmds.c
 */
struct access_info_s {
    CVDBReg reg;
    uint32_t addr;
    int idx;
};

/*
 * regio.cc
 */
uint32_t regio_readl(uint64_t addr);
void regio_writel(uint64_t addr, uint32_t val);

/*
 * log.cc
 */
void logf(const char *fmt, ...) __attribute((format(printf, 1, 2)));

#endif
