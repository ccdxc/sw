#define TRUE                            1
#define FALSE                           0

/*****************************************************************************/
/* Ether types                                                               */
/*****************************************************************************/
#define ETHERTYPE_STAG                  0x88a8
#define ETHERTYPE_CTAG                  0x8100
#define ETHERTYPE_QINQ                  0x9100
#define ETHERTYPE_MPLS_UNICAST          0x8847
#define ETHERTYPE_MPLS_MULTICAST        0x8848
#define ETHERTYPE_IPV4                  0x0800
#define ETHERTYPE_IPV6                  0x86dd
#define ETHERTYPE_ARP                   0x0806
#define ETHERTYPE_RARP                  0x8035
#define ETHERTYPE_ETHERNET              0x6558

/*****************************************************************************/
/* IP types                                                                  */
/*****************************************************************************/
#define IPTYPE_IPV4                     0
#define IPTYPE_IPV6                     1

/*****************************************************************************/
/* MPLS tag depth                                                            */
/*****************************************************************************/
#define MPLS_DEPTH                      3

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

/*****************************************************************************/
/* IP protocol types                                                         */
/*****************************************************************************/
#define IP_PROTO_ICMP                   1
#define IP_PROTO_IGMP                   2
#define IP_PROTO_IPV4                   4
#define IP_PROTO_TCP                    6
#define IP_PROTO_UDP                    17
#define IP_PROTO_IPV6                   41
#define IP_PROTO_GRE                    47
#define IP_PROTO_IPSEC_ESP              50
#define IP_PROTO_IPSEC_AH               51
#define IP_PROTO_ICMPV6                 58
#define IP_PROTO_EIGRP                  88
#define IP_PROTO_OSPF                   89
#define IP_PROTO_PIM                    103
#define IP_PROTO_VRRP                   112

/*****************************************************************************/
/* UDP services                                                              */
/*****************************************************************************/
#define UDP_PORT_VXLAN                  4789
#define UDP_PORT_VXLAN_GPE              4790
#define UDP_PORT_ROCE_V2                4791
#define UDP_PORT_GENV                   6081
#define UDP_PORT_MPLS                   6635

/*****************************************************************************/
/* TM port values                                                            */
/*****************************************************************************/
#define TM_PORT_UPLINK_0                0
#define TM_PORT_UPLINK_1                1
#define TM_PORT_UPLINK_2                2
#define TM_PORT_UPLINK_3                3
#define TM_PORT_UPLINK_4                4
#define TM_PORT_UPLINK_5                5
#define TM_PORT_UPLINK_6                6
#define TM_PORT_UPLINK_7                7
#define TM_PORT_NCSI                    8
#define TM_PORT_DMA                     9
#define TM_PORT_EGRESS                  10
#define TM_PORT_INGRESS                 11

/*****************************************************************************/
/* TM instance type                                                          */
/*****************************************************************************/
#define TM_INSTANCE_TYPE_NORMAL         0
#define TM_INSTANCE_TYPE_MULTICAST      1
#define TM_INSTANCE_TYPE_CPU            2
#define TM_INSTANCE_TYPE_SPAN           3
#define TM_INSTANCE_TYPE_CPU_AND_DROP   4
#define TM_INSTANCE_TYPE_SPAN_AND_DROP  5

/*****************************************************************************/
/* TCP flags                                                                 */
/*****************************************************************************/
#define TCP_FLAG_CWR                    0x80
#define TCP_FLAG_ECE                    0x40
#define TCP_FLAG_URG                    0x20
#define TCP_FLAG_ACK                    0x10
#define TCP_FLAG_PSH                    0x08
#define TCP_FLAG_RST                    0x04
#define TCP_FLAG_SYN                    0x02
#define TCP_FLAG_FIN                    0x01

/*****************************************************************************/
/* Key types                                                                 */
/*****************************************************************************/
#define KEY_TYPE_NONE                   0
#define KEY_TYPE_IPV4                   1
#define KEY_TYPE_IPV6                   2
#define KEY_TYPE_MAC                    3

/*****************************************************************************/
/* Packet direction                                                          */
/*****************************************************************************/
#define RX_PACKET                       0
#define TX_PACKET                       1

/*****************************************************************************/
/* Policer color                                                             */
/*****************************************************************************/
#define POLICER_COLOR_GREEN             0
#define POLICER_COLOR_YELLOW            1
#define POLICER_COLOR_RED               2

/*****************************************************************************/
/* Header sizes                                                              */
/*****************************************************************************/
#define CAPRI_GLOBAL_INTRINSIC_HDR_SZ   17
#define CAPRI_P4_INTRINSIC_HDR_SZ       5
#define CAPRI_RXDMA_INTRINSIC_HDR_SZ    15
