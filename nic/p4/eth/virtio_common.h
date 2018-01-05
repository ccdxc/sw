/*
 *  Common Virtio definitions
 */


/*  ------------------------------------------- */
/*  Virtio Specification related definitions    */
/*  ------------------------------------------- */

/* Common to both Legacy and v1.0 support */


/* Virtqueue descriptor */
#define VIRTQ_DESC_F_NEXT           1
#define VIRTQ_DESC_F_WRITE          2
#define VIRTQ_DESC_F_INDIRECT       4

header_type virtq_desc {
    fields {
        addr                        : 64;
        len                         : 32;
        flags                       : 16;
        nextidx                     : 16;
    }
}


/* Virtqueue available ring */
header_type virtq_desc_idx {
    fields {
        desc_idx                    : 16;
    }
}

#define VIRTQ_AVAIL_F_NO_INTERRUPT  1 
header_type virtq_avail {
    fields {
        flags                       : 16;
        idx                         : 16;
    }
}

/* Virtqueue used ring */
header_type virtq_used_elem {
    fields {
        id                          : 32;
        len                         : 32;
    }
}

#define VIRTQ_USED_F_NO_NOTIFY      1
header_type virtq_used {
    fields {
        flags                       : 16;
        idx                         : 16;
    }
}

header_type virtq_used_idx {
    fields {
        idx                     : 16;
    }
}

/* Virtio Net device */

#define VIRTIO_NET_HDR_F_NEEDS_CSUM 1

#define VIRTIO_NET_HDR_GSO_NONE     0
#define VIRTIO_NET_HDR_GSO_TCPV4    1
#define VIRTIO_NET_HDR_GSO_UDP      3
#define VIRTIO_NET_HDR_GSO_TCPV6    4
#define VIRTIO_NET_HDR_GSO_ECN      0x80

header_type virtio_net_hdr {
    fields {
        flags                       : 8;
        gso_type                    : 8;
        hdr_len                     : 16;
        gso_size                    : 16;
        csum_start                  : 16;
        csum_offset                 : 16;
        num_buffers                 : 16;
    }
}

/* Legacy Support */
 
/* When VIRTIO_NET_F_MRG_RXBUF is not negotiated */
header_type virtio_net_hdr_no_mrg_rxbuf {
    fields {
        flags                       : 8;
        gso_type                    : 8;
        hdr_len                     : 16;
        gso_size                    : 16;
        csum_start                  : 16;
        csum_offset                 : 16;
    }
}

/* v1.0 support */



/*  ------------------------------------------- */
/*  Virtio implementation related definitions  */
/*  ------------------------------------------- */


/* virtq_avail D */

#define VIRTQ_AVAIL_D_PARAMS                                                \
    flags, idx
#define VIRTQ_AVAIL_D_SCRATCH       virtq_avail_d
#define GENERATE_VIRTQ_AVAIL_D                                              \
    modify_field(VIRTQ_AVAIL_D_SCRATCH.flags, flags);                       \
    modify_field(VIRTQ_AVAIL_D_SCRATCH.idx, idx);    

/* virtq_desc_idx D */
#define VIRTQ_DESC_IDX_D_PARAMS                                             \
    desc_idx
#define VIRTQ_DESC_IDX_D_SCRATCH    virtq_desc_idx_d
#define GENERATE_VIRTQ_DESC_IDX_D                                           \
    modify_field(VIRTQ_DESC_IDX_D_SCRATCH.desc_idx, desc_idx);

/* virtq_desc D */
#define VIRTQ_DESC_D_PARAMS                                                 \
    addr, len, flags, nextidx
#define VIRTQ_DESC_D_SCRATCH        virtq_desc_d
#define GENERATE_VIRTQ_DESC_D                                               \
    modify_field(VIRTQ_DESC_D_SCRATCH.addr, addr);                          \
    modify_field(VIRTQ_DESC_D_SCRATCH.len, len);                            \
    modify_field(VIRTQ_DESC_D_SCRATCH.flags, flags);                        \
    modify_field(VIRTQ_DESC_D_SCRATCH.nextidx, nextidx);

/* virtio_net_hdr_no_mrg_rxbuf D */
#define VIRTIO_NET_HDR_NO_MRG_RXBUF_D_PARAMS                                \
    flags, gso_type, hdr_len, gso_size, csum_start, csum_offset
#define VIRTIO_NET_HDR_NO_MRG_RXBUF_D_SCRATCH   virtio_net_hdr_no_mrg_rxbuf_d
#define GENERATE_VIRTIO_NET_HDR_NO_MRG_RXBUF_D                              \
    modify_field(VIRTIO_NET_HDR_NO_MRG_RXBUF_D_SCRATCH.flags, flags);       \
    modify_field(VIRTIO_NET_HDR_NO_MRG_RXBUF_D_SCRATCH.gso_type, gso_type); \
    modify_field(VIRTIO_NET_HDR_NO_MRG_RXBUF_D_SCRATCH.hdr_len, hdr_len);   \
    modify_field(VIRTIO_NET_HDR_NO_MRG_RXBUF_D_SCRATCH.gso_size, gso_size); \
    modify_field(VIRTIO_NET_HDR_NO_MRG_RXBUF_D_SCRATCH.csum_start, csum_start); \
    modify_field(VIRTIO_NET_HDR_NO_MRG_RXBUF_D_SCRATCH.csum_offset, csum_offset);

/*
 * NOTE: The qstate for a LIF is seggeregated into TX and RX parts
 * driven individually via different QIDs assigned to each direction 
 */

/* TX support */

header_type virtio_tx_qstate {
    fields {
        pc                          : 8;
        // 7 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // Host notification ring
        CAPRI_QSTATE_HEADER_RING(0)
        CAPRI_QSTATE_HEADER_RING(1)
        /* Used: 128b */

        features                    : 64;

        tx_virtq_desc_addr          : 64;
        tx_virtq_avail_addr         : 64;
        tx_virtq_used_addr          : 64;
        tx_intr_assert_addr         : 32;
        tx_queue_size_mask          : 16; /* indicates the exponent */

        tx_virtq_avail_ci           : 16;
        tx_virtq_used_pi            : 16;
 
        /* Total used: 464b, pending: 48 */
    }
}

#define VIRTIO_TX_QSTATE_D_PARAMS                                           \
    rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0,     \
    pi_1, ci_1, features, tx_virtq_desc_addr, tx_virtq_avail_addr,          \
    tx_virtq_used_addr, tx_intr_assert_addr, tx_queue_size_mask,            \
    tx_virtq_avail_ci, tx_virtq_used_pi 

#define VIRTIO_TX_QSTATE_NON_STG0_D_PARAMS                                  \
    pc, VIRTIO_TX_QSTATE_D_PARAMS


#define VIRTIO_TX_QSTATE_D_SCRATCH  virtio_tx_qstate_d

#define GENERATE_VIRTIO_TX_QSTATE_D                                         \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.rsvd, rsvd);                    \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.cosA, cosA);                    \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.cosB, cosB);                    \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.cos_sel, cos_sel);              \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.eval_last, eval_last);          \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.host, host);                    \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.total, total);                  \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.pid, pid);                      \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.pi_0, pi_0);                    \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.ci_0, ci_0);                    \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.pi_1, pi_1);                    \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.ci_1, ci_1);                    \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.features, features);            \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.tx_virtq_desc_addr, tx_virtq_desc_addr);    \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.tx_virtq_avail_addr, tx_virtq_avail_addr);  \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.tx_virtq_used_addr, tx_virtq_used_addr);    \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.tx_intr_assert_addr, tx_intr_assert_addr);  \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.tx_queue_size_mask, tx_queue_size_mask);    \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.tx_virtq_avail_ci, tx_virtq_avail_ci);      \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.tx_virtq_used_pi, tx_virtq_used_pi);

#define GENERATE_VIRTIO_TX_QSTATE_NON_STG0_D                                \
    modify_field(VIRTIO_TX_QSTATE_D_SCRATCH.pc, pc);                        \
    GENERATE_VIRTIO_TX_QSTATE_D
    

/* RX support */

header_type virtio_rx_qstate {
    fields {
        pc                          : 8;
        // 7 Bytes intrinsic header
        CAPRI_QSTATE_HEADER_COMMON
        // FIXME: Remove the ring for RX
        // Host notification ring
        CAPRI_QSTATE_HEADER_RING(0)
        /* Used: 96b */

        features                    : 64;

        rx_virtq_desc_addr          : 64;
        rx_virtq_avail_addr         : 64;
        rx_virtq_used_addr          : 64;
        rx_intr_assert_addr         : 32;
        rx_queue_size_mask          : 16;

        rx_virtq_avail_ci           : 16;
        rx_virtq_used_pi            : 16;

        /* Total used: 432b, pending: 80b */
    }
}

#define VIRTIO_RX_QSTATE_D_PARAMS                                           \
    rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0, ci_0,     \
    features, rx_virtq_desc_addr, rx_virtq_avail_addr, rx_virtq_used_addr,  \
    rx_intr_assert_addr, rx_queue_size_mask, rx_virtq_avail_ci, rx_virtq_used_pi 

#define VIRTIO_RX_QSTATE_NON_STG0_D_PARAMS                                  \
    pc, VIRTIO_RX_QSTATE_D_PARAMS

#define VIRTIO_RX_QSTATE_D_SCRATCH  virtio_rx_qstate_d

#define GENERATE_VIRTIO_RX_QSTATE_D                                         \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.rsvd, rsvd);                    \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.cosA, cosA);                    \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.cosB, cosB);                    \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.cos_sel, cos_sel);              \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.eval_last, eval_last);          \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.host, host);                    \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.total, total);                  \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.pid, pid);                      \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.pi_0, pi_0);                    \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.ci_0, ci_0);                    \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.features, features);            \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.rx_virtq_desc_addr, rx_virtq_desc_addr);    \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.rx_virtq_avail_addr, rx_virtq_avail_addr);  \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.rx_virtq_used_addr, rx_virtq_used_addr);    \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.rx_intr_assert_addr, rx_intr_assert_addr);  \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.rx_queue_size_mask, rx_queue_size_mask);    \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.rx_virtq_avail_ci, rx_virtq_avail_ci);      \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.rx_virtq_used_pi, rx_virtq_used_pi);

#define GENERATE_VIRTIO_RX_QSTATE_NON_STG0_D                                \
    modify_field(VIRTIO_RX_QSTATE_D_SCRATCH.pc, pc);                        \
    GENERATE_VIRTIO_RX_QSTATE_D




