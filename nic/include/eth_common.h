
#ifndef __ETH_COMMON_H__
#define __ETH_COMMON_H__

struct eth_tx_qstate {
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
    uint16_t    spec_index;

    uint8_t     enable:1;
    uint8_t     color:1;
    uint8_t     rsvd1:6;

    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;
    uint32_t    intr_assert_addr;
    uint8_t     spurious_db_cnt;
    uint64_t    sg_ring_base;
} __attribute__((packed));

struct eth_rx_qstate {
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
    uint16_t    p_index1;
    uint16_t    c_index1;

    uint8_t     enable:1;
    uint8_t     color:1;
    uint8_t     rsvd1:6;

    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;
    uint32_t    intr_assert_addr;
    uint16_t    rss_type;
} __attribute__((packed));

struct eth_qstate {
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
    uint16_t    p_index1;
    uint16_t    c_index1;

    uint8_t     enable:1;
    uint8_t     color:1;
    uint8_t     rsvd1:6;

    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;
} __attribute__((packed));

#endif		/* __ETH_COMMON_H__ */
