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

#include "pal.h"
#include "pal_impl.h"

#define PAL_REGSZ       (1 << 20)
#define PAL_REGBASE     (~(PAL_REGSZ - 1))

typedef struct pal_region_s {
    u_int64_t pa;
    u_int64_t sz;
    void *va;
    u_int64_t mappa;
    u_int64_t mapsz;
    void *mapva;
} pal_region_t;

static u_int64_t
pr_base(const u_int64_t pa)
{
    return pa & PAL_REGBASE;
}

static void *
pr_mapfd(int fd, pal_region_t *pr)
{
    const int prot = PROT_READ | PROT_WRITE;
    const int mapflags = MAP_SHARED;
    void *va = mmap(NULL, pr->mapsz, prot, mapflags, fd, pr->mappa);
    assert(va != (void *)-1);
    return va;
}

#ifdef __aarch64__
void *
pr_map(pal_region_t *pr)
{
    pal_data_t *pd = pal_get_data();
    const int pagesize = getpagesize();

    /* map page aligned region */
    pr->mappa = pr->pa & ~(pagesize - 1);
    pr->mapsz = roundup(pr->pa + pr->sz, pagesize) - pr->mappa;
    pr->mapva = pr_mapfd(pd->memfd, pr);
    pr->va = pr->mapva + (pr->pa - pr->mappa);
    return pr->va;
}

#else

void *
pr_map(pal_region_t *pr)
{
    const char *home = getenv("HOME");
    const int oflags = O_RDWR | O_CREAT;
    char path[128];
    char z = 0;
    int fd;

    snprintf(path, sizeof(path), "%s/.palreg-%08"PRIx64, home, pr->pa);
    fd = open(path, oflags, 0666);
    assert(fd >= 0);
    lseek(fd, pr->sz - 1, SEEK_SET);
    if (write(fd, &z, 1) != 1) assert(0);

    /* map the entire file */
    pr->mappa = 0;
    pr->mapsz = pr->sz;
    pr->mapva = pr_mapfd(fd, pr);
    (void)close(fd);
    pr->va = pr->mapva;
    return pr->va;
}
#endif

pal_region_t *
pr_new(const u_int64_t pa, const u_int64_t sz)
{
    pal_data_t *pd = pal_get_data();
    pal_region_t *pr;

    pr = realloc(pd->regions, (pd->nregions + 1) * sizeof(pal_region_t));
    assert(pr);
    pd->regions = pr;
    pr = &pd->regions[pd->nregions++];
    memset(pr, 0, sizeof(*pr));
    pr->pa = pa;
    pr->sz = sz;
    return pr;
}

static pal_region_t *
pr_findpa(const u_int64_t pa)
{
    pal_data_t *pd = pal_get_data();
    pal_region_t *pr = pd->regions;
    int i;

    for (i = 0; i < pd->nregions; i++, pr++) {
        if (pr->pa <= pa && pa < pr->pa + pr->sz) {
            return pr;
        }
    }
    return NULL;
}

static pal_region_t *
pr_findva(const void *va)
{
    pal_data_t *pd = pal_get_data();
    pal_region_t *pr = pd->regions;
    int i;

    for (i = 0; i < pd->nregions; i++, pr++) {
        if (pr->va <= va && va < pr->va + pr->sz) {
            return pr;
        }
    }
    return NULL;
}

static pal_region_t *
pr_getpa(const u_int64_t pa)
{
    pal_region_t *pr = pr_findpa(pa);
    if (pr == NULL) {
        const u_int64_t basepa = pr_base(pa);
        pr = pr_new(basepa, PAL_REGSZ);
        pr_map(pr);
    }
    return pr;
}

void *
pr_ptov(const u_int64_t pa)
{
    pal_region_t *pr = pr_getpa(pa);
    return pr->va + (pa - pr->pa);
}

u_int64_t
pr_vtop(const void *va)
{
    pal_region_t *pr = pr_findva(va);
    assert(pr != NULL);
    return pr->pa + (va - pr->va);
}
