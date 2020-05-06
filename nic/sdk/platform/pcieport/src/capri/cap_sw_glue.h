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
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pal/include/pal.h"

#define SW_PRINT pciesys_logdebug
#define SWPRINT  pciesys_logdebug
#define SWPRINTF pciesys_logdebug

#define PLOG_API_MSG(f, ...) do {} while (0)
#define PLOG_MSG(f, ...) do {} while (0)
#define PLOG_ERR(f, ...) do {} while (0)
#define SLEEP(t) usleep(t)
#define sleep(t) usleep(t)

#define SBUS_ROM_MAGIC 0x53554253

struct rom_ctx_s {
    const uint32_t *buf;
    int nwords;
};

static inline int
romfile_read(void *ctx, unsigned int *datap)
{
    struct rom_ctx_s *p = (struct rom_ctx_s *)ctx;

    if (!p->nwords) {
        return 0;
    }
    *datap = *p->buf++;
    p->nwords--;
    return 1;
}

void *romfile_open(void *rom_info);
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
