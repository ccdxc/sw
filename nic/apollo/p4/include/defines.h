#include "nic/p4/common/defines.h"

/*****************************************************************************/
/* IP types                                                                  */
/*****************************************************************************/
#define IPTYPE_IPV4                     0
#define IPTYPE_IPV6                     1

/*****************************************************************************/
/* MPLS tag depth                                                            */
/*****************************************************************************/
#define MPLS_DEPTH                      2

/*****************************************************************************/
/* Key types                                                                 */
/*****************************************************************************/
#define KEY_TYPE_NONE                   0
#define KEY_TYPE_IPV4                   1
#define KEY_TYPE_IPV6                   2
#define KEY_TYPE_MAC                    3

/*****************************************************************************/
/* Mode                                                                      */
/*****************************************************************************/
#define APOLLO_MODE_DEFAULT             0
#define APOLLO_MODE_VXLAN               1
#define APOLLO_MODE_EVPN                2

/*****************************************************************************/
/* Lifs                                                                      */
/*****************************************************************************/
#define APOLLO_SERVICE_LIF              34

/*****************************************************************************/
/* Packet direction                                                          */
/*****************************************************************************/
#define RX_FROM_SWITCH                  0
#define TX_FROM_HOST                    1

/*****************************************************************************/
/* IP Types                                                                  */
/*****************************************************************************/
#define IP_TYPE_OVERLAY                 0
#define IP_TYPE_PUBLIC                  1

/*****************************************************************************/
/* Encap Types towards switch                                                */
/*****************************************************************************/
#define GW_ENCAP                        0
#define VNIC_ENCAP                      1

/*****************************************************************************/
/* drop reasons - these are bit positions to be used in ASM                  */
/*****************************************************************************/
#define P4I_DROP_SRC_MAC_ZERO           0
#define P4I_DROP_SRC_MAC_MISMATCH       1
#define P4I_DROP_VNIC_INFO_TX_MISS      2
#define P4I_DROP_VNIC_INFO_RX_MISS      3
#define P4I_DROP_SRC_DST_CHECK_FAIL     4
#define P4I_DROP_FLOW_HIT               5
#define P4I_DROP_TEP_RX_DST_IP_MISMATCH 6
#define P4I_DROP_RVPATH_SRC_IP_MISMATCH 7
#define P4I_DROP_RVPATH_VPC_MISMATCH    8
#define P4I_DROP_NACL                   9
#define P4I_DROP_REASON_MIN             P4I_DROP_SRC_MAC_ZERO
#define P4I_DROP_REASON_MAX             P4I_DROP_NACL

#define P4E_DROP_INVALID_NEXTHOP        0
#define P4E_DROP_REASON_MIN             P4E_DROP_INVALID_NEXTHOP
#define P4E_DROP_REASON_MAX             P4E_DROP_INVALID_NEXTHOP

/*****************************************************************************/
/* flow role                                                                 */
/*****************************************************************************/
#define TCP_FLOW_INITIATOR              0
#define TCP_FLOW_RESPONDER              1

/*****************************************************************************/
/* route result type and bit position                                        */
/*****************************************************************************/
#define ROUTE_RESULT_TYPE_NEXTHOP                0
#define ROUTE_RESULT_TYPE_VPC                    1
#define ROUTE_RESULT_BIT_POS                     15
#define ROUTE_RESULT_TYPE_PEER_VPC_MASK          0x8000

/*****************************************************************************/
/* number of hints in various HBM hash tables                                */
/*****************************************************************************/
#define P4_LOCAL_IP_MAPPING_NUM_HINTS_PER_ENTRY          10
#define P4_REMOTE_VNIC_MAPPING_RX_NUM_HINTS_PER_ENTRY    10
#define P4_REMOTE_VNIC_MAPPING_TX_NUM_HINTS_PER_ENTRY    8

/*****************************************************************************/
/* cpu flags and bit positions                                               */
/*****************************************************************************/
#define APOLLO_CPU_FLAGS_VLAN_VALID_BIT_POS     1
#define APOLLO_CPU_FLAGS_IPV4_1_VALID_BIT_POS   2
#define APOLLO_CPU_FLAGS_IPV6_1_VALID_BIT_POS   3
#define APOLLO_CPU_FLAGS_ETH_2_VALID_BIT_POS    4
#define APOLLO_CPU_FLAGS_IPV4_2_VALID_BIT_POS   5
#define APOLLO_CPU_FLAGS_IPV6_2_VALID_BIT_POS   6
#define APOLLO_CPU_FLAGS_DIRECTION_BIT_POS      7

#define APOLLO_CPU_FLAGS_VLAN_VALID     (1 << APOLLO_CPU_FLAGS_VLAN_VALID_BIT_POS)
#define APOLLO_CPU_FLAGS_IPV4_1_VALID   (1 << APOLLO_CPU_FLAGS_IPV4_1_VALID_BIT_POS)
#define APOLLO_CPU_FLAGS_IPV6_1_VALID   (1 << APOLLO_CPU_FLAGS_IPV6_1_VALID_BIT_POS)
#define APOLLO_CPU_FLAGS_ETH_2_VALID    (1 << APOLLO_CPU_FLAGS_ETH_2_VALID_BIT_POS)
#define APOLLO_CPU_FLAGS_IPV4_2_VALID   (1 << APOLLO_CPU_FLAGS_IPV4_2_VALID_BIT_POS)
#define APOLLO_CPU_FLAGS_IPV6_2_VALID   (1 << APOLLO_CPU_FLAGS_IPV6_2_VALID_BIT_POS)
#define APOLLO_CPU_FLAGS_DIRECTION      (1 << APOLLO_CPU_FLAGS_DIRECTION_BIT_POS)

/*****************************************************************************/
/* Header sizes                                                              */
/*****************************************************************************/
#define APOLLO_PREDICATE_HDR_SZ         1
#define APOLLO_P4_TO_RXDMA_HDR_SZ       52
#define APOLLO_P4_TO_ARM_HDR_SZ         17
#define APOLLO_P4_TO_TXDMA_HDR_SZ       25
#define APOLLO_I2E_HDR_SZ               40

#define APOLLO_INGRESS_MIRROR_BLOB_SZ   (ASICPD_RXDMA_INTRINSIC_HDR_SZ - \
                                         ASICPD_P4_INTRINSIC_HDR_SZ + \
                                         APOLLO_P4_TO_RXDMA_HDR_SZ + \
                                         APOLLO_PREDICATE_HDR_SZ + \
                                         APOLLO_PREDICATE_HDR_SZ + \
                                         APOLLO_P4_TO_TXDMA_HDR_SZ + \
                                         APOLLO_I2E_HDR_SZ)

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
