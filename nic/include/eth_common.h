// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __ETH_COMMON_H__
#define __ETH_COMMON_H__

#include <stdint.h>

#pragma pack(push, 1)

struct eth_tx_sta_qstate {
    uint8_t     rsvd1:2;
    uint8_t     spurious_db_cnt:4;
    uint8_t     spec_miss:1;
    uint8_t     color:1;
};

struct eth_tx_cfg_qstate {
    uint8_t     rsvd2:3;
    uint8_t     debug:1;
    uint8_t     intr_enable:1;
    uint8_t     cpu_queue:1;
    uint8_t     host_queue:1;
    uint8_t     enable:1;
};

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
    uint16_t    ci_miss;

    struct eth_tx_sta_qstate sta;

    struct eth_tx_cfg_qstate cfg;

    uint64_t    ring_base;
    uint8_t     ring_size;
    uint64_t    cq_ring_base;
    uint16_t    intr_assert_index;
    uint64_t    sg_ring_base;

    uint64_t    tso_hdr_addr:52;
    uint64_t    tso_hdr_len:10;
    uint64_t    rsvd3:2;
    uint64_t    tso_ipid_delta:16;
    uint64_t    tso_seq_delta:32;

    uint8_t     lg2_desc_sz : 4;
    uint8_t     lg2_cq_desc_sz : 4;
    uint8_t     rsvd4 : 4;
    uint8_t     lg2_sg_desc_sz : 4;
    uint8_t     rsvd5 : 8;

} eth_tx_qstate_t;

static_assert (sizeof(struct eth_tx_qstate) == 64, "");

struct eth_rx_sta_qstate {
    uint8_t     rsvd1:7;
    uint8_t     color:1;
};

struct eth_rx_cfg_qstate {
    uint8_t     rsvd2:3;
    uint8_t     debug:1;
    uint8_t     intr_enable:1;
    uint8_t     cpu_queue:1;
    uint8_t     host_queue:1;
    uint8_t     enable:1;
};

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

    struct eth_rx_sta_qstate sta;

    struct eth_rx_cfg_qstate cfg;

    uint64_t    ring_base;
    uint8_t     ring_size;
    uint64_t    cq_ring_base;
    uint16_t    intr_assert_index;
    uint64_t    sg_ring_base;

    uint8_t     lg2_desc_sz : 4;
    uint8_t     lg2_cq_desc_sz : 4;
    uint8_t     sg_max_elems : 4;
    uint8_t     lg2_sg_desc_sz : 4;

    uint8_t     __pad[19];

} eth_rx_qstate_t;

static_assert (sizeof(struct eth_rx_qstate) == 64, "");

#pragma pack(pop)

#endif      /* __ETH_COMMON_H__ */
