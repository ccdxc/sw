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
mapfd(int fd, const off_t off, const size_t sz, pal_mmap_region_t *pr)
{
    const int prot = PROT_READ | PROT_WRITE;
    const int mapflags = MAP_SHARED;
    void *va = mmap(NULL, sz, prot, mapflags, fd, off);

    pr->mapped = 1;

    assert(va != (void *)-1);
    return va;
}

#ifdef __aarch64__
static void *
pr_map(pal_mmap_region_t *pr)
{
    u_int64_t cur_mapped = 0;
    u_int16_t i = ((pr->pa & 0xF0000000) >> 28);
    u_int16_t j = ((pr->pa & 0xFF00000) >> 20);
    pal_data_t *pd = pal_get_data();

    pr->va = mapfd(pd->memfd, pr->pa, pr->sz, pr);

    if(pr->sz > PAL_REGSZ) {
        while (cur_mapped < pr->sz) {
           pd->regions[i][j].pa = pr->pa;
           pd->regions[i][j].va = pr->va;
           pd->regions[i][j].sz = pr->sz;
           pd->regions[i][j].mapped = 1;

           j++;
           cur_mapped += PAL_REGSZ;

           if(j == 256) {
                j = 0;
                i++;
           }
        }
    }

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
    pr->va = mapfd(fd, 0, pr->sz, pr);
    (void)close(fd);
    return pr->va;
}
#endif

static pal_mmap_region_t *
pr_new(pal_mmap_region_t *pr, u_int64_t pa, const u_int64_t sz)
{
    assert(pr);
    memset(pr, 0, sizeof(*pr));
    
    pr->pa = pr_align(pa);
    pr->sz = pr_span(pa, sz);
    return pr;
}

static inline pal_mmap_region_t *
pr_findpa(const u_int64_t pa, const u_int64_t sz)
{
    pal_data_t *pd = pal_get_data();

   /*
    * regions[16][256]
    * regions[ (pa & 0xF0000000) >> 28 ][(pa & 0xFF00000) >> 20]
    */

    return &pd->regions[(pa & 0xF0000000) >> 28 ][(pa & 0xFF00000) >> 20];
}

static pal_mmap_region_t *
pr_findva(const void *va, const u_int64_t sz)
{
    pal_data_t *pd = pal_get_data(); 
    int i, j;
    /* TODO : Improve this - understandably it is slow */

    for (i = 0; i < 16; i++) {
        for (j = 0; j < 256; j++) {
            if (pd->regions[i][j].va <= va &&
                pd->regions[i][j].va + pd->regions[i][j].sz >= va + sz) {
                return &(pd->regions[i][j]);
            }  
        }
    }
    
    return NULL;
}

pal_mmap_region_t *
pr_getpa(const u_int64_t pa, const u_int64_t sz, u_int8_t access)
{
    pal_mmap_region_t *pr = pr_findpa(pa, sz);

    /* TODO : Add access control check here. */
    if (pr == NULL || (pr->pa + pr->sz) < (pa + sz)) {
        pr = pr_new(pr, pa, sz);
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
    u_int64_t cur_cleared = 0;
    pal_mmap_region_t *pr = pr_findva(va, 4);
    pal_data_t *pd = pal_get_data();
    u_int16_t i = ((pr->pa & 0xF0000000) >> 28);
    u_int16_t j = ((pr->pa & 0xFF00000) >> 20);

    munmap(pr->va, pr->sz);    

    if(pr->sz > PAL_REGSZ) {
        while (cur_cleared < pr->sz) {
           pd->regions[i][j].mapped = 0;

           j++;
           cur_cleared += PAL_REGSZ;

           if(j == 256) {
                j = 0;
                i++;
           }
        }
    }
}
