#include "nic/p4/common/defines.h"

#define ASM_INSTRUCTION_OFFSET_MAX     (8 * 256)

#define SUPPORT_GFT_GTEST              1
#define EXCEPTION_VPORT                0x3FF

/*****************************************************************************/
/* Checksum/iCRC flags (bit positions)                                       */
/*****************************************************************************/
#define CHECKSUM_CTL_ICRC               0
#define CHECKSUM_CTL_IP_CHECKSUM        1
#define CHECKSUM_CTL_L4_CHECKSUM        2
#define CHECKSUM_CTL_INNER_IP_CHECKSUM  3
#define CHECKSUM_CTL_INNER_L4_CHECKSUM  4

/*****************************************************************************/ 
/* Checksum flags from parser (for compiling P4 code only, don't use in ASM  */ 
/*****************************************************************************/ 
#define CSUM_HDR_UDP                   1
#define CSUM_HDR_UDP_OPT_OCS           0

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
#define MATCH_GRE_PROTO_1              0x00000040
#define MATCH_TRANSPORT_SRC_PORT_2     0x00000080
#define MATCH_TRANSPORT_DST_PORT_2     0x00000100
#define MATCH_TCP_FLAGS_2              0x00000200
#define MATCH_ICMP_TYPE_2              0x00000400
#define MATCH_ICMP_CODE_2              0x00000800
#define MATCH_TENANT_ID_2              0x00001000
#define MATCH_GRE_PROTO_2              0x00002000
#define MATCH_TRANSPORT_SRC_PORT_3     0x00004000
#define MATCH_TRANSPORT_DST_PORT_3     0x00008000
#define MATCH_TCP_FLAGS_3              0x00010000
#define MATCH_ICMP_TYPE_3              0x00020000
#define MATCH_ICMP_CODE_3              0x00040000
#define MATCH_TENANT_ID_3              0x00080000
#define MATCH_GRE_PROTO_3              0x00100000

/*****************************************************************************/
/* Transposition types                                                       */
/*****************************************************************************/
#define TRANSPOSITIONS_PUSH_ETHERNET_00            0x00000001
#define TRANSPOSITIONS_PUSH_IPV4_00                0x00000002
#define TRANSPOSITIONS_PUSH_IPV6_00                0x00000004
#define TRANSPOSITIONS_PUSH_CTAG_00                0x00000008
#define TRANSPOSITIONS_PUSH_ETHERNET_01            0x00000010
#define TRANSPOSITIONS_PUSH_IPV4_01                0x00000020
#define TRANSPOSITIONS_PUSH_IPV6_01                0x00000040
#define TRANSPOSITIONS_PUSH_CTAG_01                0x00000080

#define TRANSPOSITIONS_ENCAP_LEN_MASK_00           0x00FF0000
#define TRANSPOSITIONS_ENCAP_LEN_MASK_01           0x0000FF00
#define TRANSPOSITIONS_ENCAP_LEN_SHIFT_00          16
#define TRANSPOSITIONS_ENCAP_LEN_SHIFT_01          8

#define TRANSPOSITIONS_POP_ETHERNET                0x00000001
#define TRANSPOSITIONS_POP_CTAG                    0x00000002
#define TRANSPOSITIONS_POP_IPV4                    0x00000004
#define TRANSPOSITIONS_POP_IPV6                    0x00000008
#define TRANSPOSITIONS_MODIFY_ETHERNET_SRC         0x00000010
#define TRANSPOSITIONS_MODIFY_ETHERNET_DST         0x00000020
#define TRANSPOSITIONS_MODIFY_ETHERNET_TYPE        0x00000040
#define TRANSPOSITIONS_MODIFY_CTAG                 0x00000080
#define TRANSPOSITIONS_MODIFY_IP_SRC               0x00000100
#define TRANSPOSITIONS_MODIFY_IP_DST               0x00000200
#define TRANSPOSITIONS_MODIFY_IP_DSCP              0x00000400
#define TRANSPOSITIONS_MODIFY_IP_PROTO             0x00000800
#define TRANSPOSITIONS_MODIFY_IP_TTL               0x00001000

#define TRANSPOSITIONS_PUSH_VXLAN_00               0x00000001
#define TRANSPOSITIONS_PUSH_UDP_00                 0x00000002
#define TRANSPOSITIONS_PUSH_VXLAN_01               0x00000004
#define TRANSPOSITIONS_PUSH_UDP_01                 0x00000008

#define TRANSPOSITIONS_POP_VXLAN_1                 0x00000001
#define TRANSPOSITIONS_POP_UDP_1                   0x00000002
#define TRANSPOSITIONS_MODIFY_UDP_SPORT_1          0x00000004
#define TRANSPOSITIONS_MODIFY_UDP_DPORT_1          0x00000008
#define TRANSPOSITIONS_MODIFY_TCP_SPORT_1          0x00000010
#define TRANSPOSITIONS_MODIFY_TCP_DPORT_1          0x00000020
#define TRANSPOSITIONS_MODIFY_ICMP_TYPE_1          0x00000040
#define TRANSPOSITIONS_MODIFY_ICMP_CODE_1          0x00000080
#define TRANSPOSITIONS_POP_VXLAN_2                 0x00000100
#define TRANSPOSITIONS_POP_UDP_2                   0x00000200
#define TRANSPOSITIONS_MODIFY_UDP_SPORT_2          0x00000400
#define TRANSPOSITIONS_MODIFY_UDP_DPORT_2          0x00000800
#define TRANSPOSITIONS_MODIFY_TCP_SPORT_2          0x00001000
#define TRANSPOSITIONS_MODIFY_TCP_DPORT_2          0x00002000
#define TRANSPOSITIONS_MODIFY_ICMP_TYPE_2          0x00004000
#define TRANSPOSITIONS_MODIFY_ICMP_CODE_2          0x00008000
#define TRANSPOSITIONS_POP_VXLAN_3                 0x00010000
#define TRANSPOSITIONS_POP_UDP_3                   0x00020000
#define TRANSPOSITIONS_MODIFY_UDP_SPORT_3          0x00040000
#define TRANSPOSITIONS_MODIFY_UDP_DPORT_3          0x00080000
#define TRANSPOSITIONS_MODIFY_TCP_SPORT_3          0x00100000
#define TRANSPOSITIONS_MODIFY_TCP_DPORT_3          0x00200000
#define TRANSPOSITIONS_MODIFY_ICMP_TYPE_3          0x00400000
#define TRANSPOSITIONS_MODIFY_ICMP_CODE_3          0x00800000
