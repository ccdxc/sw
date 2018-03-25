/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PAL_IMPL_H__
#define __PAL_IMPL_H__

#include "cap_top_csr_defines.h"
#include "cap_ms_c_hdr.h"

struct pal_region_s;
typedef struct pal_region_s pal_region_t;

typedef struct {
    u_int32_t memopen:1;
    u_int32_t trace_init:1;
    u_int32_t reg_trace_en:1;
    u_int32_t mem_trace_en:1;
    int memfd;
    FILE *trfp;
    size_t nregions;
    pal_region_t *regions;
} pal_data_t;

pal_data_t *pal_get_data(void);
pal_region_t *pr_getpa(const u_int64_t pa, const u_int64_t sz);

void *pr_ptov(const u_int64_t pa, const u_int64_t sz);
u_int64_t pr_vtop(const void *va, const u_int64_t sz);

void pal_reg_trace(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));
void pal_mem_trace(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));

#endif /* __PAL_IMPL_H__ */
