// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __ETH_COMMON_H__
#define __ETH_COMMON_H__

#include <stdint.h>

#pragma pack(push, 1)

typedef struct eth_qstate_intr {
    uint8_t     pc_offset;
    uint8_t     rsvd;
    uint8_t     cosA : 4;
    uint8_t     cosB : 4;
    uint8_t     cos_sel;
    uint8_t     eval_last;
    uint8_t     host : 4;
    uint8_t     total : 4;
    uint16_t    pid;
} eth_qstate_intr_t;

typedef struct eth_qstate_ring {
    uint16_t    p_index;
    uint16_t    c_index;
} eth_qstate_ring_t;

typedef struct eth_qstate_cfg {
    uint8_t     rsvd_cfg:2;
    uint8_t     intr_enable:1;
    uint8_t     eq_enable:1;
    uint8_t     cpu_queue:1;
    uint8_t     host_queue:1;
    uint8_t     debug:1;
    uint8_t     enable:1;
} eth_qstate_cfg_t;

typedef struct eth_qstate_common {
    eth_qstate_intr_t   intr;
    eth_qstate_ring_t   ring[3];
    eth_qstate_cfg_t    cfg;
    uint8_t             ring_size:5;
    uint8_t             rsvd_db_cnt:3;
    uint16_t            lif_index;
} eth_qstate_common_t;

typedef struct eth_tx_sta {
    uint8_t     rsvd:6;
    uint8_t     armed:1;
    uint8_t     color:1;
} eth_tx_sta_t;

typedef struct eth_tx_qstate {
    eth_qstate_common_t q;

    uint16_t    comp_index;

    eth_tx_sta_t sta;

    uint8_t     lg2_cq_desc_sz:4;
    uint8_t     lg2_desc_sz:4;

    uint8_t     __pad256_a:4;
    uint8_t     lg2_sg_desc_sz:4;

    uint8_t     __pad256_b[3];

    uint64_t    ring_base;
    uint64_t    cq_ring_base;
    uint64_t    sg_ring_base;
    uint64_t    intr_index_or_eq_addr;
} eth_tx_qstate_t;

static_assert (sizeof(struct eth_tx_qstate) == 64, "");

typedef struct eth_tx2_qstate {
    // BE bitfields not byte aligned, incompat with LE bitfields
    uint64_t    tso_state[2];
    uint8_t     __pad512[48];
} eth_tx2_qstate_t;

static_assert (sizeof(struct eth_tx2_qstate) == 64, "");

typedef struct eth_tx_co_qstate {
    eth_tx_qstate_t     tx;
    eth_tx2_qstate_t    tx2;
} eth_tx_co_qstate_t;

static_assert (sizeof(struct eth_tx_co_qstate) == 128, "");

typedef struct eth_rx_sta {
    uint8_t     rsvd:6;
    uint8_t     armed:1;
    uint8_t     color:1;
} eth_rx_sta_t;

typedef struct eth_rx_features {
    uint8_t     rsvd:7;
    uint8_t     encap_offload:1;
} eth_rx_features_t;

typedef struct eth_rx_qstate {
    eth_qstate_common_t q;

    uint16_t    comp_index;

    eth_rx_sta_t sta;

    uint8_t     lg2_cq_desc_sz : 4;
    uint8_t     lg2_desc_sz : 4;

    uint8_t     sg_max_elems : 4;
    uint8_t     lg2_sg_desc_sz : 4;

    eth_rx_features_t features;
    uint8_t     __pad256[2];

    uint64_t    ring_base;
    uint64_t    cq_ring_base;
    uint64_t    sg_ring_base;
    uint64_t    intr_index_or_eq_addr;
} eth_rx_qstate_t;

static_assert (sizeof(struct eth_rx_qstate) == 64, "");

typedef struct eth_eq_qstate_cfg {
    uint8_t     rsvd_cfg:6;
    uint8_t     intr_enable:1;
    uint8_t     eq_enable:1;
} eth_eq_qstate_cfg_t;

typedef struct eth_eq_qstate {
    uint64_t    eq_ring_base;
    uint8_t     eq_ring_size;

    eth_eq_qstate_cfg_t cfg;

    uint16_t    eq_index;
    uint8_t     eq_gen;
    uint8_t	rsvd;

    uint16_t    intr_index;
} eth_eq_qstate_t;

static_assert (sizeof(struct eth_eq_qstate) == 16, "");

#pragma pack(pop)

#endif      /* __ETH_COMMON_H__ */
