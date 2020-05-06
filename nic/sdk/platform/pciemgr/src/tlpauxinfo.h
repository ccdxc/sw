/*
 * Copyright (c) 2018,2020, Pensando Systems Inc.
 */

#ifndef __TLPAUXINFO_H__
#define __TLPAUXINFO_H__

/*
 * For indirect or notify transactions, the hardware delivers
 * this auxiliary information along with the pcie tlp.
 */
typedef struct tlpauxinfo_s {
#if defined(ASIC_CAPRI)
    u_int64_t direct_endaddr    :6;
#elif defined(ASIC_ELBA)
    u_int64_t spare             :3;
    u_int64_t wqebpdbxen        :1;
    u_int64_t wqebpsize         :2;
#else
#error "ASIC not specified"
#endif
    u_int64_t direct_blen       :4;
    u_int64_t is_indirect       :1;
    u_int64_t is_direct         :1;
    u_int64_t is_ur             :1;
    u_int64_t is_ca             :1;
    u_int64_t romsksel          :7;
    u_int64_t context_id        :7;
    u_int64_t vfid              :11;
    u_int64_t is_notify         :1;
    u_int64_t direct_size       :9;
    u_int64_t direct_addr       :52;
    u_int64_t aspace            :1;
    u_int64_t pmti              :10;
    u_int64_t pmt_hit           :1;
    u_int64_t indirect_reason   :5;
    u_int64_t is_host           :1;
    u_int64_t axilen            :4;
#if defined(ASIC_CAPRI)
    u_int64_t rsrv              :3;
#elif defined(ASIC_ELBA)
    u_int64_t rsrv              :1;
    u_int64_t wqetype           :1;     /* wqe type, 0=wqe, 1=doorbell */
    u_int64_t wqebpdben         :1;     /* wqe bypass doorbell enable */
#else
#error "ASIC not specified"
#endif
    u_int64_t eop               :1;
    u_int64_t sop               :1;
} __attribute__((packed)) tlpauxinfo_t;

#endif /* __TLPAUXINFO_H__ */
