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
#include <sys/param.h>
#include <sys/mman.h>
#include <execinfo.h>

#include "pal.h"
#include "internal.h"

/* Obtain a backtrace and print it to stdout. */
void
print_trace (void)
{
    void *array[10];
    size_t size;
    char **strings;
    size_t i;

    size = backtrace (array, 10);
    strings = backtrace_symbols (array, size);

    printf ("Obtained %zd stack frames.\n", size);

    for (i = 0; i < size; i++)
        printf ("%s\n", strings[i]);

    free (strings);
}

u_int8_t
pal_reg_rd8(const u_int64_t pa)
{
    const u_int8_t val = *(u_int8_t *)pr_ptov(pa, 1, FREEACCESS);
    pal_reg_trace("reg_rd8 0x%08"PRIx64" = 0x%"PRIx8"\n", pa, val);
    return val;
}

u_int16_t
pal_reg_rd16(const u_int64_t pa)
{
    const u_int16_t val = *(u_int16_t *)pr_ptov(pa, 2, FREEACCESS);
    pal_reg_trace("reg_rd16 0x%08"PRIx64" = 0x%"PRIx16"\n", pa, val);
    assert((pa & 0x1) == 0);
    return val;
}

u_int32_t
pal_reg_rd32(const u_int64_t pa)
{
    const u_int32_t val = *(u_int32_t *)pr_ptov(pa, 4, FREEACCESS);
    pal_reg_trace("reg_rd32 0x%08"PRIx64" = 0x%"PRIx32"\n", pa, val);
    assert((pa & 0x3) == 0);
    return val;
}

u_int64_t
pal_reg_rd64(const u_int64_t pa)
{
    const u_int64_t val = *(u_int64_t *)pr_ptov(pa, 8, FREEACCESS);
    pal_reg_trace("reg_rd64 0x%08"PRIx64" = 0x%"PRIx64"\n", pa, val);
    assert((pa & 0x7) == 0);
    return val;
}

void
pal_reg_rd32w(const u_int64_t pa,
              u_int32_t *w,
              const u_int32_t nw)
{
    u_int32_t *va = pr_ptov(pa, nw * 4, FREEACCESS);
    int i;

    assert((pa & 0x3) == 0);
    for (i = 0; i < nw; i++) {
        *w++ = *va++;
        pal_reg_trace("reg_rd32 0x%08"PRIx64" = 0x%"PRIx32"\n",
                      pa + i * 4, w[-1]);
    }
}

void
pal_reg_wr8(const u_int64_t pa, const u_int8_t val)
{
    pal_reg_trace("reg_wr8 0x%08"PRIx64" = 0x%"PRIx8"\n", pa, val);
    *(u_int32_t *)pr_ptov(pa, 1, FREEACCESS) = val;
}

void
pal_reg_wr16(const u_int64_t pa, const u_int16_t val)
{
    pal_reg_trace("reg_wr16 0x%08"PRIx64" = 0x%"PRIx16"\n", pa, val);
    assert((pa & 0x1) == 0);
    *(u_int32_t *)pr_ptov(pa, 2, FREEACCESS) = val;
}

void
pal_reg_wr32(const u_int64_t pa, const u_int32_t val)
{
    pal_reg_trace("reg_wr32 0x%08"PRIx64" = 0x%"PRIx32"\n", pa, val);
    assert((pa & 0x3) == 0);
    *(u_int32_t *)pr_ptov(pa, 4, FREEACCESS) = val;
}

void
pal_reg_wr64(const u_int64_t pa, const u_int64_t val)
{
    pal_reg_trace("reg_wr64 0x%08"PRIx64" = 0x%"PRIx64"\n", pa, val);
    assert((pa & 0x7) == 0);
    *(u_int64_t *)pr_ptov(pa, 8, FREEACCESS) = val;
}

void
pal_reg_wr32w(const u_int64_t pa,
              const u_int32_t *w,
              const u_int32_t nw)
{
    u_int32_t *va = pr_ptov(pa, nw * 4, FREEACCESS);
    int i;

    assert((pa & 0x3) == 0);
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


