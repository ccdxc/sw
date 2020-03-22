#ifndef __P4_COMMON_DEFINES_H__
#define __P4_COMMON_DEFINES_H__

// Add a "if asic == capri" check to support more asics in future.
#include "capri.h"

#define TRUE                           1
#define FALSE                          0

/*****************************************************************************/
/* Ether types                                                               */
/*****************************************************************************/
#define ETHERTYPE_STAG                 0x88a8
#define ETHERTYPE_VLAN                 0x8100
#define ETHERTYPE_CTAG                 0x8100
#define ETHERTYPE_QINQ                 0x9100
#define ETHERTYPE_MPLS                 0x8847
#define ETHERTYPE_IPV4                 0x0800
#define ETHERTYPE_IPV6                 0x86dd
#define ETHERTYPE_ARP                  0x0806
#define ETHERTYPE_RARP                 0x8035
#define ETHERTYPE_ETHERNET             0x6558
#define ETHERTYPE_MPLS_UNICAST         0x8847
#define ETHERTYPE_MPLS_MULTICAST       0x8848

/*****************************************************************************/
/* IP protocol types                                                         */
/*****************************************************************************/
#define IP_PROTO_ICMP                  1
#define IP_PROTO_IGMP                  2
#define IP_PROTO_IPV4                  4
#define IP_PROTO_TCP                   6
#define IP_PROTO_UDP                   17
#define IP_PROTO_IPV6                  41
#define IP_PROTO_GRE                   47
#define IP_PROTO_IPSEC_ESP             50
#define IP_PROTO_IPSEC_AH              51
#define IP_PROTO_ICMPV6                58
#define IP_PROTO_EIGRP                 88
#define IP_PROTO_OSPF                  89
#define IP_PROTO_PIM                   103
#define IP_PROTO_VRRP                  112

/*****************************************************************************/
/* IPv6 extension header types                                               */
/*****************************************************************************/
#define IPV6_PROTO_EXTN_HOPBYHOP       0
#define IPV6_PROTO_EXTN_ROUTING_HDR    43
#define IPV6_PROTO_EXTN_FRAGMENT_HDR   44
#define IPV6_PROTO_EXTN_ESP_HDR        50
#define IPV6_PROTO_EXTN_AH_HDR         51
#define IPV6_PROTO_EXTN_DEST_OPT_HDR   60
#define IPV6_PROTO_EXTN_MOBILITY_HDR   135
#define IPV6_PROTO_EXTN_NO_HDR         59

/*****************************************************************************/
/* UDP services                                                              */
/*****************************************************************************/
#define UDP_PORT_VXLAN                 4789
#define UDP_PORT_VXLAN_GPE             4790
#define UDP_PORT_ROCE_V2               4791
#define UDP_PORT_GENV                  6081
#define UDP_PORT_NATT                  4500
#define UDP_PORT_MPLS                  6635

/*****************************************************************************/
/* UDP option types (kind)                                                   */
/*****************************************************************************/
#define UDP_KIND_EOL                    0
#define UDP_KIND_NOP                    1
#define UDP_KIND_OCS                    2
#define UDP_KIND_MSS                    5
#define UDP_KIND_TIMESTAMP              6

/*****************************************************************************/
/* P4+ app types                                                             */
/*****************************************************************************/
#define P4PLUS_APPTYPE_DEFAULT         0
#define P4PLUS_APPTYPE_CLASSIC_NIC     1
#define P4PLUS_APPTYPE_RDMA            2
#define P4PLUS_APPTYPE_TCPTLS          3
#define P4PLUS_APPTYPE_IPSEC           4
#define P4PLUS_APPTYPE_STORAGE         5
#define P4PLUS_APPTYPE_TELEMETRY       6
#define P4PLUS_APPTYPE_CPU             7
#define P4PLUS_APPTYPE_RAW_REDIR       8
#define P4PLUS_APPTYPE_P4PT            9
#define P4PLUS_APPTYPE_MIN             P4PLUS_APPTYPE_DEFAULT
#define P4PLUS_APPTYPE_MAX             P4PLUS_APPTYPE_P4PT

/*****************************************************************************/
/* TCP flags                                                                 */
/*****************************************************************************/
#define TCP_FLAG_CWR                   0x80
#define TCP_FLAG_ECE                   0x40
#define TCP_FLAG_URG                   0x20
#define TCP_FLAG_ACK                   0x10
#define TCP_FLAG_PSH                   0x08
#define TCP_FLAG_RST                   0x04
#define TCP_FLAG_SYN                   0x02
#define TCP_FLAG_FIN                   0x01

/*****************************************************************************/
/* Tunnel types                                                              */
/*****************************************************************************/
#define INGRESS_TUNNEL_TYPE_NONE        0
#define INGRESS_TUNNEL_TYPE_VXLAN       1
#define INGRESS_TUNNEL_TYPE_GRE         2
#define INGRESS_TUNNEL_TYPE_IP_IN_IP    3
#define INGRESS_TUNNEL_TYPE_GENEVE      4
#define INGRESS_TUNNEL_TYPE_NVGRE       5
#define INGRESS_TUNNEL_TYPE_MPLS_L2VPN  6
#define INGRESS_TUNNEL_TYPE_MPLS_L3VPN  7
#define INGRESS_TUNNEL_TYPE_VXLAN_GPE   8
#define INGRESS_TUNNEL_TYPE_UDP_MPLS    9
#define INGRESS_TUNNEL_TYPE_GRE_MPLS    10


/*****************************************************************************/
/* P4+ to P4 flags                                                           */
/*****************************************************************************/
#define P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID             0x01
#define P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN            0x02
#define P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO        0x04
#define P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN           0x08
#define P4PLUS_TO_P4_FLAGS_INSERT_VLAN_TAG          0x10
#define P4PLUS_TO_P4_FLAGS_COMPUTE_L4_CSUM          0x20
#define P4PLUS_TO_P4_FLAGS_COMPUTE_INNER_L4_CSUM    0x40
#define P4PLUS_TO_P4_FLAGS_LKP_INST                 0x80

/*****************************************************************************/
/* P4+ to P4 flags (same as above, but specify bit positions)                */
/*****************************************************************************/
#define P4PLUS_TO_P4_FLAGS_UPDATE_IP_ID_BIT_POS             0
#define P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN_BIT_POS            1
#define P4PLUS_TO_P4_FLAGS_UPDATE_TCP_SEQ_NO_BIT_POS        2
#define P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN_BIT_POS           3
#define P4PLUS_TO_P4_FLAGS_INSERT_VLAN_TAG_BIT_POS          4
#define P4PLUS_TO_P4_FLAGS_COMPUTE_L4_CSUM_BIT_POS          5
#define P4PLUS_TO_P4_FLAGS_COMPUTE_INNER_L4_CSUM_BIT_POS    6
#define P4PLUS_TO_P4_FLAGS_LKP_INST_BIT_POS                 7

/*****************************************************************************/
/* CPU flags                                                                 */
/*****************************************************************************/
#define CPU_FLAGS_VLAN_VALID           0x01
#define CPU_FLAGS_IPV4_VALID           0x02
#define CPU_FLAGS_IPV6_VALID           0x04
#define CPU_FLAGS_IP_OPTIONS_PRESENT   0x08
#define CPU_FLAGS_TCP_OPTIONS_PRESENT  0x10

/*****************************************************************************/
/* CPU header packet types                                                   */
/*****************************************************************************/
#define CPU_PACKET_TYPE_NONE           0
#define CPU_PACKET_TYPE_IPV4           1
#define CPU_PACKET_TYPE_IPV6           2

/*****************************************************************************/
/* CPU LKP Flags                                                             */
/*****************************************************************************/
#define CPU_LKP_FLAGS_LKP_TYPE_POS     0
#define CPU_LKP_FLAGS_LKP_INST         4
#define CPU_LKP_FLAGS_LKP_DIR          5

/*****************************************************************************/
/* Header Sizes                                                              */
/*****************************************************************************/
#define CAPRI_GLOBAL_INTRINSIC_HDR_SZ  17
#define CAPRI_P4_INTRINSIC_HDR_SZ      5
#define CAPRI_RXDMA_INTRINSIC_HDR_SZ   15
#define CAPRI_TXDMA_INTRINSIC_HDR_SZ   9

/*****************************************************************************/
/* P4 to P4+ header size constants (in bytes)                                */
/*****************************************************************************/
#define P4PLUS_ROCE_HDR_SZ             20
#define P4PLUS_TCP_PROXY_HDR_SZ        62
#define P4PLUS_CLASSIC_NIC_HDR_SZ      48
#define P4PLUS_CPU_HDR_SZ              40
#define P4PLUS_CPU_PKT_SZ              41
#define P4PLUS_IPSEC_HDR_SZ            18
#define P4PLUS_RAW_REDIR_HDR_SZ        40
#define P4PLUS_P4PT_HDR_SZ             7
#define P4PLUS_MIRROR_HDR_SZ           3
#define P4PLUS_MIRROR_PKT_SZ           16

/*****************************************************************************/
/* P4 fixed header size constants (in bytes)                                */
/*****************************************************************************/
#define UDP_HDR_SIZE                   8
#define IPV6_BASE_HDR_SIZE             40
#define IPV4_BASE_HDR_SIZE             20

/*****************************************************************************/
/* P4+ to P4 header size constants (in bytes)                                */
/*****************************************************************************/
#define P4PLUS_TO_P4_HDR_SZ            20

/*****************************************************************************/
/* Policer color                                                             */
/*****************************************************************************/
#define POLICER_COLOR_GREEN            0
#define POLICER_COLOR_YELLOW           1
#define POLICER_COLOR_RED              2

/*****************************************************************************/
/* GRE services                                                              */
/*****************************************************************************/
#define GRE_PROTO_NVGRE                0x20006558
#define GRE_PROTO_ERSPAN_T3            0x22EB   /* Type III version 2 */

/*****************************************************************************/
/* Queue types                                                               */
/*****************************************************************************/
#define Q_TYPE_RXQ                     0
#define Q_TYPE_TXQ                     1
#define Q_TYPE_ADMINQ                  2
#define Q_TYPE_RDMA_SQ                 3
#define Q_TYPE_RDMA_RQ                 4
#define Q_TYPE_RDMA_CQ                 5
#define Q_TYPE_RDMA_EQ                 6

/*****************************************************************************/
/* LIF stats offsets                                                         */
/*****************************************************************************/
#define LIF_STATS_TX_UCAST_OFFSET                   256
#define LIF_STATS_TX_MCAST_OFFSET                   272
#define LIF_STATS_TX_BCAST_OFFSET                   304
#define LIF_STATS_TX_UCAST_DROP_OFFSET              320
#define LIF_STATS_TX_MCAST_DROP_OFFSET              336
#define LIF_STATS_TX_BCAST_DROP_OFFSET              352

/*****************************************************************************/
/* Classic NIC header flags                                                  */
/*****************************************************************************/
#define CLASSIC_NIC_PKT_TYPE_NON_IP     0x000
#define CLASSIC_NIC_PKT_TYPE_IPV4       0x001
#define CLASSIC_NIC_PKT_TYPE_IPV4_TCP   0x003
#define CLASSIC_NIC_PKT_TYPE_IPV4_UDP   0x005
#define CLASSIC_NIC_PKT_TYPE_IPV6       0x008
#define CLASSIC_NIC_PKT_TYPE_IPV6_TCP   0x018
#define CLASSIC_NIC_PKT_TYPE_IPV6_UDP   0x028

#endif //__P4_COMMON_DEFINES_H__
