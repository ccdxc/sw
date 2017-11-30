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
} __attribute__((packed)) prt_common_t;

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
    u_int64_t sizedw_lo :3;
    /* u_int64_t [1] */
    u_int64_t sizedw_hi :8;
    u_int64_t pmvdis    :1;
    u_int64_t spare     :3;
    u_int64_t rsrv      :52;
} __attribute__((packed)) prt_res_t;

#define DBTYPE64        0x0
#define DBTYPE64        0x0
#define DBTYPE64        0x0
#define DBTYPE64        0x0

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
    u_int64_t idxshf_lo :1;
    /* u_int64_t [1] */
    u_int64_t idxshf_hi :1;
    u_int64_t idxwidth  :4;
    u_int64_t qidshift  :2;
    u_int64_t qidwidth  :4;
    u_int64_t qidsel    :1;
    u_int64_t rsrv      :52;
} __attribute__((packed)) prt_db_t;

typedef u_int32_t prt_t[PRT_NWORDS];

#endif /* __PRT_H__ */
