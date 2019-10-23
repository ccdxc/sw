#include "nic/p4/common/defines.h"

/*****************************************************************************/
/* IP types                                                                  */
/*****************************************************************************/
#define IPTYPE_IPV4                     0
#define IPTYPE_IPV6                     1

/*****************************************************************************/
/* key types                                                                 */
/*****************************************************************************/
#define KEY_TYPE_NONE                   0
#define KEY_TYPE_IPV4                   1
#define KEY_TYPE_IPV6                   2
#define KEY_TYPE_MAC                    3

/*****************************************************************************/
/* Lifs                                                                      */
/*****************************************************************************/
#define APULU_SERVICE_LIF               34

/*****************************************************************************/
/* Lif types                                                                 */
/*****************************************************************************/
#define P4_LIF_TYPE_HOST                0
#define P4_LIF_TYPE_UPLINK              1

/*****************************************************************************/
/* drop reasons - these are bit positions to be used in ASM                  */
/*****************************************************************************/
#define P4I_DROP_REASON_MIN             0
#define P4I_DROP_VNI_INVALID            0
#define P4I_DROP_NACL                   1
#define P4I_DROP_REASON_MAX             1

#define P4E_DROP_REASON_MIN             0
#define P4E_DROP_SESSION_INVALID        0
#define P4E_DROP_SESSION_HIT            1
#define P4E_DROP_NEXTHOP_INVALID        2
#define P4E_DROP_REASON_MAX             2

/*****************************************************************************/
/* nexthop types                                                            */
/*****************************************************************************/
#define NEXTHOP_TYPE_VPC                0
#define NEXTHOP_TYPE_ECMP               1
#define NEXTHOP_TYPE_TUNNEL             2
#define NEXTHOP_TYPE_NEXTHOP            3

/*****************************************************************************/
/* flow role                                                                 */
/*****************************************************************************/
#define TCP_FLOW_INITIATOR              0
#define TCP_FLOW_RESPONDER              1

/*****************************************************************************/
/* rewrite flags                                                             */
/*****************************************************************************/
#define TX_REWRITE_DMAC_START                   0
#define TX_REWRITE_DMAC_MASK                    2
#define TX_REWRITE_DMAC_NONE                    0
#define TX_REWRITE_DMAC_FROM_MAPPING            1
#define TX_REWRITE_DMAC_FROM_NEXTHOP            2
#define TX_REWRITE_DMAC_FROM_TUNNEL             3
#define TX_REWRITE_DMAC_BITS                    1:0

#define TX_REWRITE_SMAC_START                   2
#define TX_REWRITE_SMAC_MASK                    1
#define TX_REWRITE_SMAC_NONE                    0
#define TX_REWRITE_SMAC_FROM_VRMAC              1
#define TX_REWRITE_SMAC_BITS                    2:2

#define TX_REWRITE_SIP_START                    3
#define TX_REWRITE_SIP_MASK                     1
#define TX_REWRITE_SIP_NONE                     0
#define TX_REWRITE_SIP_FROM_NAT                 1
#define TX_REWRITE_SIP_BITS                     3:3

#define TX_REWRITE_SPORT_START                  4
#define TX_REWRITE_SPORT_MASK                   1
#define TX_REWRITE_SPORT_NONE                   0
#define TX_REWRITE_SPORT_FROM_NAT               1
#define TX_REWRITE_SPORT_BITS                   4:4

#define TX_REWRITE_ENCAP_START                  5
#define TX_REWRITE_ENCAP_MASK                   2
#define TX_REWRITE_ENCAP_NONE                   0
#define TX_REWRITE_ENCAP_VLAN                   1
#define TX_REWRITE_ENCAP_VXLAN                  2
#define TX_REWRITE_ENCAP_BITS                   6:5

#define RX_REWRITE_DMAC_START                   0
#define RX_REWRITE_DMAC_MASK                    2
#define RX_REWRITE_DMAC_NONE                    0
#define RX_REWRITE_DMAC_FROM_MAPPING            1
#define RX_REWRITE_DMAC_FROM_NEXTHOP            2
#define RX_REWRITE_DMAC_BITS                    1:0

#define RX_REWRITE_SMAC_START                   2
#define RX_REWRITE_SMAC_MASK                    1
#define RX_REWRITE_SMAC_NONE                    0
#define RX_REWRITE_SMAC_FROM_VRMAC              1
#define RX_REWRITE_SMAC_BITS                    2:2

#define RX_REWRITE_DIP_START                    3
#define RX_REWRITE_DIP_MASK                     1
#define RX_REWRITE_DIP_NONE                     0
#define RX_REWRITE_DIP_FROM_NAT                 1
#define RX_REWRITE_DIP_BITS                     3:3

#define RX_REWRITE_DPORT_START                  4
#define RX_REWRITE_DPORT_MASK                   1
#define RX_REWRITE_DPORT_NONE                   0
#define RX_REWRITE_DPORT_FROM_NAT               1
#define RX_REWRITE_DPORT_BITS                   4:4

#define RX_REWRITE_ENCAP_START                  5
#define RX_REWRITE_ENCAP_MASK                   1
#define RX_REWRITE_ENCAP_NONE                   0
#define RX_REWRITE_ENCAP_VLAN                   1
#define RX_REWRITE_ENCAP_BITS                   5:5

#define TX_REWRITE(a, attr, val) \
    ((((a) >> TX_REWRITE_ ## attr ## _START) & TX_REWRITE_ ## attr ## _MASK) == TX_REWRITE_ ## attr ## _ ## val)

#define RX_REWRITE(a, attr, val) \
    ((((a) >> RX_REWRITE_ ## attr ## _START) & RX_REWRITE_ ## attr ## _MASK) == RX_REWRITE_ ## attr ## _ ## val)

/*****************************************************************************/
/* route result type and bit position                                        */
/*****************************************************************************/
#define ROUTE_RESULT_TYPE_NEXTHOP                0
#define ROUTE_RESULT_TYPE_REM_SVC_TUNNEL_BIT_POS 21
#define ROUTE_RESULT_TYPE_SVC_TUNNEL_BIT_POS     22
#define ROUTE_RESULT_TYPE_PEER_VPC_BIT_POS       23
#define ROUTE_RESULT_TYPE_REM_SVC_TUNNEL_MASK    0x200000
#define ROUTE_RESULT_TYPE_SVC_TUNNEL_MASK        0x400000
#define ROUTE_RESULT_TYPE_PEER_VPC_MASK          0x800000

/*****************************************************************************/
/* number of hints in various HBM hash tables                                */
/*****************************************************************************/
#define P4_LOCAL_MAPPING_NUM_HINTS_PER_ENTRY             10
#define P4_MAPPING_NUM_HINTS_PER_ENTRY                    9

/*****************************************************************************/
/* Header sizes                                                              */
/*****************************************************************************/
#define APULU_P4I_TO_RXDMA_HDR_SZ       52
#define APULU_I2E_HDR_SZ                34
#define APULU_P4_TO_ARM_HDR_SZ          17

#define APULU_INGRESS_MIRROR_BLOB_SZ    (CAPRI_P4_INTRINSIC_HDR_SZ + \
                                         APULU_I2E_HDR_SZ)

#define PKTQ_PAGE_SIZE                  10240

/* Qstate definition for packet Q - RxDMA to TxDMA */
#define PKTQ_QSTATE                                                            \
    pc, rsvd, cosA, cosB, cos_sel, eval_last, host_rings, total_rings, pid,    \
    p_index0, c_index0, sw_pindex0, sw_cindex0, ring0_base, ring1_base,        \
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

/*****************************************************************************/
/* Apulu Pkt memory                                                        */
/*****************************************************************************/
#define APULU_PKT_DESC_SHIFT              0
#define APULU_PKT_DESC_SIZE              (1 << APULU_PKT_DESC_SHIFT)

