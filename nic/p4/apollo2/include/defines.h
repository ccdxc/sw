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

/* Qstate definition for packet Q - RxDMA to TxDMA and RxDMA to FTE */
#define PKTQ_QSTATE \
                    rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, p_index0, c_index0, \
                    arm_pindex1, arm_cindex1, \
                    sw_pindex0, sw_cindex0, \
                    ring_base0, ring_base1, ring_sz_mask0, ring_sz_mask1
