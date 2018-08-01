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
