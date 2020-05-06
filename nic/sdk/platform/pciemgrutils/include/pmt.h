/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#ifndef __PCIEHWUTIL_PMT_H__
#define __PCIEHWUTIL_PMT_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

/******************************************************************
 * PCIe Match Table (PMT)
 *
 * PMT entry is the tcam entry used to match the incoming PCIe TLP.
 * The corresponding PMR entry provides auxiliary information used
 * in processing the transaction after the PMT match determines the
 * entry that should be used for processing the TLP.
 */

#define PMT_NWORDS      5
#define PMR_NWORDS      4

/* defines for PMT.type and PMR.type fields */
#define PMT_TYPE_CFG    0       /* host cfg */
#define PMT_TYPE_MEM    1       /* host mem bar */
#define PMT_TYPE_RC     2       /* rc dma */
#define PMT_TYPE_IO     5       /* host I/O bar */

/* all PMTs start with these common fields */
#define PMT_CMN_FIELDS                                    \
    u_int64_t valid     :1;     /* entry is valid */      \
    u_int64_t tblid     :2;     /* table id */            \
    u_int64_t type      :3;     /* PMT_TYPE_* */          \
    u_int64_t port      :3;     /* incoming pcie port */  \
    u_int64_t rw        :1      /* 0=read, 1=write */

/* common pmt entry format */
typedef struct {
    PMT_CMN_FIELDS;
} __attribute__((packed)) pmt_cmn_format_t;

/* cfg pmt entry format */
typedef struct {
    PMT_CMN_FIELDS;
    u_int64_t bdf       :16;    /* bdf of tlp */
    u_int64_t addrdw    :10;    /* config space dw address */
    u_int64_t rsrv      :28;
} __attribute__((packed)) pmt_cfg_format_t;

/* bar pmt entry format */
typedef struct {
    PMT_CMN_FIELDS;
    u_int64_t addrdw    :50;    /* tlp address */
    u_int64_t rsrv      :4;
} __attribute__((packed)) pmt_bar_format_t;

/******************************************************************
 * PMR entry is the RAM extension of the corresponding PMT entry
 * containing auxiliary information used by hw after the PMT tcam
 * match is determined.
 */

/* cfg pmr entry format */
typedef struct {
    u_int64_t valid     :1;     /* entry is valid */
    u_int64_t type      :3;     /* matches PMT.type */
    u_int64_t vfbase    :11;    /* vf base for vf id range for entry */
    u_int64_t indirect  :1;     /* sw handles tlp */
    u_int64_t notify    :1;     /* notify sw */
    u_int64_t pstart    :3;     /* port     wildcard base */
    u_int64_t bstart    :8;     /* bus      wildcard base */
    u_int64_t dstart    :5;     /* device   wildcard base */
    u_int64_t fstart    :3;     /* function wildcard base */
    u_int64_t plimit    :3;     /* port     wildcard limit */
    u_int64_t blimit    :8;     /* bus      wildcard limit */
    u_int64_t dlimit    :5;     /* device   wildcard limit */
    u_int64_t flimit    :3;     /* function wildcard limit */
    u_int64_t vfstridesel:4;    /* p:bdf wildcard vf stride selector */
    u_int64_t td        :1;     /* tlp digest, generate ecrc on completion */
#if defined(ASIC_CAPRI)
    u_int64_t addrdw    :34;    /* target resource address */
#elif defined(ASIC_ELBA)
    u_int64_t addrdw    :35;    /* target resource address */
#else
#error "ASIC not specified"
#endif
    u_int64_t aspace    :1;     /* target address space, 1=external (pcie) */
    u_int64_t romsksel  :7;     /* read-only mask selector */
#if defined(ASIC_CAPRI)
    u_int64_t spare     :8;     /* implemented but unused in hw */
#elif defined(ASIC_ELBA)
    u_int64_t spare     :7;     /* implemented but unused in hw */
#else
#error "ASIC not specified"
#endif
    u_int64_t rsrv      :18;    /* unimplemented bits */
} __attribute__((packed)) pmr_cfg_entry_t;

/* bar pmr entry format */
typedef struct {
    u_int64_t valid     :1;     /* entry is valid */
    u_int64_t type      :3;     /* matches PMT.type */
    u_int64_t vfbase    :11;    /* vf base for vf ids valid for entry */
    u_int64_t indirect  :1;     /* sw handles tlp */
    u_int64_t notify    :1;     /* notify sw */
    u_int64_t prtb      :12;    /* base  of contiguous prt entries */
    u_int64_t prtc      :12;    /* count of contiguous prt entries */
    u_int64_t prtsize   :5;     /* power-of-2 resource size, eg. 4=16 bytes */
    u_int64_t vfstart   :6;     /* low  bit pos of vf field in addr */
    u_int64_t vfend     :6;     /* high bit pos of vf field in addr */
    u_int64_t vflimit   :11;    /* vf field upper limit */
    u_int64_t bdf       :16;    /* bdf for completions */
    u_int64_t td        :1;     /* tlp digest, generate ecrc on completion */
    u_int64_t pagesize  :3;     /* encoded page size, PID bit pos start */
    u_int64_t qtypestart:5;     /* low bit pos of 3-bit qtype */
    u_int64_t qtypemask :3;     /* qtype mask on 3 bits at qtypestart */
    u_int64_t qidstart  :5;     /* 32b db: low  bit pos of qid field in addr */
    u_int64_t qidend    :5;     /* 32b db: high bit pos of qid field in addr */
#if defined(ASIC_CAPRI)
    u_int64_t spare     :3;     /* implemented but unused in hw */
#elif defined(ASIC_ELBA)
    u_int64_t hstridesel:3;     /* host stride select */
#else
#error "ASIC not specified"
#endif
    u_int64_t rsrv      :18;    /* unimplemented bits */
} __attribute__((packed)) pmr_bar_entry_t;

/* all pmt formats */
typedef union {
    pmt_cmn_format_t cmn;
    pmt_cfg_format_t cfg;
    pmt_bar_format_t bar;
    u_int64_t all;
} pmt_format_t;

/* data and mask format used to describe pmt_tcam_t format */
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

/* tcam entry as words for reading/writing to hw */
typedef union {
    pmt_tcam_t tcam;
    u_int32_t w[PMT_NWORDS];
} pmt_entry_t;

/* PMR entry format */
typedef union {
    pmr_cfg_entry_t cfg;
    pmr_bar_entry_t bar;
    u_int32_t w[PMR_NWORDS];
} pmr_entry_t;

/* full PMT/PMR entry */
typedef struct pmt_s {
    pmt_entry_t pmte;
    pmr_entry_t pmre;
} pmt_t;

/* PMT flags for cfg/bar encoding */
#define PMTF_NONE       0x00     /* no flags */
#define PMTF_RD         0x01     /* read only */
#define PMTF_WR         0x02     /* write only, e.g db */
#define PMTF_RW         (PMTF_RD | PMTF_WR)
#define PMTF_NOTIFY     0x04     /* copy to notify ring */
#define PMTF_INDIRECT   0x08     /* handle by sw as indirect transaction */

void
pmt_cfg_enc(pmt_t *pmt,
            const u_int8_t port,
            const u_int16_t bdf,
            const u_int16_t bdfm,
            const u_int16_t bdfst,
            const u_int16_t bdflim,
            const u_int64_t cfgpa,
            const u_int16_t addr,
            const u_int16_t addrm,
            const u_int8_t romsksel,
            const u_int8_t vfstridesel,
            const u_int32_t pmtf);

void
pmt_bar_enc(pmt_t *pmt,
            const u_int8_t port,
            const u_int8_t type,
            const u_int64_t pmtsize,
            const u_int32_t prtsize,
            const u_int32_t pmtf);

const char *pmt_type_str(int type);
int pmt_is_valid(const pmt_t *pmt);

void pmt_entry_enc(pmt_entry_t *pmte, const pmt_datamask_t *dm);
void pmt_entry_dec(const pmt_entry_t *pmte, pmt_datamask_t *dm);

u_int32_t pmr_pagesize_enc(const u_int32_t pagesize);
u_int32_t pmr_pagesize_dec(const u_int32_t encoded_pagesize);

int pmt_allows_rd(const pmt_t *pmt);
int pmt_allows_wr(const pmt_t *pmt);

/* cfg operators */
void pmt_cfg_set_ports(pmt_t *pmt,
                       const u_int8_t port,
                       const u_int8_t portm,
                       const u_int8_t portstart,
                       const u_int8_t portlimit);
void pmt_cfg_set_bdfs(pmt_t *pmt,
                      const u_int16_t bdf,
                      const u_int16_t bdfm,
                      const u_int16_t bdfstart,
                      const u_int16_t bdflimit);

/* bar operators */
void pmt_bar_setaddr(pmt_t *pmt, const u_int64_t addr);
u_int64_t pmt_bar_getaddr(const pmt_t *pmt);
u_int64_t pmt_bar_getsize(const pmt_t *pmt);

void pmt_bar_set_bdf(pmt_t *pmt, const u_int16_t bdf);
void pmt_bar_set_prts(pmt_t *pmt, const u_int32_t prtb, const u_int32_t prtc);
void pmt_bar_set_qtype(pmt_t *pmt, const u_int8_t qtst, const u_int8_t qtmsk);
void pmt_bar_set_pagesize(pmt_t *pmt, const u_int32_t pagesize);
u_int32_t pmt_bar_get_pagesize(const pmt_t *pmt);
void pmt_bar_set_vfparams(pmt_t *pmt,
                          const int bitb, const int bitc,
                          const int vfb,  const int vfc);

void pmt_bar_setr_prt(pmt_t *pmt, const int bitb, const int bitc);
void pmt_bar_setr_lif(pmt_t *pmt, const int bitb, const int bitc);
void pmt_bar_setr_vf(pmt_t *pmt, const int bitb, const int bitc);
void pmt_bar_setr_qtype(pmt_t *pmt, const int bitb, const int bitc);
void pmt_bar_setr_qid(pmt_t *pmt, const int bitb, const int bitc);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHWUTIL_PMT_H__ */
