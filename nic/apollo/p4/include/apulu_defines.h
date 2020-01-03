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
/* Direction (lif connected to host or uplink)                               */
/*****************************************************************************/
#define P4_LIF_DIR_HOST                 0
#define P4_LIF_DIR_UPLINK               1

/*****************************************************************************/
/* drop reasons - these are bit positions to be used in ASM                  */
/*****************************************************************************/
#define P4I_DROP_REASON_MIN             0
#define P4I_DROP_VNI_INVALID            0
#define P4I_DROP_NACL                   1
#define P4I_DROP_VNIC_POLICER_TX        2
#define P4I_DROP_MAC_IP_BINDING_FAIL    3
#define P4I_DROP_REASON_MAX             3

#define P4E_DROP_REASON_MIN             0
#define P4E_DROP_SESSION_INVALID        0
#define P4E_DROP_SESSION_HIT            1
#define P4E_DROP_NEXTHOP_INVALID        2
#define P4E_DROP_VNIC_POLICER_RX        3
#define P4E_DROP_COPP                   4
#define P4E_DROP_REASON_MAX             4

/*****************************************************************************/
/* nexthop types                                                            */
/*****************************************************************************/
#define NEXTHOP_TYPE_VPC                0
#define NEXTHOP_TYPE_ECMP               1
#define NEXTHOP_TYPE_TUNNEL             2
#define NEXTHOP_TYPE_NEXTHOP            3
#define NEXTHOP_TYPE_NAT                4
#define NEXTHOP_TYPE_MAX                5

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

#define TX_REWRITE_DIP_START                    4
#define TX_REWRITE_DIP_MASK                     1
#define TX_REWRITE_DIP_NONE                     0
#define TX_REWRITE_DIP_FROM_NAT                 1
#define TX_REWRITE_DIP_BITS                     4:4

#define TX_REWRITE_SPORT_START                  5
#define TX_REWRITE_SPORT_MASK                   1
#define TX_REWRITE_SPORT_NONE                   0
#define TX_REWRITE_SPORT_FROM_NAT               1
#define TX_REWRITE_SPORT_BITS                   5:5

#define TX_REWRITE_DPORT_START                  6
#define TX_REWRITE_DPORT_MASK                   1
#define TX_REWRITE_DPORT_NONE                   0
#define TX_REWRITE_DPORT_FROM_NAT               1
#define TX_REWRITE_DPORT_BITS                   6:6

#define TX_REWRITE_ENCAP_START                  7
#define TX_REWRITE_ENCAP_MASK                   2
#define TX_REWRITE_ENCAP_NONE                   0
#define TX_REWRITE_ENCAP_VLAN                   1
#define TX_REWRITE_ENCAP_VXLAN                  2
#define TX_REWRITE_ENCAP_MPLSoUDP               3
#define TX_REWRITE_ENCAP_BITS                   8:7

#define TX_REWRITE_VNI_START                    9
#define TX_REWRITE_VNI_MASK                     1
#define TX_REWRITE_VNI_DEFAULT                  0
#define TX_REWRITE_VNI_FROM_TUNNEL              1
#define TX_REWRITE_VNI_BITS                     9:9

#define TX_REWRITE_TTL_START                    10
#define TX_REWRITE_TTL_MASK                     1
#define TX_REWRITE_TTL_NONE                     0
#define TX_REWRITE_TTL_DEC                      1
#define TX_REWRITE_TTL_BITS                     10:10

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

#define RX_REWRITE_SIP_START                    3
#define RX_REWRITE_SIP_MASK                     1
#define RX_REWRITE_SIP_NONE                     0
#define RX_REWRITE_SIP_FROM_NAT                 1
#define RX_REWRITE_SIP_BITS                     3:3

#define RX_REWRITE_DIP_START                    4
#define RX_REWRITE_DIP_MASK                     1
#define RX_REWRITE_DIP_NONE                     0
#define RX_REWRITE_DIP_FROM_NAT                 1
#define RX_REWRITE_DIP_BITS                     4:4

#define RX_REWRITE_SPORT_START                  5
#define RX_REWRITE_SPORT_MASK                   1
#define RX_REWRITE_SPORT_NONE                   0
#define RX_REWRITE_SPORT_FROM_NAT               1
#define RX_REWRITE_SPORT_BITS                   5:5

#define RX_REWRITE_DPORT_START                  6
#define RX_REWRITE_DPORT_MASK                   1
#define RX_REWRITE_DPORT_NONE                   0
#define RX_REWRITE_DPORT_FROM_NAT               1
#define RX_REWRITE_DPORT_BITS                   6:6

#define RX_REWRITE_ENCAP_START                  7
#define RX_REWRITE_ENCAP_MASK                   1
#define RX_REWRITE_ENCAP_NONE                   0
#define RX_REWRITE_ENCAP_VLAN                   1
#define RX_REWRITE_ENCAP_BITS                   7:7

#define RX_REWRITE_TTL_START                    8
#define RX_REWRITE_TTL_MASK                     1
#define RX_REWRITE_TTL_NONE                     0
#define RX_REWRITE_TTL_DEC                      1
#define RX_REWRITE_TTL_BITS                     8:8

#define TX_REWRITE(a, attr, val) \
    ((((a) >> TX_REWRITE_ ## attr ## _START) & TX_REWRITE_ ## attr ## _MASK) == TX_REWRITE_ ## attr ## _ ## val)

#define RX_REWRITE(a, attr, val) \
    ((((a) >> RX_REWRITE_ ## attr ## _START) & RX_REWRITE_ ## attr ## _MASK) == RX_REWRITE_ ## attr ## _ ## val)

/*****************************************************************************/
/* route result type and bit position                                        */
/*****************************************************************************/
#define ROUTE_RESULT_METER_EN_SIZE           1
#define ROUTE_RESULT_SNAT_TYPE_SIZE          2
#define ROUTE_RESULT_DNAT_EN_SIZE            1
#define ROUTE_RESULT_DNAT_IDX_SIZE           28         // DNAT_EN == 1
#define ROUTE_RESULT_NHTYPE_SIZE             2          // DNAT_EN == 0
#define ROUTE_RESULT_NEXTHOP_SIZE            26         // DNAT_EN == 0

#define ROUTE_RESULT_METER_EN_MASK           0x80000000
#define ROUTE_RESULT_SNAT_TYPE_MASK          0x60000000
#define ROUTE_RESULT_DNAT_EN_MASK            0x10000000
#define ROUTE_RESULT_DNAT_IDX_MASK           0x0FFFFFFF
#define ROUTE_RESULT_NHTYPE_MASK             0x0C000000
#define ROUTE_RESULT_NEXTHOP_MASK            0x03FFFFFF

#define ROUTE_RESULT_METER_EN_END_BIT       31
#define ROUTE_RESULT_METER_EN_START_BIT     (ROUTE_RESULT_METER_EN_END_BIT -\
                                             ROUTE_RESULT_METER_EN_SIZE + 1)
#define ROUTE_RESULT_METER_EN_SHIFT          ROUTE_RESULT_METER_EN_START_BIT

#define ROUTE_RESULT_SNAT_TYPE_END_BIT      (ROUTE_RESULT_METER_EN_START_BIT - 1)
#define ROUTE_RESULT_SNAT_TYPE_START_BIT    (ROUTE_RESULT_SNAT_TYPE_END_BIT -\
                                             ROUTE_RESULT_SNAT_TYPE_SIZE + 1)
#define ROUTE_RESULT_SNAT_TYPE_SHIFT         ROUTE_RESULT_SNAT_TYPE_START_BIT

#define ROUTE_RESULT_DNAT_EN_END_BIT        (ROUTE_RESULT_SNAT_TYPE_START_BIT - 1)
#define ROUTE_RESULT_DNAT_EN_START_BIT      (ROUTE_RESULT_DNAT_EN_END_BIT -\
                                             ROUTE_RESULT_DNAT_EN_SIZE + 1)
#define ROUTE_RESULT_DNAT_EN_SHIFT           ROUTE_RESULT_DNAT_EN_START_BIT

#define ROUTE_RESULT_DNAT_IDX_END_BIT       (ROUTE_RESULT_DNAT_EN_START_BIT - 1)
#define ROUTE_RESULT_DNAT_IDX_START_BIT     (ROUTE_RESULT_DNAT_IDX_END_BIT -\
                                             ROUTE_RESULT_DNAT_IDX_SIZE + 1)
#define ROUTE_RESULT_DNAT_IDX_SHIFT          ROUTE_RESULT_DNAT_IDX_START_BIT

#define ROUTE_RESULT_NHTYPE_END_BIT         (ROUTE_RESULT_DNAT_EN_START_BIT - 1)
#define ROUTE_RESULT_NHTYPE_START_BIT       (ROUTE_RESULT_NHTYPE_END_BIT -\
                                             ROUTE_RESULT_NHTYPE_SIZE + 1)
#define ROUTE_RESULT_NHTYPE_SHIFT            ROUTE_RESULT_NHTYPE_START_BIT

#define ROUTE_RESULT_NEXTHOP_END_BIT        (ROUTE_RESULT_NHTYPE_START_BIT - 1)
#define ROUTE_RESULT_NEXTHOP_START_BIT      (ROUTE_RESULT_NEXTHOP_END_BIT -\
                                             ROUTE_RESULT_NEXTHOP_SIZE + 1)
#define ROUTE_RESULT_NEXTHOP_SHIFT           ROUTE_RESULT_NEXTHOP_START_BIT

/*****************************************************************************/
/* number of hints in various HBM hash tables                                */
/*****************************************************************************/
#define P4_LOCAL_MAPPING_NUM_HINTS_PER_ENTRY              9
#define P4_MAPPING_NUM_HINTS_PER_ENTRY                    9

/*****************************************************************************/
/* cpu flags and bit positions                                               */
/*****************************************************************************/
#define APULU_CPU_FLAGS_VLAN_VALID_BIT_POS     0
#define APULU_CPU_FLAGS_IPV4_1_VALID_BIT_POS   1
#define APULU_CPU_FLAGS_IPV6_1_VALID_BIT_POS   2
#define APULU_CPU_FLAGS_ETH_2_VALID_BIT_POS    3
#define APULU_CPU_FLAGS_IPV4_2_VALID_BIT_POS   4
#define APULU_CPU_FLAGS_IPV6_2_VALID_BIT_POS   5

#define APULU_CPU_FLAGS_VLAN_VALID     (1 << APULU_CPU_FLAGS_VLAN_VALID_BIT_POS)
#define APULU_CPU_FLAGS_IPV4_1_VALID   (1 << APULU_CPU_FLAGS_IPV4_1_VALID_BIT_POS)
#define APULU_CPU_FLAGS_IPV6_1_VALID   (1 << APULU_CPU_FLAGS_IPV6_1_VALID_BIT_POS)
#define APULU_CPU_FLAGS_ETH_2_VALID    (1 << APULU_CPU_FLAGS_ETH_2_VALID_BIT_POS)
#define APULU_CPU_FLAGS_IPV4_2_VALID   (1 << APULU_CPU_FLAGS_IPV4_2_VALID_BIT_POS)
#define APULU_CPU_FLAGS_IPV6_2_VALID   (1 << APULU_CPU_FLAGS_IPV6_2_VALID_BIT_POS)

/*****************************************************************************/
/* Header sizes                                                              */
/*****************************************************************************/
#define APULU_P4I_TO_RXDMA_HDR_SZ       52
#define APULU_I2E_HDR_SZ                38
#define APULU_P4_TO_ARM_HDR_SZ          34
#define APULU_ARM_TO_P4_HDR_SZ          5

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
#define APULU_PKT_DESC_SHIFT              7
#define APULU_PKT_DESC_SIZE              (1 << APULU_PKT_DESC_SHIFT)

