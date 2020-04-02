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
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/mman.h>

#include "pal.h"
#include "mm_int.h"
#include "internal.h"
#include "capmem_dev.h"

static pal_data_t pal_data;

void pal_init(char *application_name) {
   pal_mm_init(application_name);
}

#ifdef __aarch64__
static int
cmp_physmemrange(const void *p1, const void *p2)
{
    const pal_mem_phys_range_t *r1 = p1;
    const pal_mem_phys_range_t *r2 = p2;
    if (r1->pa == r2->pa) {
        return 0;
    } else {
        return (r1->pa < r2->pa) ? -1 : 1;
    }
}

static void
physmem_map_init(pal_data_t *pd)
{
    static const char path[] = CAPMEM_DEV;
    struct capmem_ranges_args args;
    int nranges, i, pass, j;
    struct capmem_range *r;

    /*
     * We'll enable both cached/coherent and non-cached/non-coherent accesses
     * using two different fd's with /dev/capmem.
     */
    pd->memfd_nonccoh = open(path, O_RDWR | O_SYNC);
    assert(pd->memfd_nonccoh >= 0);
    pd->memfd_ccoh = open(path, O_RDWR);
    assert(pd->memfd_ccoh >= 0);

    /*
     * Extract the capmem memory regions.
     */
    if (ioctl(pd->memfd_ccoh, CAPMEM_GET_NRANGES, &nranges) < 0) {
        perror("CAPMEM_GET_NRANGES");
        return;
    }
    if (nranges < 0 || nranges > CAPMEM_MAX_RANGES) {
        fprintf(stderr, "CAPMEM_GET_NRANGES: Invalid nranges %d\n", nranges);
        return;
    }
    if (nranges == 0) {
        return;
    }
    r = calloc(nranges, sizeof (*r));
    assert(r);
    args.range = r;
    args.nranges = nranges;
    if (ioctl(pd->memfd_ccoh, CAPMEM_GET_RANGES, &args) < 0) {
        perror("CAPMEM_GET_RANGES");
        free(r);
        return;
    }
    
    /*
     * Save the dataplane memory ranges.
     */
    for (pass = 1; pass <= 2; pass++) {
        j = 0;
        for (i = 0; i < nranges; i++) {
            // Ignore non-RAM and the copyin/copyout 64kB reservation
            if (r[i].start < 0x80000000 || r[i].len == 65536) {
                continue;
            }
            if (pass == 2) {
                pd->physmem[j].pa = r[i].start;
                pd->physmem[j].sz = r[i].len;
                if (r[i].type == CAPMEM_TYPE_COHERENT) {
                    pd->physmem[j].flags |= PAL_MEM_PHYS_COHERENT;
                }
            }
            ++j;
        }
        if (pass == 1) {
            if (j == 0) {
                break;
            }
            pd->nphysmem = j;
            pd->physmem = calloc(j, sizeof (pd->physmem[0]));
            assert(pd->physmem);
        }
    }
    free(r);

    // Sort and pack the ranges.
    qsort(pd->physmem, pd->nphysmem, sizeof (pd->physmem[0]), cmp_physmemrange);
    i = 0;
    for (j = 1; j < pd->nphysmem; j++) {
        if (pd->physmem[j].flags == pd->physmem[i].flags &&
            pd->physmem[j].pa == pd->physmem[i].pa + pd->physmem[i].sz) {
            pd->physmem[i].sz += pd->physmem[j].sz;
        } else if (++i != j) {
            pd->physmem[i] = pd->physmem[j];
        }
    }
    pd->nphysmem = i + 1;
}
#else
static void
physmem_map_init(pal_data_t *pd)
{
    // 6GB simulated
    static pal_mem_phys_range_t ranges[] = {
        { 0x0c0000000ULL, 0x004000000ULL, PAL_MEM_PHYS_COHERENT },
        { 0x0c4000000ULL, 0x17c000000ULL, 0 }
    };
    pd->nphysmem = 2;
    pd->physmem = ranges;
}
#endif

pal_data_t *
pal_get_data(void)
{
    pal_data_t *pd = &pal_data;
    if (!pd->memopen) {
        physmem_map_init(pd);
	pd->regions = NULL;
        pd->memopen = 1;
    }
    return pd;
}

int
pal_get_env(void)
{
#ifdef __aarch64__
#define MS_STA_VER \
    (CAP_ADDR_BASE_MS_MS_OFFSET + CAP_MS_CSR_STA_VER_BYTE_ADDRESS)

    union {
        struct {
            u_int32_t chip_type:4;
            u_int32_t chip_version:12;
            u_int32_t chip_build:16;
        } __attribute__((packed));
        u_int32_t w;
    } reg;

    reg.w = pal_reg_rd32(MS_STA_VER);
    return reg.chip_type;
#else
    const char *env = getenv("PAL_ENV");
    if (env) {
        return strtoul(env, NULL, 0);
    }
    return PAL_ENV_HAPS; /* for now x86_64 emulates HAPS */
#endif
}

int
pal_is_asic(void)
{
    return pal_get_env() == PAL_ENV_ASIC;
}

