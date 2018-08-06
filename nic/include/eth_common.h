// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __ETH_COMMON_H__
#define __ETH_COMMON_H__

typedef struct eth_tx_qstate {
    uint8_t     pc_offset;
    uint8_t     rsvd0;
    uint8_t     cosA : 4;
    uint8_t     cosB : 4;
    uint8_t     cos_sel;
    uint8_t     eval_last;
    uint8_t     host : 4;
    uint8_t     total : 4;
    uint16_t    pid;

    uint16_t    p_index0;
    uint16_t    c_index0;
    uint16_t    comp_index;
    uint16_t    ci_fetch;

    uint8_t     enable:1;
    uint8_t     color:1;
    uint8_t     rsvd1:6;

    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;
    uint32_t    intr_assert_addr;
    uint8_t     spurious_db_cnt;
    uint64_t    sg_ring_base;

    uint8_t     __pad[16];

} __attribute__((packed)) eth_tx_qstate_t;

typedef struct eth_rx_qstate {
    uint8_t     pc_offset;
    uint8_t     rsvd0;
    uint8_t     cosA : 4;
    uint8_t     cosB : 4;
    uint8_t     cos_sel;
    uint8_t     eval_last;
    uint8_t     host : 4;
    uint8_t     total : 4;
    uint16_t    pid;

    uint16_t    p_index0;
    uint16_t    c_index0;
    uint16_t    comp_index;
    uint16_t    c_index1;

    uint8_t     enable:1;
    uint8_t     color:1;
    uint8_t     rsvd1:6;

    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;
    uint32_t    intr_assert_addr;
    uint16_t    rss_type;

    uint8_t     __pad[23];

} __attribute__((packed)) eth_rx_qstate_t;


typedef struct eth_admin_qstate {
    uint8_t     pc_offset;
    uint8_t     rsvd0;
    uint8_t     cosA : 4;
    uint8_t     cosB : 4;
    uint8_t     cos_sel;
    uint8_t     eval_last;
    uint8_t     host : 4;
    uint8_t     total : 4;
    uint16_t    pid;

    uint16_t    p_index0;
    uint16_t    c_index0;
    uint16_t    comp_index;
    uint16_t    ci_fetch;

    uint8_t     enable:1;
    uint8_t     color:1;
    uint8_t     rsvd1:6;

    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;
    uint32_t    intr_assert_addr;
    uint64_t    nicmgr_qstate_addr;

    uint8_t     __pad[17];

} __attribute__((packed)) eth_admin_qstate_t;


typedef struct eth_qstate {

    uint8_t     pc_offset;
    uint8_t     rsvd0;
    uint8_t     cosA : 4;
    uint8_t     cosB : 4;
    uint8_t     cos_sel;
    uint8_t     eval_last;
    uint8_t     host : 4;
    uint8_t     total : 4;
    uint16_t    pid;

    uint16_t    p_index0;
    uint16_t    c_index0;
    uint16_t    comp_index;
    uint16_t    c_index1;

    uint8_t     enable:1;
    uint8_t     color:1;
    uint8_t     rsvd1:6;

    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;
    uint32_t    intr_assert_addr;

    uint8_t     __pad[25];

} __attribute__((packed)) eth_qstate_t;


static_assert (sizeof(struct eth_rx_qstate) == 64, "");
static_assert (sizeof(struct eth_tx_qstate) == 64, "");
static_assert (sizeof(struct eth_admin_qstate) == 64, "");
static_assert (sizeof(struct eth_qstate) == 64, "");


#endif    // __ETH_COMMON_H__
