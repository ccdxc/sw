/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __TLPAUXINFO_H__
#define __TLPAUXINFO_H__

/*
 * For indirect or notify transactions, the hardware delivers
 * this auxiliary information along with the pcie tlp.
 */
typedef struct tlpauxinfo_s {
    /* u_int64_t [0] */
    u_int64_t direct_endaddr    :6;
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
    /* u_int64_t [1] direct_addr_lo:15 */
    u_int64_t direct_addr       :52;
    u_int64_t aspace            :1;
    u_int64_t pmti              :10;
    u_int64_t pmt_hit           :1;
    u_int64_t indirect_reason   :5;
    u_int64_t is_host           :1;
    u_int64_t axilen            :4;
    u_int64_t rsrv              :3;
    u_int64_t eop               :1;
    u_int64_t sop               :1;
} __attribute__((packed)) tlpauxinfo_t;

#endif /* __TLPAUXINFO_H__ */
