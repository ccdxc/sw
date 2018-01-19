/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>

#include "pal.h"
#include "pal_impl.h"

void *
pal_mem_map(const u_int64_t pa, const size_t sz)
{
    return pr_ptov(pa, sz);
}

void
pal_mem_unmap(void *va)
{
    /* XXX */
}

u_int64_t
pal_mem_vtop(const void *va)
{
    return pr_vtop(va, 4);
}

int
pal_mem_rd(const u_int64_t pa, void *buf, const size_t sz)
{
#ifdef __aarch64__
    pal_data_t *pd = pal_get_data();

    lseek(pd->memfd, pa, SEEK_SET);
    return read(pd->memfd, buf, sz);
#else
    memcpy(buf, pr_ptov(pa, sz), sz);
    return sz;
#endif
}

int
pal_mem_wr(const u_int64_t pa, const void *buf, const size_t sz)
{
#ifdef __aarch64__
    pal_data_t *pd = pal_get_data();

    lseek(pd->memfd, pa, SEEK_SET);
    return write(pd->memfd, buf, sz);
#else
    memcpy(pr_ptov(pa, sz), buf, sz);
    return sz;
#endif
}
