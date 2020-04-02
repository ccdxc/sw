/*
 * Copyright (c) 2018, Pensando Systems Inc.
 *
 * internal.h : Add all helper/internal functions here.
 */

#ifndef __PAL_INTERNAL_H__
#define __PAL_INTERNAL_H__

#include <string.h>
#include "cap_top_csr_defines.h"
#include "cap_ms_c_hdr.h"

struct pal_mem_phys_range_s;

/* TODO: Find a better home for this */
#define MAXUUID 32
#define FREEACCESS 0x01
#define CONTROLLEDACCESS 0x00

typedef struct pal_mmap_region_s {
    u_int64_t pa;
    u_int64_t sz;
    u_int32_t mapped;
    void *va;
    struct pal_mmap_region_s *next;
    struct pal_mmap_region_s *prev;
} pal_mmap_region_t;

typedef struct {
    u_int32_t memopen:1;
    u_int32_t trace_init:1;
    u_int32_t reg_trace_en:1;
    u_int32_t mem_trace_en:1;
    u_int32_t init_done:1;
    int memfd_nonccoh; // "/dev/capmem" non cached/non coherent
    int memfd_ccoh; // "/dev/capmem" cached/coherent

    FILE *trfp;
    size_t nregions;
    char app_uuid[MAXUUID];

    pal_mmap_region_t *regions;

    // Dataplane memory
    struct pal_mem_phys_range_s *physmem;
    int nphysmem;
} pal_data_t;

pal_data_t *pal_get_data(void);
pal_mmap_region_t *pr_getpa(const u_int64_t pa, const u_int64_t sz, u_int8_t access);

void *pr_ptov(const u_int64_t pa, const u_int64_t sz, u_int8_t access);
u_int64_t pr_vtop(const void *va, const u_int64_t sz);
void pr_mem_unmap(void *va);

#endif /* __PAL_INTERNAL_H__ */

