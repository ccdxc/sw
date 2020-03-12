#include "nic/p4/common/defines.h"

#define LIF_BIT_WIDTH                  11

#define CPU_LPORT                      1023

#define ACT_ALLOW                      0
#define ACT_DROP                       1

#define ASM_INSTRUCTION_OFFSET_MAX     (64 * 256)

/*****************************************************************************/
/* Default defines for any feature disable/enable knobs                      */
/*****************************************************************************/
#define P4_FEATURE_DISABLED            0
#define P4_FEATURE_ENABLED             1

/*****************************************************************************/
/* IP header types                                                         */
/*****************************************************************************/
#define IP_HEADER_TYPE_IPV4            0
#define IP_HEADER_TYPE_IPV6            1

/*****************************************************************************/
/* ROCE headers                                                              */
/*****************************************************************************/
#define ROCE_BTH_IMMDT                      1
#define ROCE_BTH_RETH                       2
#define ROCE_BTH_RETH_IMMDT                 3
#define ROCE_BTH_AETH                       4
#define ROCE_BTH_AETH_ATOMICACKETH          5
#define ROCE_BTH_ATOMICETH                  6
#define ROCE_BTH_IETH                       7
#define ROCE_BTH_RDETH                      8
#define ROCE_BTH_RDETH_DETH                 9
#define ROCE_BTH_RDETH_DETH_IMMDT           10
#define ROCE_BTH_RDETH_DETH_RETH            11
#define ROCE_BTH_RDETH_DETH_RETH_IMMDT      12
#define ROCE_BTH_RDETH_AETH                 13
#define ROCE_BTH_RDETH_AETH_ATOMICACKETH    14
#define ROCE_BTH_RDETH_ATOMICETH            15
#define ROCE_BTH_DETH                       16
#define ROCE_BTH_DETH_IMMDT                 17

/*****************************************************************************/
/* flow key types                                                            */
/*****************************************************************************/
#define FLOW_KEY_LOOKUP_TYPE_NONE              0
#define FLOW_KEY_LOOKUP_TYPE_MAC               1
#define FLOW_KEY_LOOKUP_TYPE_IPV4              2 /* IPv4 and IPv6 are kept as
                                                  * 0010b and 0011b so that
                                                  * if an ACL is needed to match
                                                  * either kind of traffic,
                                                  * entry can be installed as
                                                  * 001Xb (X denotes don't care
                                                  */
#define FLOW_KEY_LOOKUP_TYPE_IPV6              3
#define FLOW_KEY_LOOKUP_TYPE_IP_MASK           0x02

/*****************************************************************************/
/* flow direction                                                            */
/*****************************************************************************/
#define FLOW_DIR_FROM_DMA              0
#define FLOW_DIR_FROM_UPLINK           1

/*****************************************************************************/
/* flow miss actions                                                         */
/*****************************************************************************/
#define FLOW_MISS_ACTION_CPU           0
#define FLOW_MISS_ACTION_DROP          1
#define FLOW_MISS_ACTION_FLOOD         2
#define FLOW_MISS_ACTION_REDIRECT      3

/*****************************************************************************/
/* NIC mode                                                                  */
/*****************************************************************************/
#define NIC_MODE_CLASSIC               1
#define NIC_MODE_SMART                 0

/*****************************************************************************/
/* LIF filter mode                                                           */
/*****************************************************************************/
#define LIF_FILTER_UNICAST             0x01
#define LIF_FILTER_MULTICAST           0x02
#define LIF_FILTER_ALL_MULTICAST       0x04
#define LIF_FILTER_BROADCAST           0x08
#define LIF_FILTER_PROMISCUOUS         0x10

/*****************************************************************************/
/* drop reasons - these are bit positions to be used in ASM                  */
/*****************************************************************************/
#define DROP_MALFORMED_PKT                      0
#define DROP_PARSER_ICRC_ERR                    1
#define DROP_PARSER_LEN_ERR                     2
#define DROP_HARDWARE_ERR                       3
#define DROP_INPUT_MAPPING                      4
#define DROP_INPUT_MAPPING_DEJAVU               5
#define DROP_MULTI_DEST_NOT_PINNED_UPLINK       6
#define DROP_FLOW_HIT                           7
#define DROP_FLOW_MISS                          8
#define DROP_NACL                               9
#define DROP_IPSG                               10
#define DROP_IP_NORMALIZATION                   11
#define DROP_TCP_NORMALIZATION                  12
#define DROP_TCP_RST_WITH_INVALID_ACK_NUM       13
#define DROP_TCP_NON_SYN_FIRST_PKT              14
#define DROP_ICMP_NORMALIZATION                 15
#define DROP_INPUT_PROPERTIES_MISS              16
#define DROP_TCP_OUT_OF_WINDOW                  17
#define DROP_TCP_SPLIT_HANDSHAKE                18
#define DROP_TCP_WIN_ZERO_DROP                  19
#define DROP_TCP_DATA_AFTER_FIN                 20
#define DROP_TCP_NON_RST_PKT_AFTER_RST          21
#define DROP_TCP_INVALID_RESPONDER_FIRST_PKT    22
#define DROP_TCP_UNEXPECTED_PKT                 23
#define DROP_SRC_LIF_MISMATCH                   24
#define DROP_VF_IP_LABEL_MISMATCH               25
#define DROP_VF_BAD_RR_DST_IP                   26
#define DROP_IF_LABEL_MISMATCH                  27
#define DROP_ICMP_FRAGMENT_PKT                  28
#define DROP_MIN                                DROP_MALFORMED_PKT
/*****************************************************************************/
/* Note: After adding a new drop code, update the system.proto file as well. */
/*****************************************************************************/
#define DROP_MAX                                DROP_ICMP_FRAGMENT_PKT

/*****************************************************************************/
/* egress drop reasons - these are bit positions to be used in ASM           */
/*****************************************************************************/
#define EGRESS_DROP_OUTPUT_MAPPING              0
#define EGRESS_DROP_PRUNE_SRC_PORT              1
#define EGRESS_DROP_MIRROR                      2
#define EGRESS_DROP_POLICER                     3
#define EGRESS_DROP_COPP                        4
#define EGRESS_DROP_CHECKSUM_ERR                5
#define EGRESS_DROP_HARDWARE_ERR                6
#define EGRESS_DROP_MIN                         EGRESS_DROP_OUTPUT_MAPPING
#define EGRESS_DROP_MAX                         EGRESS_DROP_HARDWARE_ERR

/*****************************************************************************/
/* Rewrite flags                                                             */
/*****************************************************************************/
#define REWRITE_FLAGS_MAC_DA           0x01
#define REWRITE_FLAGS_MAC_SA           0x02
#define REWRITE_FLAGS_TTL_DEC          0x04

/*****************************************************************************/
/* flow role                                                                 */
/*****************************************************************************/
#define TCP_FLOW_INITIATOR             0
#define TCP_FLOW_RESPONDER             1

/*****************************************************************************/
/* flow states                                                               */
/*****************************************************************************/
#define FLOW_STATE_INIT                0
#define FLOW_STATE_TCP_SYN_RCVD        1
#define FLOW_STATE_TCP_ACK_RCVD        2
#define FLOW_STATE_TCP_SYN_ACK_RCVD    3
#define FLOW_STATE_ESTABLISHED         4
#define FLOW_STATE_FIN_RCVD            5
#define FLOW_STATE_BIDIR_FIN_RCVD      6
#define FLOW_STATE_RESET               7

/*****************************************************************************/
#define TCP_SYN_REXMIT                 0x0001
#define TCP_WIN_ZERO_DROP              0x0002
#define TCP_FULL_REXMIT                0x0004
#define TCP_PARTIAL_OVERLAP            0x0008
#define TCP_PACKET_REORDER             0x0010
#define TCP_OUT_OF_WINDOW              0x0020
#define TCP_ACK_ERR                    0x0040
#define TCP_NORMALIZATION_DROP         0x0080
#define TCP_SPLIT_HANDSHAKE_DETECTED   0x0100
#define TCP_DATA_AFTER_FIN             0x0200
#define TCP_NON_RST_PKT_AFTER_RST      0x0400
#define TCP_INVALID_RESPONDER_FIRST_PKT 0x0800
#define TCP_UNEXPECTED_PKT             0x1000
#define TCP_RST_WITH_INVALID_ACK_NUM   0x2000

/*****************************************************************************/
/* DHCP related                                                              */
/*****************************************************************************/
#define DHCP_CLIENT_PORT               67
#define DHCP_SERVER_PORT               68

/*****************************************************************************/
/* Recirc reason codes                                                       */
/*****************************************************************************/
#define RECIRC_NONE                    0
#define RECIRC_FLOW_HASH_OVERFLOW      1
#define RECIRC_VM_BOUNCE               2

/*****************************************************************************/
/* Checksum/iCRC flags (bit positions)                                       */
/*****************************************************************************/
#define CHECKSUM_CTL_ICRC               0
#define CHECKSUM_CTL_IP_CHECKSUM        1
#define CHECKSUM_CTL_L4_CHECKSUM        2
#define CHECKSUM_CTL_INNER_IP_CHECKSUM  3
#define CHECKSUM_CTL_INNER_L4_CHECKSUM  4

#define CHECKSUM_L3_L4_UPDATE_MASK        \
    (1<<CHECKSUM_CTL_IP_CHECKSUM | 1 << CHECKSUM_CTL_L4_CHECKSUM)
#define CHECKSUM_INNER_L3_L4_UPDATE_MASK  \
    (1<<CHECKSUM_CTL_IP_CHECKSUM | 1 << CHECKSUM_CTL_L4_CHECKSUM | \
     1<<CHECKSUM_CTL_INNER_IP_CHECKSUM | 1 << CHECKSUM_CTL_INNER_L4_CHECKSUM)

/*****************************************************************************/
/* Checksum flags from parser (for compiling P4 code only, don't use in ASM  */
/*****************************************************************************/
#define CSUM_HDR_IP                    0
#define CSUM_HDR_INNER_IP              0
#define CSUM_HDR_UDP                   0
#define CSUM_HDR_UDP_OPT_OCS           0
#define CSUM_HDR_TCP                   0
#define CSUM_HDR_INNER_UDP             0

// IPSEC
#define IPSEC_HEADER_AH                1
#define IPSEC_HEADER_ESP               2

// Minimum ethernet frame length - excludes CRC (4 bytes)
#define MIN_ETHER_FRAME_LEN            60

//Normalization
#define NORMALIZATION_ACTION_ALLOW     1
#define NORMALIZATION_ACTION_DROP      2
#define NORMALIZATION_ACTION_EDIT      3

#define TCP_NORMALIZATION_REASON_TCP_RES              100
#define TCP_NORMALIZATION_REASON_TCP_NOSYN_MSS        101
#define TCP_NORMALIZATION_REASON_TCP_NOSYN_WS         102
#define TCP_NORMALIZATION_REASON_TCP_NOURG_URGPTR     103
#define TCP_NORMALIZATION_REASON_TCP_URG_NOURGPTR     104
#define TCP_NORMALIZATION_REASON_URG_NOPAYLOAD        105
#define TCP_NORMALIZATION_REASON_NOACK_ECHOTS         106
#define TCP_NORMALIZATION_REASON_RST_PAYLOAD          107
#define TCP_NORMALIZATION_REASON_PAYLOAD_GT_MSS       108
#define TCP_NORMALIZATION_REASON_PAYLOAD_GT_WS        109
#define TCP_NORMALIZATION_REASON_NONNEG_TS_PRESENT    110
#define TCP_NORMALIZATION_REASON_NEG_TS_NOT_PRESENT   111
#define TCP_NORMALIZATION_REASON_BAD_FLAGS            112
#define TCP_NORMALIZATION_REASON_NONSYN_NOACK         113

#define IP_NORMALIZATION_REASON_RSVD                  120
#define IP_NORMALIZATION_REASON_DF                    121
#define IP_NORMALIZATION_REASON_OPTIONS               122
#define IP_NORMALIZATION_REASON_TOTAL_LENGTH          123

#define ICMP_NORMALIZATION_REASON_REQ_RESP            130
#define ICMP_NORMALIZATION_REASON_BAD_REQ             131
#define ICMP_NORMALIZATION_REASON_CODE_NON_ZERO       132
#define ICMP_NORMALIZATION_REASON_CODE_REDIRECT       133

#define IP_FLAGS_RSVD_MASK             4
#define IP_FLAGS_DF_MASK               2
#define IP_FLAGS_MF_MASK               1

// Different types of DDoS attacks that can be configured.
#define DDOS_TYPE_ANY                  0
#define DDOS_TYPE_SYN                  1
#define DDOS_TYPE_ICMP                 2
#define DDOS_TYPE_UDP                  3

#define P4_RECIRC_HDR_SZ               5

/*****************************************************************************/
/* ERSPAN types                                                              */
/*****************************************************************************/
#define ERSPAN_TYPE_I                  1
#define ERSPAN_TYPE_II                 2
#define ERSPAN_TYPE_III                3

/*****************************************************************************/
/* TCP OPTIONS                                                               */
/*****************************************************************************/
#define CPU_TCP_OPTIONS_WINDOW_SCALE   0x01
#define CPU_TCP_OPTIONS_MSS            0x02
#define CPU_TCP_OPTIONS_TIMESTAMP      0x04
#define CPU_TCP_OPTIONS_SACK_PERMITTED 0x08

/*****************************************************************************/
/* ROCE flags                                                                */
/*****************************************************************************/
#define ROCE_FLAGS_IPV6                0x1
#define ROCE_FLAGS_CNP                 0x2
#define ROCE_FLAGS_ECN                 0x4

/*****************************************************************************/
/* IPSEC flags                                                               */
/*****************************************************************************/
#define IPSEC_MODE_TUNNEL              1
#define IPSEC_MODE_TRANSPORT           2

#define IPSEC_ENCAP_TYPE_AH            1
#define IPSEC_ENCAP_TYPE_ESP           2
#define IPSEC_ENCAP_TYPE_AH_ESP        3

#define IPSEC_L3_PROTOCOL_IPV4         1

/*****************************************************************************/
/* When FTE needs to see a copy of a packet instead of redirecting we will   */
/* replicate the packet and send one copy to regular destination dicated by  */
/* flow and the other replicated packet will be sent to CPU. This will       */
/* that the packet will be delivered to FTE and will not be lost.            */
/* Copy is not repliable hence we are using this approach.                   */
/* Reserved multicast indices that will be used in P4 for packets like       */
/* 1. FIN Packets that will be forwarded and redirected to CPU_LPORT         */
/* 2. ALGs have some requirements where the requirment is to drive           */
/*    specific qnum per flow but that is not possible with this solution.    */
/*****************************************************************************/
#define P4_NW_MCAST_INDEX_FIN_COPY      1
#define P4_NW_MCAST_INDEX_RST_COPY      2
#define P4_NW_MCAST_INDEX_FLOW_REL_COPY 3
#define P4_NW_MCAST_INDEX_P4PT_COPY     4


/*****************************************************************************/
/* Defines for repl_type field in tm_replication_data_t                      */
/*****************************************************************************/
#define TM_REPL_TYPE_DEFAULT            0  // This is for normal multicast replication
#define TM_REPL_TYPE_TO_CPU_REL_COPY    1  // This is for copy to cpu using replication and reliable
#define TM_REPL_TYPE_HONOR_INGRESS      2  // Normal forwarding, Honor the ingress flow decided rewrites.


/*****************************************************************************/
/* Number of Hints in Flow table                                             */
/*****************************************************************************/
#define P4_FLOW_NUM_HINTS_PER_ENTRY     5

/*****************************************************************************/
/* P4 ingress to P4 egress flags                                             */
/*****************************************************************************/
#define P4_I2E_FLAGS_FLOW_MISS          0
#define P4_I2E_FLAGS_TUNNEL_TERMINATE   1
#define P4_I2E_FLAGS_TUNNEL_ORIGINATE   2
#define P4_I2E_FLAGS_UPLINK             3
// #define P4_I2E_FLAGS_NIC_MODE           4
#define P4_I2E_FLAGS_IP_FRAGMENT        4
