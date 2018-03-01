/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHBAR_H__
#define __PCIEHBAR_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

struct pciehbars_s;
typedef struct pciehbars_s pciehbars_t;

typedef struct pciehbarreg_s {
    u_int16_t flags;                    /* PCIEHBARREGF_* flags below */
    u_int16_t regtype;                  /* PCIEHBARREGTYPE_* below */
    u_int64_t paddr;                    /* physical address */
    u_int32_t size;                     /* actual size of region */
    u_int32_t align;                    /* alignment of this region in bar */
    u_int32_t baroff;                   /* offset from beginning of bar */
    u_int8_t upd[8];                    /* db*: upd vector for qtypes */
    u_int8_t qtyshift;                  /* db*: qtype shift */
    u_int8_t qtywidth;                  /* db*: qtype width */
    u_int16_t npids;                    /* db64: number of PIDs */
    u_int16_t nlifs;                    /* db64: number of LIFs */
    u_int8_t idxshift;                  /* db32/16: shift data for index */
    u_int8_t idxwidth;                  /* db32/16: index width in data */
    u_int8_t qidshift;                  /* db32/16: shift data for qid */
    u_int8_t qidwidth;                  /* db32/16: qid width in data */
} pciehbarreg_t;

/* BAR region flags - flags */
#define PCIEHBARREGF_RD         0x0001  /* read permission */
#define PCIEHBARREGF_WR         0x0002  /* write permission */
#define PCIEHBARREGF_RW         (PCIEHBARREGF_RD | PCIEHBARREGF_WR)
#define PCIEHBARREGF_NOTIFYRD   0x0004  /* notify on read access */
#define PCIEHBARREGF_NOTIFYWR   0x0008  /* notify on write access */
#define PCIEHBARREGF_NOTIFYRW   (PCIEHBARREGF_NOTIFYRD | PCIEHBARREGF_NOTIFYWR)
#define PCIEHBARREGF_INDIRECTRD 0x0010  /* indirect on read access */
#define PCIEHBARREGF_INDIRECTWR 0x0020  /* indirect on write access */
#define PCIEHBARREGF_INDIRECTRW (PCIEHBARREGF_INDIRECTRD | \
                                 PCIEHBARREGF_INDIRECTWR)
#define PCIEHBARREGF_MSIX_TBL   0x0040  /* region contains msix table */
#define PCIEHBARREGF_MSIX_PBA   0x0080  /* region contains msix pba */
#define PCIEHBARREGF_DBQIDADDR  0x0100  /* qid source: address */
#define PCIEHBARREGF_MEM        0x0200  /* allow all size accesses */

/* BAR region type - regtype */
#define PCIEHBARREGT_RES        0       /* resource region */
#define PCIEHBARREGT_DB64       1       /* 64-bit doorbell region */
#define PCIEHBARREGT_DB32       2       /* 32-bit doorbell region */
#define PCIEHBARREGT_DB16       3       /* 16-bit doorbell region */

/* BAR region "update" functions - upd[] */
#define PCIEHBARUPD_NONE        0x00
#define PCIEHBARUPD_SCHED_EVAL  0x01    /* schedule if evaluate rings */
#define PCIEHBARUPD_SCHED_CLEAR 0x02    /* schedule always clear */
#define PCIEHBARUPD_SCHED_SET   0x04    /* schedule always set */
#define PCIEHBARUPD_SCHED_BITS  0x07
#define PCIEHBARUPD_PICI_CISET  0x08    /* set ci */
#define PCIEHBARUPD_PICI_PISET  0x10    /* set pi */
#define PCIEHBARUPD_PICI_PIINC  0x20    /* increment pi */
#define PCIEHBARUPD_PICI_BITS   0x38
#define PCIEHBARUPD_PID_CHECK   0x80    /* check pid */

typedef enum pciehbartype_e {
    PCIEHBARTYPE_NONE,          /* invalid bar type */
    PCIEHBARTYPE_MEM,           /* 32-bit memory bar */
    PCIEHBARTYPE_MEM64,         /* 64-bit memory bar */
    PCIEHBARTYPE_IO,            /* 32-bit I/O bar */
} pciehbartype_t;

typedef struct pciehbar_s {
    u_int32_t msix_tbl:1;       /* bar contains msix table */
    u_int32_t msix_pba:1;       /* bar contains msix pba */
    u_int32_t rombar:1;         /* bar is rombar */
    pciehbartype_t type;        /* bar type (mem, mem64, io) */
    u_int8_t cfgidx;            /* config bars index (0-5) */
    u_int32_t size;             /* bar size */
    u_int8_t nregs;             /* number of entries in regs[] */
    pciehbarreg_t *regs;        /* barreg table */
} pciehbar_t;

void pciehbar_add_reg(pciehbar_t *pbar, const pciehbarreg_t *preg);
void pciehbar_delete(pciehbar_t *pbar);

pciehbars_t *pciehbars_new(void);
void pciehbars_delete(pciehbars_t *pbars);
void pciehbars_add_bar(pciehbars_t *pbars, const pciehbar_t *pbar);
void pciehbars_finalize(pciehbars_t *pbars);
u_int8_t  pciehbars_get_msix_tblbir(pciehbars_t *pbars);
u_int32_t pciehbars_get_msix_tbloff(pciehbars_t *pbars);
u_int8_t  pciehbars_get_msix_pbabir(pciehbars_t *pbars);
u_int32_t pciehbars_get_msix_pbaoff(pciehbars_t *pbars);
pciehbar_t *pciehbars_get_first(pciehbars_t *pbars);
pciehbar_t *pciehbars_get_next(pciehbars_t *pbars, pciehbar_t *pbar);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHBAR_H__ */
