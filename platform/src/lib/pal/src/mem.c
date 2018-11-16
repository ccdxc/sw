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

#if PAL_TRACE
    pal_mem_trace("mem_map 0x%08"PRIx64" sz %ld\n", pa, sz);
#endif

#if PAL_SECURE
    if (va == NULL && pal_pa_access_allowed(pa, sz)) {
        va = pr_ptov(pa, sz, FREEACCESS);
    } 
#endif

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

void *
pal_memcpy(void *dst, const void *src, size_t n)
{
    volatile u_int8_t *d = dst;
    const u_int8_t *s = src;
    int i;

    if (src != NULL) {
        for (i = 0; i < n; i++) {
            *d++ = *s++;
        }
    } else {
        /* TODO : Remove this.
         * 
         *        This is placed here to satisfy a hack within HAL which uses
         *        the pal_mem_wr interface to perform ZERO-ing of memory by
         *        passing src as NULL.
         */
        for (i = 0; i < n; i++) {
            *d++ = 0;
        }
    }

    return dst;
}

int
pal_mem_rd(const u_int64_t pa, void *buf, const size_t sz, u_int32_t flags)
{
    void *va = pr_ptov(pa, sz, FREEACCESS);

#if PAL_TRACE
    pal_mem_trace("mem_rd 0x%08"PRIx64" sz %ld\n", pa, sz);
#endif

#if PAL_SECURE
    if (va == NULL && pal_pa_access_allowed(pa, sz)) {
        va = pr_ptov(pa, sz, FREEACCESS);
    } 
#endif

    if (va != NULL) {
        pal_memcpy(buf, va, sz);
        return sz;
    } else {
        printf("No permission to perform this operation.\n");
        return 0;
    }

}

int
pal_mem_wr(const u_int64_t pa, const void *buf, const size_t sz, u_int32_t flags)
{
    void *va = pr_ptov(pa, sz, FREEACCESS);

#if PAL_TRACE
    pal_mem_trace("mem_wr 0x%08"PRIx64" sz %ld\n", pa, sz);
#endif

#if PAL_SECURE
    if (va == NULL && pal_pa_access_allowed(pa, sz)) {
        va = pr_ptov(pa, sz, FREEACCESS); 
    } 
#endif

    if (va != NULL) { 
        pal_memcpy(va, buf, sz);
        return sz;
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

#if PAL_SECURE
    if (d == NULL && pal_pa_access_allowed(pa, sz)) {
        d = pr_ptov(pa, sz, FREEACCESS);
    }
#endif

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

