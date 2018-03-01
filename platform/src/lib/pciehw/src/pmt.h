/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PMT_H__
#define __PMT_H__

#define PMT_BASE        \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_DHS_TGT_PMT_BYTE_ADDRESS)
#define PMT_COUNT_ASIC  CAP_PXB_CSR_DHS_TGT_PMT_ENTRIES
#define PMT_COUNT_FPGA  512
#define PMT_COUNT       PMT_COUNT_ASIC
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
#define PMT_TYPE_MEM    1       /* host mem bar */
#define PMT_TYPE_RC     2       /* rc dma */
#define PMT_TYPE_IO     5       /* host i/o bar */

/* common pmt entry format */
typedef struct {
    u_int64_t valid     :1;
    u_int64_t tblid     :2;
    u_int64_t type      :3;
    u_int64_t port      :3;
    u_int64_t rw        :1;
} __attribute__((packed)) pmt_cmn_format_t;

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
} __attribute__((packed)) pmt_cfg_format_t;

/* bar pmt entry format */
typedef struct {
    u_int64_t valid     :1;
    u_int64_t tblid     :2;
    u_int64_t type      :3;
    u_int64_t port      :3;
    u_int64_t rw        :1;
    u_int64_t addrdw    :50;
    u_int64_t rsrv      :4;
} __attribute__((packed)) pmt_bar_format_t;

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
    /* u_int64_t [1] addrdw_lo:4 */
    u_int64_t addrdw    :34;
    u_int64_t aspace    :1;
    u_int64_t romsksel  :7;
    u_int64_t spare     :8;
    u_int64_t rsrv      :18;
} __attribute__((packed)) pmr_cfg_entry_t;

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
    /* u_int64_t [1] vflimit_lo:6 */
    u_int64_t vflimit   :11;
    u_int64_t bdf       :16;
    u_int64_t td        :1;
    u_int64_t pagesize  :3;
    u_int64_t qtypestart:5;
    u_int64_t qtypemask :3;
    u_int64_t qidstart  :5;
    u_int64_t qidend    :5;
    u_int64_t spare     :3;
    u_int64_t rsrv      :18;
} __attribute__((packed)) pmr_bar_entry_t;

typedef union {
    pmt_cmn_format_t cmn;
    pmt_cfg_format_t cfg;
    pmt_bar_format_t bar;
    u_int64_t all;
} pmt_format_t;

typedef struct {
    pmt_format_t data;
    pmt_format_t mask;
} pmt_datamask_t;

/* tcam entry format */
typedef struct {
    u_int64_t x;                /* tcam x */
    u_int64_t y;                /* tcam y */
    u_int32_t v;                /* 1=entry valid */
} __attribute__((packed)) pmt_tcam_t;

typedef union {
    pmt_tcam_t tcam;
    u_int32_t w[PMT_NWORDS];
} pmt_entry_t;

typedef union {
    pmr_cfg_entry_t cfg;
    pmr_bar_entry_t bar;
    u_int32_t w[PMR_NWORDS];
} pmr_entry_t;

typedef struct {
    pmt_entry_t pmte;
    pmr_entry_t pmre;
} pmt_t;

struct pciehw_s;
typedef struct pciehw_s pciehw_t;
struct pciehwdev_s;
typedef struct pciehwdev_s pciehwdev_t;
struct pciehwbar_s;
typedef struct pciehwbar_s pciehwbar_t;

void pciehw_pmt_init(pciehw_t *phw);
int pciehw_pmt_load_cfg(pciehwdev_t *phwdev);
void pciehw_pmt_unload_cfg(pciehwdev_t *phwdev);
int pciehw_pmt_alloc_bar(pciehwdev_t *phwdev, const pciehbar_t *bar);
void pciehw_pmt_free_bar(const int pmti);
void pciehw_pmt_setaddr(pciehwbar_t *phwbar, const u_int64_t addr);
void pciehw_pmt_load_bar(pciehwbar_t *phwbar);
void pciehw_pmt_enable_bar(pciehwbar_t *phwbar, const int on);
void pciehw_pmt_dbg(int argc, char *argv[]);

#endif /* __PMT_H__ */
