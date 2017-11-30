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

void *
pal_mem_map(const u_int64_t pa, const size_t sz)
{
    pal_region_t *pr = pr_new(pa, sz);
    return pr_map(pr);
}

void
pal_mem_unmap(void *va)
{
    /* XXX */
}

u_int64_t
pal_mem_vtop(const void *va)
{
    return pr_vtop(va);
}

int
pal_mem_rd(const u_int64_t pa, void *buf, const size_t sz)
{
    pal_data_t *pd = pal_get_data();

    lseek(pd->memfd, pa, SEEK_SET);
    return read(pd->memfd, buf, sz);
}

int
pal_mem_wr(const u_int64_t pa, const void *buf, const size_t sz)
{
    pal_data_t *pd = pal_get_data();

    lseek(pd->memfd, pa, SEEK_SET);
    return write(pd->memfd, buf, sz);
}
