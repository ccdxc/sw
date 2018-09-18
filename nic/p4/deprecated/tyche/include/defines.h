#include "nic/p4/common/defines.h"

/*****************************************************************************/
/* MPLS tag depth                                                            */
/*****************************************************************************/
#define MPLS_DEPTH 3

/*****************************************************************************/
/* Match types                                                               */
/*****************************************************************************/
#define MATCH_ETHERNET_DST             0x00000001
#define MATCH_ETHERNET_SRC             0x00000002
#define MATCH_ETHERNET_TYPE            0x00000004
#define MATCH_CUSTOMER_VLAN_ID         0x00000008
#define MATCH_IP_SRC                   0x00000010
#define MATCH_IP_DST                   0x00000020
#define MATCH_IP_DSCP                  0x00000040
#define MATCH_IP_PROTO                 0x00000080
#define MATCH_IP_TTL                   0x00000100

#define MATCH_TRANSPORT_SRC_PORT_1     0x00000001
#define MATCH_TRANSPORT_DST_PORT_1     0x00000002
#define MATCH_TCP_FLAGS_1              0x00000004
#define MATCH_ICMP_TYPE_1              0x00000008
#define MATCH_ICMP_CODE_1              0x00000010
#define MATCH_TENANT_ID_1              0x00000020
#define MATCH_MPLS_1                   0x00000040
#define MATCH_TRANSPORT_SRC_PORT_2     0x00000080
#define MATCH_TRANSPORT_DST_PORT_2     0x00000100
#define MATCH_TCP_FLAGS_2              0x00000200
#define MATCH_ICMP_TYPE_2              0x00000400
#define MATCH_ICMP_CODE_2              0x00000800
#define MATCH_CUSTOM_FIELD_1           0x00001000
#define MATCH_CUSTOM_FIELD_2           0x00002000
#define MATCH_CUSTOM_FIELD_3           0x00004000

/*****************************************************************************/
/* Rewrite types                                                             */
/*****************************************************************************/
#define REWRITE_PUSH_ETHERNET_0                 0x00000001
#define REWRITE_PUSH_CTAG_0                     0x00000002
#define REWRITE_PUSH_IPV4_0                     0x00000004
#define REWRITE_PUSH_IPV6_0                     0x00000008

#define REWRITE_ENCAP_LEN_MASK_0                0x0000FF00
#define REWRITE_ENCAP_LEN_SHIFT_0               8

#define REWRITE_POP_ETHERNET                    0x00000001
#define REWRITE_POP_CTAG                        0x00000002
#define REWRITE_POP_IPV4                        0x00000004
#define REWRITE_POP_IPV6                        0x00000008
#define REWRITE_MODIFY_ETHERNET_SRC             0x00000010
#define REWRITE_MODIFY_ETHERNET_DST             0x00000020
#define REWRITE_MODIFY_ETHERNET_TYPE            0x00000040
#define REWRITE_MODIFY_CTAG                     0x00000080
#define REWRITE_MODIFY_IP_SRC                   0x00000100
#define REWRITE_MODIFY_IP_DST                   0x00000200
#define REWRITE_MODIFY_IP_DSCP                  0x00000400
#define REWRITE_MODIFY_IP_PROTO                 0x00000800
#define REWRITE_MODIFY_IP_TTL                   0x00001000

#define REWRITE_PUSH_VXLAN_0                    0x00000001
#define REWRITE_PUSH_GRE_0                      0x00000002
#define REWRITE_PUSH_UDP_0                      0x00000004
#define REWRITE_PUSH_MPLS_0                     0x00000008

#define REWRITE_POP_VXLAN_1                     0x00000001
#define REWRITE_POP_GRE_1                       0x00000002
#define REWRITE_POP_UDP_1                       0x00000004
#define REWRITE_POP_MPLS_1                      0x00000008
#define REWRITE_MODIFY_UDP_SPORT_1              0x00000010
#define REWRITE_MODIFY_UDP_DPORT_1              0x00000020
#define REWRITE_MODIFY_TCP_SPORT_1              0x00000040
#define REWRITE_MODIFY_TCP_DPORT_1              0x00000080
#define REWRITE_MODIFY_ICMP_TYPE_1              0x00000100
#define REWRITE_MODIFY_ICMP_CODE_1              0x00000200
#define REWRITE_MODIFY_UDP_SPORT_2              0x00000400
#define REWRITE_MODIFY_UDP_DPORT_2              0x00000800
#define REWRITE_MODIFY_TCP_SPORT_2              0x00001000
#define REWRITE_MODIFY_TCP_DPORT_2              0x00002000
#define REWRITE_MODIFY_ICMP_TYPE_2              0x00004000
#define REWRITE_MODIFY_ICMP_CODE_2              0x00008000
