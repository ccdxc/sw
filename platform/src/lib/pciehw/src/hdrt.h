/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __HDRT_H__
#define __HDRT_H__

#define HDRT_BASE       \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_DHS_ITR_PCIHDRT_BYTE_ADDRESS)
#define HDRT_COUNT      CAP_PXB_CSR_DHS_ITR_PCIHDRT_ENTRIES
#define HDRT_NWORDS     3
#define HDRT_STRIDE     CAP_PXB_CSR_DHS_ITR_PCIHDRT_ENTRY_BYTE_SIZE

typedef struct {
    /* u_int64_t [0] */
    u_int64_t valid     :1;
    u_int64_t bdf       :16;
    u_int64_t td        :1;
    u_int64_t pasid_en  :1;
    u_int64_t pasid_sel :11;
    u_int64_t pasid     :20;
    u_int64_t pasid_ex  :1;
    u_int64_t pasid_priv:1;
    u_int64_t tlpattr   :2;
    u_int64_t tph_wr_ph :2;
    u_int64_t tph_wr_en :1;
    u_int64_t tph_rd_ph :2;
    u_int64_t tph_rd_en :1;
    u_int64_t tph_st    :8;
    /* u_int64_t [1] msg_st_lo:5 */
    u_int64_t msi_st    :8;
    u_int64_t rc_cfg1   :1;
    u_int64_t rsrv      :60;
} __attribute__((packed)) hdrt_t;

void pciehw_hdrt_init(void);
int pciehw_hdrt_load(const u_int32_t lif, const u_int16_t bdf);
int pciehw_hdrt_unload(const u_int32_t lif);
void pciehw_hdrt_dbg(int argc, char *argv[]);

#endif /* __HDRT_H__ */
