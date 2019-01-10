/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#ifndef __PCIEHBAR_H__
#define __PCIEHBAR_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

struct pciehbars_s; typedef struct pciehbars_s pciehbars_t;
union prt_u; typedef union prt_u prt_t;
struct pmt_s; typedef struct pmt_s pmt_t;

typedef struct pciehbarreg_s {
    u_int64_t baroff;           /* offset from beginning of bar */
    pmt_t pmt;                  /* pmt describing this region */
    u_int8_t nprts;             /* number of entries in prts[] */
    prt_t *prts;                /* prt table */
} pciehbarreg_t;

typedef enum pciehbartype_e {
    PCIEHBARTYPE_NONE,          /* invalid bar type */
    PCIEHBARTYPE_MEM,           /* 32-bit memory bar */
    PCIEHBARTYPE_MEM64,         /* 64-bit memory bar */
    PCIEHBARTYPE_IO,            /* 32-bit I/O bar */
} pciehbartype_t;

typedef struct pciehbar_s {
    pciehbartype_t type;        /* bar type (mem, mem64, io) */
    u_int64_t size;             /* bar size (power2) */
    u_int32_t prefetch:1;       /* prefetch enabled */
    u_int8_t cfgidx;            /* config bars index (0-5) */
    u_int8_t nregs;             /* number of entries in regs[] */
    pciehbarreg_t *regs;        /* barreg table */
} pciehbar_t;

void pciehbarreg_add_prt(pciehbarreg_t *preg, const prt_t *prt);
void pciehbar_add_reg(pciehbar_t *pbar, const pciehbarreg_t *preg);
void pciehbar_delete(pciehbar_t *pbar);

pciehbars_t *pciehbars_new(void);
void pciehbars_delete(pciehbars_t *pbars);
void pciehbars_add_bar(pciehbars_t *pbars, const pciehbar_t *pbar);
void pciehbars_add_rombar(pciehbars_t *pbars, const pciehbar_t *prombar);
void pciehbars_finalize(pciehbars_t *pbars);
void pciehbars_set_msix_tbl(pciehbars_t *pbars, const int bir, const int off);
u_int8_t  pciehbars_get_msix_tblbir(pciehbars_t *pbars);
u_int32_t pciehbars_get_msix_tbloff(pciehbars_t *pbars);
void pciehbars_set_msix_pba(pciehbars_t *pbars, const int bir, const int off);
u_int8_t  pciehbars_get_msix_pbabir(pciehbars_t *pbars);
u_int32_t pciehbars_get_msix_pbaoff(pciehbars_t *pbars);
pciehbar_t *pciehbars_get_first(pciehbars_t *pbars);
pciehbar_t *pciehbars_get_next(pciehbars_t *pbars, pciehbar_t *pbar);
pciehbar_t *pciehbars_get_rombar(pciehbars_t *pbars);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHBAR_H__ */
