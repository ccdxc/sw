#ifndef _ATHENA_DEFINES_H_
#define _ATHENA_DEFINES_H_

//#include "nic/p4/common/defines.h"
#include "common/defines.h"

/*****************************************************************************/
/* Key types                                                                 */
/*****************************************************************************/
#define P4_KEY_TYPE_NONE                    0
#define P4_KEY_TYPE_IPV4                    1
#define P4_KEY_TYPE_IPV6                    2
#define P4_KEY_TYPE_MAC                     3

/*****************************************************************************/
/* Flow-cache index types                                                    */
/*****************************************************************************/
#define FLOW_CACHE_INDEX_TYPE_SESSION_INFO      0
#define FLOW_CACHE_INDEX_TYPE_CONNTRACK_INFO    1

/*****************************************************************************/
/* Packet direction                                                          */
/*****************************************************************************/
#define TX_FROM_HOST                    0
#define RX_FROM_SWITCH                  1

/*****************************************************************************/
/* Packet direction :egress                                                  */
/*****************************************************************************/
#define UPLINK_HOST                             TM_PORT_UPLINK_1
#define UPLINK_SWITCH                           TM_PORT_UPLINK_0


/*****************************************************************************/
/* User L3 rewrite types                                                     */
/*****************************************************************************/
#define L3REWRITE_NONE                          0
#define L3REWRITE_SNAT                          1
#define L3REWRITE_DNAT                          2


/*****************************************************************************/
/* Encap types                                                               */
/*****************************************************************************/
#define REWRITE_ENCAP_NONE                      0
#define REWRITE_ENCAP_L2                        1
#define REWRITE_ENCAP_MPLSOUDP                  2
#define REWRITE_ENCAP_MPLSOGRE                  3


/*****************************************************************************/
/* P4E packet types                                                          */
/*****************************************************************************/
#define P4E_PACKET_NORMAL                       0
#define P4E_PACKET_OVERLAY_IPV4                 1
#define P4E_PACKET_OVERLAY_IPV6                 2

/*****************************************************************************/
/* Packet redirect types                                                     */
/*****************************************************************************/
#define PACKET_ACTION_REDIR_RXDMA               0
#define PACKET_ACTION_REDIR_UPLINK              1

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
/* VNIC Type                                                                 */
/*****************************************************************************/
#define P4_VNIC_TYPE_L3                         0
#define P4_VNIC_TYPE_L2                         1


/*****************************************************************************/
/* Conntrack Flow types                                                      */
/*****************************************************************************/
#define CONNTRACK_FLOW_TYPE_TCP                 0
#define CONNTRACK_FLOW_TYPE_UDP                 1
#define CONNTRACK_FLOW_TYPE_ICMP                2
#define CONNTRACK_FLOW_TYPE_OTHERS              3

/*****************************************************************************/
/* Conntrack Flow States                                                     */
/*****************************************************************************/
#define CONNTRACK_FLOW_STATE_UNESTABLISHED      0
#define CONNTRACK_FLOW_STATE_SYN_SENT           1
#define CONNTRACK_FLOW_STATE_SYN_RECV           2
#define CONNTRACK_FLOW_STATE_SYNACK_SENT        3
#define CONNTRACK_FLOW_STATE_SYNACK_RECV        4
#define CONNTRACK_FLOW_STATE_ESTABLISHED        5
#define CONNTRACK_FLOW_STATE_FIN_SENT           6
#define CONNTRACK_FLOW_STATE_FIN_RECV           7
#define CONNTRACK_FLOW_STATE_TIME_WAIT          8
#define CONNTRACK_FLOW_STATE_RST_CLOSE          9
#define CONNTRACK_FLOW_STATE_REMOVED            10
#define CONNTRACK_FLOW_STATE_RSVD0              11
#define CONNTRACK_FLOW_STATE_RSVD1              12
#define CONNTRACK_FLOW_STATE_RSVD2              13
#define CONNTRACK_FLOW_STATE_RSVD3              14
#define CONNTRACK_FLOW_STATE_RSVD4              15


/*****************************************************************************/
/* Stats flags for P4E                                                       */
/*****************************************************************************/
#define P4E_STATS_FLAG_TX_TO_HOST               0x01
#define P4E_STATS_FLAG_TX_TO_SWITCH             0x02
#define P4E_STATS_FLAG_TX_TO_ARM                0x04



#endif /* _ATHENA_DEFINES_H_ */
