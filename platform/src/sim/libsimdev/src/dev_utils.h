/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __DEV_UTILS_H__
#define __DEV_UTILS_H__

typedef struct qstate {     // 64 B
    uint8_t     pc_offset;
    uint8_t     rsvd0;
    uint8_t     cosA : 4;
    uint8_t     cosB : 4;
    uint8_t     cos_sel;
    uint8_t     eval_last;
    uint8_t     host : 4;
    uint8_t     total : 4;
    uint16_t    pid;
    uint8_t     app_data[56];   /* application specific area */
} __attribute__((packed)) qstate_t;

u_int64_t lif_qstate_map_addr(const int lif);
u_int64_t intr_msixcfg_addr(const int intr);
u_int64_t intr_fwcfg_addr(const int intr);
u_int64_t intr_drvcfg_addr(const int intr);
u_int64_t intr_assert_addr(const int intr);
u_int64_t intr_pba_addr(const int lif);
u_int64_t intr_pba_cfg_addr(const int lif);
u_int64_t db_host_addr(const int lif);
void intr_pba_cfg(const int lif,
                  const int intr_start, const size_t intr_count);
void intr_drvcfg(const int intr);
void intr_drvcfg_unmasked(const int intr);
void intr_pba_clear(const int intr);
void intr_msixcfg(const int intr,
                  const u_int64_t msgaddr,
                  const u_int32_t msgdata,
                  const int vctrl);
void intr_fwcfg_msi(const int intr, const int lif, const int port_id);

int msixtbl_rd(const int intrb,
               const u_int64_t offset,
               const u_int8_t size,
               u_int64_t *valp);
int msixtbl_wr(const int intrb,
               const u_int64_t offset,
               const u_int8_t size,
               const u_int64_t val);
int msixpba_rd(const int lif,
               const u_int64_t offset,
               const u_int8_t size,
               u_int64_t *valp);
int msixpba_wr(const int lif,
               const u_int64_t offset,
               const u_int8_t size,
               const u_int64_t val);

#endif /* __DEV_UTILS_H__ */
