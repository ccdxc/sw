/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __CAP_SW_GLUE_H__
#define __CAP_SW_GLUE_H__

/*
 * Glue file between cap_xxx.c ASIC API files and our application.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include "nic/sdk/platform/pciemgrutils/include/pciesys.h"
#include "nic/sdk/platform/pal/include/pal.h"

typedef u_int8_t bool;
#define true 1
#define false 0

#define SW_PRINT pciesys_logdebug
#define SWPRINT  pciesys_logdebug
#define SWPRINTF pciesys_logdebug

#define PLOG_API_MSG(f, ...) do {} while (0)
#define PLOG_MSG(f, ...) do {} while (0)
#define PLOG_ERR(f, ...) do {} while (0)
#define SLEEP(t) usleep(t)
#define sleep(t) usleep(t)

void *romfile_open(void *rom_info);
int romfile_read(void *ctx, unsigned int *datap);
void romfile_close(void *ctx);

static inline int sknobs_get_value(const char *name, int defval)
{
    return defval;
}

static inline void cap_sw_writereg(uint64_t addr, uint32_t data)
{
    pal_reg_wr32(addr, data);
}

static inline uint32_t cap_sw_readreg(uint64_t addr)
{
    return pal_reg_rd32(addr);
}

#endif /* __CAP_SW_GLUE_H__ */
