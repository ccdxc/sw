#include "nic/p4/common/defines.h"

/*****************************************************************************/
/* IP types                                                                  */
/*****************************************************************************/
#define IPTYPE_IPV4                     0
#define IPTYPE_IPV6                     1

/*****************************************************************************/
/* Key types                                                                 */
/*****************************************************************************/
#define KEY_TYPE_NONE                   0
#define KEY_TYPE_IPV4                   1
#define KEY_TYPE_IPV6                   2
#define KEY_TYPE_MAC                    3

/*****************************************************************************/
/* Lifs                                                                      */
/*****************************************************************************/
#define ARTEMIS_SERVICE_LIF             1020

/*****************************************************************************/
/* Packet direction                                                          */
/*****************************************************************************/
#define RX_FROM_SWITCH                  0
#define TX_FROM_HOST                    1

/*****************************************************************************/
/* drop reasons - these are bit positions to be used in ASM                  */
/*****************************************************************************/
#define P4I_DROP_NACL                   1
#define P4I_DROP_REASON_MIN             0
#define P4I_DROP_REASON_MAX             0

#define P4E_DROP_FLOW_HIT               1
#define P4E_DROP_REASON_MIN             0
#define P4E_DROP_REASON_MAX             0

/*****************************************************************************/
/* flow role                                                                 */
/*****************************************************************************/
#define TCP_FLOW_INITIATOR              0
#define TCP_FLOW_RESPONDER              1

/*****************************************************************************/
/* route result type and bit position                                        */
/*****************************************************************************/
#define ROUTE_RESULT_TYPE_NEXTHOP       0
#define ROUTE_RESULT_TYPE_VPC           1
#define ROUTE_RESULT_BIT_POS            15

/*****************************************************************************/
/* Next pipe id                                                              */
/*****************************************************************************/
#define PIPE_EGRESS                     0
#define PIPE_CPS                        1
#define PIPE_CLASSIC_NIC                2
#define PIPE_ARM                        3

/*****************************************************************************/
/* Rewrite flags                                                             */
/*****************************************************************************/
#define TX_REWRITE_DMAC_SHIFT                   0
#define TX_REWRITE_DMAC_MASK                    1
#define TX_REWRITE_DMAC_NONE                    0
#define TX_REWRITE_DMAC_FROM_NEXTHOP            1

#define TX_REWRITE_SRC_IP_SHIFT                 1
#define TX_REWRITE_SRC_IP_MASK                  3
#define TX_REWRITE_SRC_IP_NONE                  0
#define TX_REWRITE_SRC_IP_FROM_PUBLIC           1
#define TX_REWRITE_SRC_IP_FROM_SERVICE          2
#define TX_REWRITE_SRC_IP_FROM_46               3

#define TX_REWRITE_DPORT_SHIFT                  3
#define TX_REWRITE_DPORT_MASK                   1
#define TX_REWRITE_DPORT_NONE                   0
#define TX_REWRITE_DPORT_FROM_SESSION           1

#define TX_REWRITE_DST_IP_SHIFT                 4
#define TX_REWRITE_DST_IP_MASK                  1
#define TX_REWRITE_DST_IP_NONE                  0
#define TX_REWRITE_DST_IP_FROM_SESSION          1

#define TX_REWRITE_ENCAP_SHIFT                  5
#define TX_REWRITE_ENCAP_MASK                   1
#define TX_REWRITE_ENCAP_NONE                   0
#define TX_REWRITE_ENCAP_VXLAN                  1

#define RX_REWRITE_SMAC_SHIFT                   0
#define RX_REWRITE_SMAC_MASK                    1
#define RX_REWRITE_SMAC_NONE                    0
#define RX_REWRITE_SMAC_FROM_VRMAC              1

#define RX_REWRITE_SRC_IP_SHIFT                 1
#define RX_REWRITE_SRC_IP_MASK                  3
#define RX_REWRITE_SRC_IP_NONE                  0
#define RX_REWRITE_SRC_IP_FROM_SESSION          1
#define RX_REWRITE_SRC_IP_FROM_64               2

#define RX_REWRITE_SPORT_SHIFT                  3
#define RX_REWRITE_SPORT_MASK                   1
#define RX_REWRITE_SPORT_NONE                   0
#define RX_REWRITE_SPORT_FROM_SESSION           1

#define RX_REWRITE_DST_IP_SHIFT                 4
#define RX_REWRITE_DST_IP_MASK                  3
#define RX_REWRITE_DST_IP_NONE                  0
#define RX_REWRITE_DST_IP_FROM_CA               1
#define RX_REWRITE_DST_IP_FROM_SERVICE          2
#define RX_REWRITE_DST_IP_FROM_64               3

#define TX_REWRITE(a, attr, val) \
    ((((a) >> TX_REWRITE_ ## attr ## _SHIFT) & TX_REWRITE_ ## attr ## _MASK) == TX_REWRITE_ ## attr ## _ ## val)

#define RX_REWRITE(a, attr, val) \
    ((((a) >> RX_REWRITE_ ## attr ## _SHIFT) & RX_REWRITE_ ## attr ## _MASK) == RX_REWRITE_ ## attr ## _ ## val)

/*****************************************************************************/
/* cpu flags and bit positions                                               */
/*****************************************************************************/
#define ARTEMIS_CPU_FLAGS_VLAN_VALID_BIT_POS    1
#define ARTEMIS_CPU_FLAGS_IPV4_1_VALID_BIT_POS  2
#define ARTEMIS_CPU_FLAGS_IPV6_1_VALID_BIT_POS  3
#define ARTEMIS_CPU_FLAGS_ETH_2_VALID_BIT_POS   4
#define ARTEMIS_CPU_FLAGS_IPV4_2_VALID_BIT_POS  5
#define ARTEMIS_CPU_FLAGS_IPV6_2_VALID_BIT_POS  6
#define ARTEMIS_CPU_FLAGS_DIRECTION_BIT_POS     7

#define ARTEMIS_CPU_FLAGS_VLAN_VALID    (1 << ARTEMIS_CPU_FLAGS_VLAN_VALID_BIT_POS)
#define ARTEMIS_CPU_FLAGS_IPV4_1_VALID  (1 << ARTEMIS_CPU_FLAGS_IPV4_1_VALID_BIT_POS)
#define ARTEMIS_CPU_FLAGS_IPV6_1_VALID  (1 << ARTEMIS_CPU_FLAGS_IPV6_1_VALID_BIT_POS)
#define ARTEMIS_CPU_FLAGS_ETH_2_VALID   (1 << ARTEMIS_CPU_FLAGS_ETH_2_VALID_BIT_POS)
#define ARTEMIS_CPU_FLAGS_IPV4_2_VALID  (1 << ARTEMIS_CPU_FLAGS_IPV4_2_VALID_BIT_POS)
#define ARTEMIS_CPU_FLAGS_IPV6_2_VALID  (1 << ARTEMIS_CPU_FLAGS_IPV6_2_VALID_BIT_POS)
#define ARTEMIS_CPU_FLAGS_DIRECTION     (1 << ARTEMIS_CPU_FLAGS_DIRECTION_BIT_POS)

/*****************************************************************************/
/* Header sizes                                                              */
/*****************************************************************************/
#define ARTEMIS_PREDICATE_HDR_SZ        0
#define ARTEMIS_P4_TO_RXDMA_HDR_SZ      0
#define ARTEMIS_P4_TO_ARM_HDR_SZ        0
#define ARTEMIS_P4_TO_TXDMA_HDR_SZ      0
#define ARTEMIS_I2E_HDR_SZ              0

#define ARTEMIS_INGRESS_MIRROR_BLOB_SZ  (CAPRI_RXDMA_INTRINSIC_HDR_SZ - \
                                         CAPRI_P4_INTRINSIC_HDR_SZ + \
                                         ARTEMIS_P4_TO_RXDMA_HDR_SZ + \
                                         ARTEMIS_PREDICATE_HDR_SZ + \
                                         ARTEMIS_PREDICATE_HDR_SZ + \
                                         ARTEMIS_P4_TO_TXDMA_HDR_SZ + \
                                         ARTEMIS_I2E_HDR_SZ)

#define PKTQ_PAGE_SIZE                  10240

/* Qstate definition for packet Q - RxDMA to TxDMA */
#define PKTQ_QSTATE \
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host_rings, total_rings, pid, \
    p_index0, c_index0, sw_pindex0, sw_cindex0, ring0_base, ring1_base,     \
    ring_size, rxdma_cindex_addr

#define PKTQ_QSTATE_DVEC_SCRATCH(_scratch_qstate_hdr, _scratch_qstate_txdma_q) \
    modify_field(_scratch_qstate_hdr.pc, pc);                                  \
    modify_field(_scratch_qstate_hdr.rsvd, rsvd);                              \
    modify_field(_scratch_qstate_hdr.cosA, cosA);                              \
    modify_field(_scratch_qstate_hdr.cosB, cosB);                              \
    modify_field(_scratch_qstate_hdr.cos_sel, cos_sel);                        \
    modify_field(_scratch_qstate_hdr.eval_last, eval_last);                    \
    modify_field(_scratch_qstate_hdr.host_rings, host_rings);                  \
    modify_field(_scratch_qstate_hdr.total_rings, total_rings);                \
    modify_field(_scratch_qstate_hdr.pid, pid);                                \
    modify_field(_scratch_qstate_hdr.p_index0, p_index0);                      \
    modify_field(_scratch_qstate_hdr.c_index0, c_index0);                      \
                                                                               \
    modify_field(_scratch_qstate_txdma_q.sw_pindex0, sw_pindex0);              \
    modify_field(_scratch_qstate_txdma_q.sw_cindex0, sw_cindex0);              \
    modify_field(_scratch_qstate_txdma_q.ring0_base, ring0_base);              \
    modify_field(_scratch_qstate_txdma_q.ring1_base, ring1_base);              \
    modify_field(_scratch_qstate_txdma_q.ring_size, ring_size);                \
    modify_field(_scratch_qstate_txdma_q.rxdma_cindex_addr, rxdma_cindex_addr)
