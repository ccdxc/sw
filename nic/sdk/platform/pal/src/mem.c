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
#include <sys/param.h>
#include <sys/mman.h>

#include "pal.h"
#include "mm_int.h"
#include "internal.h"

void *
pal_mem_map(const u_int64_t pa, const size_t sz, u_int32_t flags)
{
    void *va = pr_ptov(pa, sz, FREEACCESS);

    pal_mem_trace("mem_map 0x%08"PRIx64" sz %ld\n", pa, sz);

    if (va == NULL && pal_pa_access_allowed(pa, sz)) {
        va = pr_ptov(pa, sz, FREEACCESS);
    }

    return va;
}

void
pal_mem_unmap(void *va)
{
    /* XXX */
    pr_mem_unmap(va);
}

u_int64_t
pal_mem_vtop(const void *va)
{
    return pr_vtop(va, 4);
}

void *
pal_mem_ptov(const u_int64_t pa)
{
    return pr_ptov(pa, 4, FREEACCESS);
}

int
pal_mem_rd(const u_int64_t addr, void *buf, const size_t sz, uint32_t flags)
{
    void *va;

    pal_mem_trace("mem_rd 0x%08"PRIx64" sz %ld\n", addr, sz);
    va = pr_ptov(addr, sz, FREEACCESS);
    if (va == NULL && pal_pa_access_allowed(addr, sz)) {
        va = pr_ptov(addr, sz, FREEACCESS);
    }
    if (va != NULL) {
        return pal_memcpy(buf, va, sz);
    } else {
        printf("No permission to perform this operation.\n");
        return 0;
    }
}

int
pal_mem_wr(const u_int64_t addr, const void *buf, const size_t sz, uint32_t flags)
{

    void *va;

    pal_mem_trace("mem_wr 0x%08"PRIx64" sz %ld\n", addr, sz);
    va = pr_ptov(addr, sz, FREEACCESS);
    if (va == NULL && pal_pa_access_allowed(addr, sz)) {
        va = pr_ptov(addr, sz, FREEACCESS);
    }

    if (va != NULL) {
        return pal_memcpy(va, buf, sz);
    } else {
        printf("Application does not have permission to access this memory region.\n");
        return 0;
    }
}

int
pal_memset(const uint64_t pa, u_int8_t c, const size_t sz, u_int32_t flags)
{
    u_int8_t *d = pr_ptov(pa, sz, FREEACCESS);
    int i;

    if (d == NULL && pal_pa_access_allowed(pa, sz)) {
        d = pr_ptov(pa, sz, FREEACCESS);
    }

    if (d != NULL) {
        for (i = 0; i < sz; i++) {
            *d++ = c;
        }
        return sz;
    } else {
        printf("No permission to perform this operation.\n");
        return 0;
    }
}

int
pal_mem_get_phys_ranges(pal_mem_phys_rangetab_t *tab)
{
    const pal_data_t *pd = pal_get_data();

    if (tab == NULL) {
        return -1;
    }
    tab->nranges = pd->nphysmem;
    tab->range = pd->physmem;
    return 0;
}

uint64_t
pal_mem_get_phys_totalsize(void)
{
    const pal_data_t *pd = pal_get_data();
    uint64_t totsz;
    int i;

    totsz = 0;
    for (i = 0; i < pd->nphysmem; i++) {
        totsz += pd->physmem[i].sz;
    }
    return totsz;
}
