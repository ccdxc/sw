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
#define APOLLO_SERVICE_LIF              1020

/*****************************************************************************/
/* Packet direction                                                          */
/*****************************************************************************/
#define RX_FROM_SWITCH                  0
#define TX_FROM_HOST                    1

/*****************************************************************************/
/* drop reasons - these are bit positions to be used in ASM                  */
/*****************************************************************************/
#define P4I_DROP_REASON_MIN             0
#define P4I_DROP_REASON_MAX             0

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
/* number of hints in various HBM hash tables                                */
/*****************************************************************************/
#define P4_LOCAL_IP_MAPPING_NUM_HINTS_PER_ENTRY          10
#define P4_REMOTE_VNIC_MAPPING_RX_NUM_HINTS_PER_ENTRY    10
#define P4_REMOTE_VNIC_MAPPING_TX_NUM_HINTS_PER_ENTRY    8

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
