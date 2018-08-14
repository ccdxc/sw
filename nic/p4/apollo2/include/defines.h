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
#define HOSTPORT_LIF                    0
#define SWITCHPORT_LIF                  1

/*****************************************************************************/
/* Packet direction                                                          */
/*****************************************************************************/
#define RX_FROM_SWITCH                  0
#define TX_FROM_HOST                    1

/*****************************************************************************/
/* IP Types                                                                  */
/*****************************************************************************/
#define IP_TYPE_OVERLAY     0
#define IP_TYPE_PUBLIC      1

/*****************************************************************************/
/* Encap Types towards switch                                                */
/*****************************************************************************/
#define GW_ENCAP            0
#define VNIC_ENCAP          1

/*****************************************************************************/
/* drop reasons - these are bit positions to be used in ASM                  */
/*****************************************************************************/
#define DROP_FLOW_HIT                           0
#define DROP_TEP_RX_IP_MISMATCH                 1

#define APOLLO_PREDICATE_HDR_SZ     1
#define APOLLO_P4_TO_RXDMA_HDR_SZ   71

#define PKTQ_PAGE_SIZE              10240

/* Qstate definition for packet Q - RxDMA to TxDMA and RxDMA to FTE */
#define PKTQ_QSTATE \
                    rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, p_index0, c_index0, \
                    arm_pindex1, arm_cindex1, \
                    sw_pindex0, sw_cindex0, \
                    ring_base0, ring_base1, ring_sz_mask0, ring_sz_mask1

#define PKTQ_QSTATE_DVEC_SCRATCH(_scratch_qstate_hdr, _scratch_qstate_txdma_fte_Q) \
    modify_field(_scratch_qstate_hdr.rsvd, rsvd);                                 \
    modify_field(_scratch_qstate_hdr.cosA, cosA);                                 \
    modify_field(_scratch_qstate_hdr.cosB, cosB);                                 \
    modify_field(_scratch_qstate_hdr.cos_sel, cos_sel);                           \
    modify_field(_scratch_qstate_hdr.eval_last, eval_last);                       \
    modify_field(_scratch_qstate_hdr.host, host);                                 \
    modify_field(_scratch_qstate_hdr.total, total);                               \
    modify_field(_scratch_qstate_hdr.pid, pid);                                   \
    modify_field(_scratch_qstate_hdr.p_index0, p_index0);                         \
    modify_field(_scratch_qstate_hdr.c_index0, c_index0);                         \
                                                                                  \
    modify_field(_scratch_qstate_txdma_fte_Q.arm_pindex1, arm_pindex1);           \
    modify_field(_scratch_qstate_txdma_fte_Q.arm_cindex1, arm_cindex1);           \
    modify_field(_scratch_qstate_txdma_fte_Q.sw_pindex0, sw_pindex0);             \
    modify_field(_scratch_qstate_txdma_fte_Q.sw_cindex0, sw_cindex0);             \
    modify_field(_scratch_qstate_txdma_fte_Q.ring_base0, ring_base0);             \
    modify_field(_scratch_qstate_txdma_fte_Q.ring_base1, ring_base1);             \
    modify_field(_scratch_qstate_txdma_fte_Q.ring_sz_mask0, ring_sz_mask0);       \
    modify_field(_scratch_qstate_txdma_fte_Q.ring_sz_mask1, ring_sz_mask1)
