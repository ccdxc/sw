/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PRT_H__
#define __PRT_H__

#define PRT_BASE        \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_DHS_TGT_PRT_BYTE_ADDRESS)
#define PRT_COUNT       CAP_PXB_CSR_DHS_TGT_PRT_ENTRIES
#define PRT_NWORDS      3
#define PRT_STRIDE      CAP_PXB_CSR_DHS_TGT_PRT_ENTRY_BYTE_SIZE

/* defines for PRT.type */
#define PRT_TYPE_RES    0       /* Capri resource */
#define PRT_TYPE_DB64   1       /* 64-bit doorbells */
#define PRT_TYPE_DB32   2       /* 32-bit doorbells */
#define PRT_TYPE_DB16   3       /* 16-bit doorbells */

/* common prt entry format */
typedef struct {
    u_int64_t valid     :1;
    u_int64_t type      :2;
    u_int64_t indirect  :1;
    u_int64_t notify    :1;
} __attribute__((packed)) prt_cmn_t;

/* resource prt entry format */
typedef struct {
    /* u_int64_t [0] */
    u_int64_t valid     :1;
    u_int64_t type      :2;
    u_int64_t indirect  :1;
    u_int64_t notify    :1;
    u_int64_t vfstride  :5;
    u_int64_t aspace    :1;
    u_int64_t addrdw    :50;
    /* u_int64_t [1] sizedw_lo:3 */
    u_int64_t sizedw    :11;
    u_int64_t pmvdis    :1;
    u_int64_t spare     :3;
    u_int64_t rsrv      :52;
} __attribute__((packed)) prt_res_t;

/* db64/db32/db16 prt entry format */
typedef struct {
    /* u_int64_t [0] */
    u_int64_t valid     :1;
    u_int64_t type      :2;
    u_int64_t indirect  :1;
    u_int64_t notify    :1;
    u_int64_t vfstride  :5;
    u_int64_t lif       :11;
    u_int64_t updvec    :40;
    u_int64_t stridesel :2;
    /* u_int64_t [1] idxshift_lo:1 */
    u_int64_t idxshift  :2;
    u_int64_t idxwidth  :4;
    u_int64_t qidshift  :2;
    u_int64_t qidwidth  :4;
    u_int64_t qidsel    :1;
    u_int64_t rsrv      :52;
} __attribute__((packed)) prt_db_t;

typedef union {
    prt_cmn_t cmn;
    prt_res_t res;
    prt_db_t db;
    u_int32_t w[PRT_NWORDS];
} prt_t;

struct pciehw_s;
typedef struct pciehw_s pciehw_t;
struct pciehwdev_s;
typedef struct pciehwdev_s pciehwdev_t;
struct pciehwbar_s;
typedef struct pciehwbar_s pciehwbar_t;

void pciehw_prt_init(pciehw_t *phw);
int pciehw_prt_alloc(pciehwdev_t *phwdev, const pciehbar_t *bar);
void pciehw_prt_free(const int prtbase, const int prtcount);
int pciehw_prt_load(const int prtbase, const int prtcount);
void pciehw_prt_unload(const int prtbase, const int prtcount);
void pciehw_prt_dbg(int argc, char *argv[]);

#endif /* __PRT_H__ */
