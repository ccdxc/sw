#ifndef __COMMON_DEFINES_H__
#define __COMMON_DEFINES_H__

//Queue Types used on a regular LIF
//where RDMA and Ethernet can be enabled at the same time
#define Q_TYPE_RXQ     0
#define Q_TYPE_TXQ     1
#define Q_TYPE_ADMINQ  2
#define Q_TYPE_RDMA_SQ 3
#define Q_TYPE_RDMA_RQ 4
#define Q_TYPE_RDMA_CQ 5
#define Q_TYPE_RDMA_EQ 6

/*****************************************************************************/
/* Ether types                                                               */
/*****************************************************************************/
#define ETHERTYPE_VLAN                 0x8100
#define ETHERTYPE_QINQ                 0x9100
#define ETHERTYPE_MPLS                 0x8847
#define ETHERTYPE_IPV4                 0x0800
#define ETHERTYPE_IPV6                 0x86dd
#define ETHERTYPE_ARP                  0x0806
#define ETHERTYPE_RARP                 0x8035
#define ETHERTYPE_ETHERNET             0x6558

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

/*****************************************************************************/
/* UDP option types (kind)                                                   */
/*****************************************************************************/
#define UDP_KIND_EOL                    0
#define UDP_KIND_NOP                    1
#define UDP_KIND_OCS                    2
#define UDP_KIND_MSS                    5
#define UDP_KIND_TIMESTAMP              6


#endif //__COMMON_DEFINES_H__

