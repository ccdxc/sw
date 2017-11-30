/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PMT_H__
#define __PMT_H__

#define PMT_BASE        \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_DHS_TGT_PMT_BYTE_ADDRESS)
#define PMT_COUNT       \
    CAP_PXB_CSR_DHS_TGT_PMT_ENTRIES
#define PMT_NWORDS      5
#define PMT_STRIDE      \
    (CAP_PXB_CSR_DHS_TGT_PMT_ENTRY_ARRAY_ELEMENT_SIZE * 4 * 8)
#define PMT_GRST   \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CFG_TGT_PMT_GRST_BYTE_ADDRESS)
#define PMR_BASE        \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_DHS_TGT_PMR_BYTE_ADDRESS)
#define PMR_COUNT       \
    CAP_PXB_CSR_DHS_TGT_PMR_ENTRIES
#define PMR_STRIDE      \
    CAP_PXB_CSR_DHS_TGT_PMR_ENTRY_BYTE_SIZE
#define PMR_NWORDS      \
    (CAP_PXB_CSR_DHS_TGT_PMR_ENTRY_BYTE_SIZE / sizeof(u_int32_t))

/* defines for PMT.type and PMR.type fields */
#define PMT_TYPE_CFG    0       /* host cfg */
#define PMT_TYPE_BAR    1       /* host mem/io bar */
#define PMT_TYPE_RC     2       /* rc dma */

/* generic tcam entry format */
typedef union {
    struct {
        u_int64_t x;            /* tcam x */
        u_int64_t y;            /* tcam y */
        u_int32_t v;            /* 1=entry valid */
    };
    u_int32_t words[PMT_NWORDS];
} __attribute__((packed)) pmt_tcam_entry_t;

/* common pmt entry format */
typedef struct {
    u_int64_t valid     :1;
    u_int64_t tblid     :2;
    u_int64_t type      :3;
    u_int64_t port      :3;
    u_int64_t rw        :1;
} __attribute__((packed)) pmt_common_t;

/* cfg pmt entry format */
typedef struct {
    u_int64_t valid     :1;
    u_int64_t tblid     :2;
    u_int64_t type      :3;
    u_int64_t port      :3;
    u_int64_t rw        :1;
    u_int64_t bdf       :16;
    u_int64_t addrdw    :10;
    u_int64_t rsrv      :28;
} __attribute__((packed)) pmt_cfg_t;

/* bar pmt entry format */
typedef struct {
    u_int64_t valid     :1;
    u_int64_t tblid     :2;
    u_int64_t type      :3;
    u_int64_t port      :3;
    u_int64_t rw        :1;
    u_int64_t addrdw    :50;
    u_int64_t rsrv      :4;
} __attribute__((packed)) pmt_bar_t;

/* common pmr entry format */
typedef struct {
    /* u_int64_t [0] */
    u_int64_t valid     :1;
    u_int64_t type      :3;
} __attribute__((packed)) pmr_common_t;

/* cfg pmr entry format */
typedef struct {
    /* u_int64_t [0] */
    u_int64_t valid     :1;
    u_int64_t type      :3;
    u_int64_t vfbase    :11;
    u_int64_t indirect  :1;
    u_int64_t notify    :1;
    u_int64_t pstart    :3;
    u_int64_t bstart    :8;
    u_int64_t dstart    :5;
    u_int64_t fstart    :3;
    u_int64_t plimit    :3;
    u_int64_t blimit    :8;
    u_int64_t dlimit    :5;
    u_int64_t flimit    :3;
    u_int64_t stridesel :4;
    u_int64_t td        :1;
    u_int64_t addrdw_lo :4;
    /* u_int64_t [1] */
    u_int64_t addrdw_hi :30;
    u_int64_t aspace    :1;
    u_int64_t romsksel  :7;
    u_int64_t spare     :8;
    u_int64_t rsrv      :18;
} __attribute__((packed)) pmr_cfg_t;

/* bar pmr entry format */
typedef struct {
    /* u_int64_t [0] */
    u_int64_t valid     :1;
    u_int64_t type      :3;
    u_int64_t vfbase    :11;
    u_int64_t indirect  :1;
    u_int64_t notify    :1;
    u_int64_t prtbase   :12;
    u_int64_t prtcount  :12;
    u_int64_t prtsize   :5;
    u_int64_t vfstart   :6;
    u_int64_t vfend     :6;
    u_int64_t vflimit_lo:6;
    /* u_int64_t [1] */
    u_int64_t vflimit_hi:5;
    u_int64_t bdf       :16;
    u_int64_t td        :1;
    u_int64_t pagesize  :3;
    u_int64_t qtypestart:5;
    u_int64_t qtypemask :3;
    u_int64_t qidstart  :5;
    u_int64_t qidend    :5;
    u_int64_t spare     :3;
    u_int64_t rsrv      :18;
} __attribute__((packed)) pmr_bar_t;

typedef u_int32_t pmr_t[PMR_NWORDS];

typedef struct {
    u_int32_t valid;
    u_int64_t data;
    u_int64_t mask;
    pmr_t pmr;
} pmt_t;

#endif /* __PMT_H__ */
