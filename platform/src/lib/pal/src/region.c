/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "pal.h"
#include "pal_impl.h"

#define PAL_REGSZ       (1 << 20)
#define PAL_REGBASE     (~(PAL_REGSZ - 1))

typedef struct pal_region_s {
    u_int64_t pa;
    u_int64_t sz;
    void *va;
} pal_region_t;

static u_int64_t
pr_align(const u_int64_t pa)
{
    return pa & PAL_REGBASE;
}

static u_int64_t
pr_span(const u_int64_t pa, const u_int64_t sz)
{
    const u_int64_t aligned_pa = pr_align(pa);
    const u_int64_t upper_pa = roundup(pa + sz, PAL_REGSZ);
    const u_int64_t span_sz = upper_pa - aligned_pa;
    return span_sz;
}

static void *
mapfd(int fd, const off_t off, const size_t sz)
{
    const int prot = PROT_READ | PROT_WRITE;
    const int mapflags = MAP_SHARED;
    void *va = mmap(NULL, sz, prot, mapflags, fd, off);
    assert(va != (void *)-1);
    return va;
}

#ifdef __aarch64__
static void *
pr_map(pal_region_t *pr)
{
    pal_data_t *pd = pal_get_data();
    pr->va = mapfd(pd->memfd, pr->pa, pr->sz);
    return pr->va;
}

#else

static void *
pr_map(pal_region_t *pr)
{
    const char *home = getenv("HOME");
    const int oflags = O_RDWR | O_CREAT;
    char path[128];
    char z = 0;
    int fd;
    struct stat sb;

    /* create file to simulate address region */
    snprintf(path, sizeof(path), "%s/.palreg-%08"PRIx64, home, pr->pa);
    fd = open(path, oflags, 0666);
    assert(fd >= 0);

    /* if new/short file, grow to required size for mmap */
    if (fstat(fd, &sb) < 0 || sb.st_size < pr->sz) {
        lseek(fd, pr->sz - 1, SEEK_SET);
        if (write(fd, &z, 1) != 1) assert(0);
    }

    /* map the entire file */
    pr->va = mapfd(fd, 0, pr->sz);
    (void)close(fd);
    return pr->va;
}
#endif

static pal_region_t *
pr_new(const u_int64_t pa, const u_int64_t sz)
{
    pal_data_t *pd = pal_get_data();
    pal_region_t *pr;

    pr = realloc(pd->regions, (pd->nregions + 1) * sizeof(pal_region_t));
    assert(pr);
    pd->regions = pr;
    pr = &pd->regions[pd->nregions++];
    memset(pr, 0, sizeof(*pr));
    pr->pa = pr_align(pa);
    pr->sz = pr_span(pa, sz);
    return pr;
}

static pal_region_t *
pr_findpa(const u_int64_t pa, const u_int64_t sz)
{
    pal_data_t *pd = pal_get_data();
    pal_region_t *pr = pd->regions;
    int i;

    for (i = 0; i < pd->nregions; i++, pr++) {
        if (pr->pa <= pa && pa + sz < pr->pa + pr->sz) {
            return pr;
        }
    }
    return NULL;
}

static pal_region_t *
pr_findva(const void *va, const u_int64_t sz)
{
    pal_data_t *pd = pal_get_data();
    pal_region_t *pr = pd->regions;
    int i;

    for (i = 0; i < pd->nregions; i++, pr++) {
        if (pr->va <= va && va + sz < pr->va + pr->sz) {
            return pr;
        }
    }
    return NULL;
}

pal_region_t *
pr_getpa(const u_int64_t pa, const u_int64_t sz)
{
    pal_region_t *pr = pr_findpa(pa, sz);
    if (pr == NULL) {
        pr = pr_new(pa, sz);
        pr_map(pr);
    }
    return pr;
}

void *
pr_ptov(const u_int64_t pa, const u_int64_t sz)
{
    pal_region_t *pr = pr_getpa(pa, sz);
    assert(pr != NULL);
    return pr->va + (pa - pr->pa);
}

u_int64_t
pr_vtop(const void *va, const u_int64_t sz)
{
    pal_region_t *pr = pr_findva(va, sz);
    assert(pr != NULL);
    return pr->pa + (va - pr->va);
}
