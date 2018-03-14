/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>

#include "pal.h"
#include "pal_impl.h"

u_int32_t
pal_reg_rd32(const u_int64_t pa)
{
    const u_int32_t val = *(u_int32_t *)pr_ptov(pa, 4);
    pal_reg_trace("reg_rd32 0x%08"PRIx64" = 0x%"PRIx32"\n", pa, val);
    return val;
}

u_int64_t
pal_reg_rd64(const u_int64_t pa)
{
    const u_int64_t val = *(u_int64_t *)pr_ptov(pa, 8);
    pal_reg_trace("reg_rd64 0x%08"PRIx64" = 0x%"PRIx64"\n", pa, val);
    return val;
}

void
pal_reg_rd32w(const u_int64_t pa,
              u_int32_t *w,
              const u_int32_t nw)
{
    u_int32_t *va = pr_ptov(pa, nw * 4);
    int i;

    for (i = 0; i < nw; i++) {
        *w++ = *va++;
        pal_reg_trace("reg_rd32 0x%08"PRIx64" = 0x%"PRIx32"\n",
                      pa + i * 4, w[-1]);
    }
}

void
pal_reg_wr32(const u_int64_t pa, const u_int32_t val)
{
    pal_reg_trace("reg_wr32 0x%08"PRIx64" = 0x%"PRIx32"\n", pa, val);
    *(u_int32_t *)pr_ptov(pa, 4) = val;
}

void
pal_reg_wr64(const u_int64_t pa, const u_int64_t val)
{
    pal_reg_trace("reg_wr64 0x%08"PRIx64" = 0x%"PRIx64"\n", pa, val);
    *(u_int64_t *)pr_ptov(pa, 8) = val;
}

void
pal_reg_wr32w(const u_int64_t pa,
              const u_int32_t *w,
              const u_int32_t nw)
{
    u_int32_t *va = pr_ptov(pa, nw * 4);
    int i;

    for (i = 0; i < nw; i++) {
        pal_reg_trace("reg_wr32 0x%08"PRIx64" = 0x%"PRIx32"\n",
                      pa + i * 4, *w);
        *va++ = *w++;
    }
}

u_int64_t
pal_reg_rd64_safe(const u_int64_t pa)
{
    u_int64_t val;
    u_int32_t *w = (u_int32_t *)&val;

    pal_reg_rd32w(pa, w, 2);
    return val;
}

void
pal_reg_wr64_safe(const u_int64_t pa, const u_int64_t val)
{
    const u_int32_t *w = (const u_int32_t *)&val;

    pal_reg_wr32w(pa, w, 2);
}

