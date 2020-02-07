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
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "pal.h"
#include "internal.h"

#define PAL_REGSZ       (1 << 20)
#define PAL_REGBASE     (~(PAL_REGSZ - 1))

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
pr_map(pal_mmap_region_t *pr)
{
    pal_data_t *pd = pal_get_data();

    /*
     * This is a temporary hack to map the non-cached/non-coherent region and the
     * cached/coherent regions usind different FDs opened on /dev/capmem, with and
     * without O_SYNC flag respectively. Eventually, the cache settings would be
     * derived by the kernel mapping passed on by u-boot memory map settings, so
     * we can map with a single FD. 
     */
    if (pr->pa >= 0xc0000000 && pr->pa < 0xc4000000) {
        pr->va = mapfd(pd->memfd_ccoh, pr->pa, pr->sz);
    } else {
        pr->va = mapfd(pd->memfd_nonccoh, pr->pa, pr->sz);
    }

    pr->mapped = 1;
    return pr->va;
}

#else

static void *
pr_map(pal_mmap_region_t *pr)
{
    const char *home = getenv("HOME");
    /* Make all accesses uncached by adding the O_SYNC flag */
    const int oflags = O_RDWR | O_CREAT | O_SYNC;
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
    pr->mapped = 1;
    (void)close(fd);
    return pr->va;
}
#endif

static pal_mmap_region_t *
pr_new(const u_int64_t pa, const u_int64_t sz)
{
    pal_data_t *pd = pal_get_data();
    pal_mmap_region_t *pr;

    pr = (pal_mmap_region_t*) malloc(sizeof(pal_mmap_region_t));
    assert(pr);
    memset(pr, 0, sizeof(*pr));
    pr->next = pd->regions;
    pr->prev = NULL;
    
    if (pd->regions) {
        pd->regions->prev = pr;
    }

    pd->regions = pr;
    pr->pa = pr_align(pa);
    pr->sz = pr_span(pa, sz);
    pd->nregions++;
    return pr;
}

static pal_mmap_region_t *
pr_findpa(const u_int64_t pa, const u_int64_t sz)
{
    pal_data_t *pd = pal_get_data();
    pal_mmap_region_t *pr = pd->regions;

    while(pr != NULL) {
        if (pr->pa <= pa && pa + sz < pr->pa + pr->sz && pr->mapped) {
            return pr;
        }
        pr = pr->next;
    }

    return NULL;
}

static pal_mmap_region_t *
pr_findva(const void *va, const u_int64_t sz)
{
    pal_data_t *pd = pal_get_data();
    pal_mmap_region_t *pr = pd->regions;

    while(pr != NULL) {
        if (pr->va <= va && va + sz < pr->va + pr->sz) {
            return pr;
        }
        pr = pr->next;
    }
    return NULL;
}

pal_mmap_region_t *
pr_getpa(const u_int64_t pa, const u_int64_t sz, u_int8_t access)
{
    pal_mmap_region_t *pr = pr_findpa(pa, sz);
    if (pr == NULL && access == FREEACCESS) {
        pr = pr_new(pa, sz);
            pr_map(pr);
    }

    return pr;
}

void *
pr_ptov(const u_int64_t pa, const u_int64_t sz, u_int8_t access)
{
    pal_mmap_region_t *pr = NULL;

    pr = pr_getpa(pa, sz, access);

    if (pr == NULL) {
        return NULL;
    } else {
        return pr->va + (pa - pr->pa);
    }
}

u_int64_t
pr_vtop(const void *va, const u_int64_t sz)
{
    pal_mmap_region_t *pr = pr_findva(va, sz);
    assert(pr != NULL);
    return pr->pa + (va - pr->va);
}

void
pr_mem_unmap(void *va)
{
    pal_data_t *pd = pal_get_data();
    pal_mmap_region_t *pr = pr_findva(va, 4);
    pr->mapped = 0;
    munmap(pr->va, pr->sz);    

    if (pr->next) {
        pr->next->prev = pr->prev;
    }

    if (pr->prev) {
        pr->prev->next = pr->next;
    }

    if (pd->regions == pr) {
        pd->regions = pr->next;
    }

    free(pr);
}

