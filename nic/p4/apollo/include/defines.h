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
/* Lifs                                                                      */
/*****************************************************************************/
#define APOLLO_SERVICE_LIF              1020

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
#define DROP_SRC_MAC_ZERO               0
#define DROP_SRC_MAC_MISMATCH           1
#define DROP_SRC_DST_CHECK_FAIL         2
#define DROP_FLOW_HIT                   3
#define DROP_TEP_RX_DST_IP_MISMATCH     4
#define DROP_DST_SLOT_ID_MISS           5
#define DROP_RVPATH_SRC_IP_MISMATCH     6
#define DROP_RVPATH_VCN_MISMATCH        7

/*****************************************************************************/
/* flow role                                                                 */
/*****************************************************************************/
#define TCP_FLOW_INITIATOR             0
#define TCP_FLOW_RESPONDER             1

/*****************************************************************************/
/* Header sizes                                                              */
/*****************************************************************************/
#define APOLLO_PREDICATE_HDR_SZ         1
#define APOLLO_P4_TO_RXDMA_HDR_SZ       53
#define APOLLO_P4_TO_ARM_HDR_SZ         2
#define APOLLO_P4_TO_TXDMA_HDR_SZ       25
#define APOLLO_I2E_HDR_SZ               40

#define PKTQ_PAGE_SIZE                  10240

/* Qstate definition for packet Q - RxDMA to TxDMA and RxDMA to FTE */
#define PKTQ_QSTATE \
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host_rings, total_rings, pid, \
    p_index0, c_index0, arm_pindex1, arm_cindex1, sw_pindex0, sw_cindex0, \
    ring_base0, ring_base1, ring_size0, ring_size1

#define PKTQ_QSTATE_DVEC_SCRATCH(_scratch_qstate_hdr, _scratch_qstate_txdma_fte_q) \
    modify_field(_scratch_qstate_hdr.pc, pc);                                 \
    modify_field(_scratch_qstate_hdr.rsvd, rsvd);                                 \
    modify_field(_scratch_qstate_hdr.cosA, cosA);                                 \
    modify_field(_scratch_qstate_hdr.cosB, cosB);                                 \
    modify_field(_scratch_qstate_hdr.cos_sel, cos_sel);                           \
    modify_field(_scratch_qstate_hdr.eval_last, eval_last);                       \
    modify_field(_scratch_qstate_hdr.host_rings, host_rings);                     \
    modify_field(_scratch_qstate_hdr.total_rings, total_rings);                   \
    modify_field(_scratch_qstate_hdr.pid, pid);                                   \
    modify_field(_scratch_qstate_hdr.p_index0, p_index0);                         \
    modify_field(_scratch_qstate_hdr.c_index0, c_index0);                         \
                                                                                  \
    modify_field(_scratch_qstate_txdma_fte_q.arm_pindex1, arm_pindex1);           \
    modify_field(_scratch_qstate_txdma_fte_q.arm_cindex1, arm_cindex1);           \
    modify_field(_scratch_qstate_txdma_fte_q.sw_pindex0, sw_pindex0);             \
    modify_field(_scratch_qstate_txdma_fte_q.sw_cindex0, sw_cindex0);             \
    modify_field(_scratch_qstate_txdma_fte_q.ring_base0, ring_base0);             \
    modify_field(_scratch_qstate_txdma_fte_q.ring_base1, ring_base1);             \
    modify_field(_scratch_qstate_txdma_fte_q.ring_size0, ring_size0);             \
    modify_field(_scratch_qstate_txdma_fte_q.ring_size1, ring_size1)
