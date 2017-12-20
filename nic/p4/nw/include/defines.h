#define TRUE                           1
#define FALSE                          0

#define LIF_BIT_WIDTH                  11

#define CPU_LPORT                      1023

#define ACT_ALLOW                      0
#define ACT_DROP                       1

#define ASM_INSTRUCTION_OFFSET_MAX     (8 * 256)

/*****************************************************************************/
/* Default defines for any feature disable/enable knobs                      */
/*****************************************************************************/
#define P4_FEATURE_DISABLED            0
#define P4_FEATURE_ENABLED             1

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
/* Tunnel types                                                              */
/*****************************************************************************/
#define INGRESS_TUNNEL_TYPE_NONE       0
#define INGRESS_TUNNEL_TYPE_VXLAN      1
#define INGRESS_TUNNEL_TYPE_GRE        2
#define INGRESS_TUNNEL_TYPE_IP_IN_IP   3
#define INGRESS_TUNNEL_TYPE_GENEVE     4
#define INGRESS_TUNNEL_TYPE_NVGRE      5
#define INGRESS_TUNNEL_TYPE_MPLS_L2VPN 6
#define INGRESS_TUNNEL_TYPE_MPLS_L3VPN 7
#define INGRESS_TUNNEL_TYPE_VXLAN_GPE  8

/*****************************************************************************/
/* IP header types                                                         */
/*****************************************************************************/
#define IP_HEADER_TYPE_IPV4            0
#define IP_HEADER_TYPE_IPV6            1

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

/*****************************************************************************/
/* GRE services                                                              */
/*****************************************************************************/
#define GRE_PROTO_NVGRE                0x20006558
#define GRE_PROTO_ERSPAN_T3            0x22EB   /* Type III version 2 */

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
/* TM port values                                                            */
/*****************************************************************************/
#define TM_PORT_UPLINK_0               0
#define TM_PORT_UPLINK_1               1
#define TM_PORT_UPLINK_2               2
#define TM_PORT_UPLINK_3               3
#define TM_PORT_UPLINK_4               4
#define TM_PORT_UPLINK_5               5
#define TM_PORT_UPLINK_6               6
#define TM_PORT_UPLINK_7               7
#define TM_PORT_NCSI                   8
#define TM_PORT_DMA                    9 /* Tx and Rx DMA */
#define TM_PORT_EGRESS                 10
#define TM_PORT_INGRESS                11
#define TM_UPLINK_PORT_BEGIN           TM_PORT_UPLINK_0
#define TM_UPLINK_PORT_END             TM_PORT_NCSI
#define TM_DMA_PORT_BEGIN              TM_PORT_DMA
#define TM_DMA_PORT_END                TM_PORT_DMA
#define TM_P4_PORT_BEGIN               TM_PORT_EGRESS
#define TM_P4_PORT_END                 TM_PORT_INGRESS
#define TM_NUM_UPLINK_PORTS            (TM_UPLINK_PORT_END-TM_UPLINK_PORT_BEGIN+1)
#define TM_NUM_PORTS                   (TM_PORT_INGRESS+1)

/*****************************************************************************/
/* TM reserved queues                                                        */
/*****************************************************************************/
#define TM_P4_IG_RECIRC_QUEUE          31 /* Recirc queue in the P4 IG port */

/*****************************************************************************/
/* TM instance type                                                          */
/*****************************************************************************/
#define TM_INSTANCE_TYPE_NORMAL        0
#define TM_INSTANCE_TYPE_MULTICAST     1
#define TM_INSTANCE_TYPE_CPU           2
#define TM_INSTANCE_TYPE_SPAN          3
#define TM_INSTANCE_TYPE_CPU_AND_DROP  4
#define TM_INSTANCE_TYPE_SPAN_AND_DROP 5

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
#define FLOW_KEY_LOOKUP_TYPE_FROM_VM_BOUNCE    4
#define FLOW_KEY_LOOKUP_TYPE_TO_VM_BOUNCE      5
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
/* packet type                                                               */
/*****************************************************************************/
#define PACKET_TYPE_UNICAST            0
#define PACKET_TYPE_MULTICAST          1
#define PACKET_TYPE_BROADCAST          2

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
#define DROP_INPUT_MAPPING             0
#define DROP_INPUT_MAPPING_DEJAVU      1
#define DROP_FLOW_HIT                  2
#define DROP_FLOW_MISS                 3
#define DROP_IPSG                      4
#define DROP_INGRESS_POLICER           5
#define DROP_RX_POLICER                6
#define DROP_NACL                      7
#define DROP_MALFORMED_PKT             8
#define DROP_PING_OF_DEATH             9
#define DROP_FRAGMENT_TOO_SMALL        10
#define DROP_IP_NORMALIZATION          11
#define DROP_TCP_NORMALIZATION         12
#define DROP_TCP_RST_WITH_INVALID_ACK_NUM         13
#define DROP_TCP_NON_SYN_FIRST_PKT     14
#define DROP_ICMP_NORMALIZATION        15
#define DROP_ICMP_SRC_QUENCH_MSG       16
#define DROP_ICMP_REDIRECT_MSG         17
#define DROP_ICMP_INFO_REQ_MSG         18
#define DROP_ICMP_ADDR_REQ_MSG         19
#define DROP_ICMP_TRACEROUTE_MSG       20
#define DROP_ICMP_RSVD_TYPE_MSG        21
#define DROP_INPUT_PROPERTIES_MISS     22
#define DROP_TCP_OUT_OF_WINDOW         23
#define DROP_TCP_SPLIT_HANDSHAKE       24
#define DROP_TCP_WIN_ZERO_DROP         25
#define DROP_TCP_ACK_ERR               26
#define DROP_TCP_DATA_AFTER_FIN        27
#define DROP_TCP_NON_RST_PKT_AFTER_RST 28
#define DROP_TCP_INVALID_RESPONDER_FIRST_PKT 29
#define DROP_TCP_UNEXPECTED_PKT        30
#define DROP_SRC_LIF_MISMATCH          31
#define DROP_PARSER_ICRC_ERR           32
#define DROP_PARSER_LEN_ERR            33
#define DROP_PARSER_CSUM_ERR           34
#define DROP_MIN                       DROP_INPUT_MAPPING
#define DROP_MAX                       DROP_PARSER_CSUM_ERR

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
/* TCP flag masks                                                            */
/*****************************************************************************/
#define TCP_FLAGS_MASK_FIN             1
#define TCP_FLAGS_MASK_SYN             2
#define TCP_FLAGS_MASK_RST             4
#define TCP_FLAGS_MASK_PSH             8
#define TCP_FLAGS_MASK_ACK             16
#define TCP_FLAGS_MASK_URG             32
#define TCP_FLAGS_MASK_ECE             64
#define TCP_FLAGS_MASK_CWR             128
#define TCP_FLAGS_MASK_NS              256

/*****************************************************************************/
/* ICMP related                                                              */
/*****************************************************************************/
#define ICMP_NEIGHBOR_SOLICITATION     135
#define ICMP_NEIGHBOR_ADVERTISEMENT    136
#define ICMP_ECHO_REQ_TYPE_CODE        0x0800
#define ICMP_ECHO_REPLY_TYPE_CODE      0x0000

/*****************************************************************************/
/* ICMPv6 related                                                              */
/*****************************************************************************/
#define ICMPV6_ECHO_REQ_TYPE_CODE        0x8000
#define ICMPV6_ECHO_REPLY_TYPE_CODE      0x8100

/*****************************************************************************/
/* DHCP related                                                              */
/*****************************************************************************/
#define DHCP_CLIENT_PORT               67
#define DHCP_SERVER_PORT               68

/*****************************************************************************/
/* Policer color                                                           */
/*****************************************************************************/
#define POLICER_COLOR_GREEN            0
#define POLICER_COLOR_RED              1
#define POLICER_COLOR_YELLOW           2

/*****************************************************************************/
/* Recirc reason codes                                                       */
/*****************************************************************************/
#define RECIRC_FLOW_HASH_OVERFLOW      0
#define RECIRC_VM_BOUNCE               1

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
#define CSUM_HDR_UDP                   1
#define CSUM_HDR_TCP                   2
#define CSUM_HDR_UDP_OPT_OCS           0

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

/*****************************************************************************/
/* P4+ app types                                                             */
/*****************************************************************************/
#define P4PLUS_APPTYPE_DEFAULT         0
#define P4PLUS_APPTYPE_CLASSIC_NIC     1
#define P4PLUS_APPTYPE_RDMA            2
#define P4PLUS_APPTYPE_TCPTLS          3
#define P4PLUS_APPTYPE_IPSEC           4
#define P4PLUS_APPTYPE_NDE             5
#define P4PLUS_APPTYPE_STORAGE         6
#define P4PLUS_APPTYPE_TELEMETRY       7
#define P4PLUS_APPTYPE_CPU             8
#define P4PLUS_APPTYPE_RAW_REDIR       9
#define P4PLUS_APPTYPE_P4PT            10
#define P4PLUS_APP_TYPE_MIN            P4PLUS_APPTYPE_DEFAULT
#define P4PLUS_APP_TYPE_MAX            P4PLUS_APPTYPE_P4PT

#define CAPRI_GLOBAL_INTRINSIC_HDR_SZ  17
#define CAPRI_P4_INTRINSIC_HDR_SZ      5
#define P4_RECIRC_HDR_SZ               5

/*****************************************************************************/
/* P4 to P4+ header size constants (in bytes)                                */
/*****************************************************************************/
#define CAPRI_RXDMA_INTRINSIC_HDR_SZ   15
#define P4PLUS_ROCE_HDR_SZ             20
#define P4PLUS_TCP_PROXY_HDR_SZ        71 /* 39 app hdr + 32 sack */
#define P4PLUS_CLASSIC_NIC_HDR_SZ      48
#define P4PLUS_CPU_HDR_SZ              40
#define P4PLUS_CPU_PKT_SZ              37
#define P4PLUS_IPSEC_HDR_SZ            18
#define P4PLUS_RAW_REDIR_HDR_SZ        40
#define P4PLUS_P4PT_HDR_SZ             7

/*****************************************************************************/
/* P4 fixed header size constants (in bytes)                                */
/*****************************************************************************/
#define UDP_HDR_SIZE                   8
#define IPV6_BASE_HDR_SIZE             40
#define IPV4_BASE_HDR_SIZE             20

/*****************************************************************************/
/* P4+ to P4 header size constants (in bytes)                                */
/*****************************************************************************/
#define CAPRI_TXDMA_INTRINSIC_HDR_SZ   9
#define P4PLUS_TO_P4_HDR_SZ            10

/*****************************************************************************/
/* Classic NIC header flags                                                  */
/*****************************************************************************/
#define CLASSIC_NIC_HEADER_FLAGS_NONE          0x000
#define CLASSIC_NIC_HEADER_FLAGS_IPV4          0x001
#define CLASSIC_NIC_HEADER_FLAGS_IPV4_TCP      0x003
#define CLASSIC_NIC_HEADER_FLAGS_IPV4_UDP      0x005
#define CLASSIC_NIC_HEADER_FLAGS_IPV6          0x008
#define CLASSIC_NIC_HEADER_FLAGS_IPV6_TCP      0x018
#define CLASSIC_NIC_HEADER_FLAGS_IPV6_UDP      0x028

/*****************************************************************************/
/* CPU flags                                                                 */
/*****************************************************************************/
#define CPU_FLAGS_VLAN_VALID                   0x01
#define CPU_FLAGS_IPV4_VALID                   0x02
#define CPU_FLAGS_IPV6_VALID                   0x04
#define CPU_FLAGS_IP_OPTIONS_PRESENT           0x08
#define CPU_FLAGS_TCP_OPTIONS_PRESENT          0x10

/*****************************************************************************/
/* CPU header packet types                                                   */
/*****************************************************************************/
#define CPU_PACKET_TYPE_NONE                   0
#define CPU_PACKET_TYPE_IPV4                   1
#define CPU_PACKET_TYPE_IPV6                   2

/*****************************************************************************/
/* TCP OPTIONS                                                               */
/*****************************************************************************/
#define CPU_TCP_OPTIONS_WINDOW_SCALE          0x01
#define CPU_TCP_OPTIONS_MSS                   0x02
#define CPU_TCP_OPTIONS_TIMESTAMP             0x04
#define CPU_TCP_OPTIONS_SACK_PERMITTED        0x08

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
#define P4_NW_MCAST_INDEX_FIN_COPY                     1
#define P4_NW_MCAST_INDEX_RST_COPY                     2
#define P4_NW_MCAST_INDEX_FLOW_REL_COPY                3
#define P4_NW_MCAST_INDEX_P4PT_COPY                    4


/*****************************************************************************/
/* Defines for repl_type field in tm_replication_data_t                      */
/*****************************************************************************/
#define TM_REPL_TYPE_DEFAULT            0  // This is for normal multicast replication
#define TM_REPL_TYPE_TO_CPU_REL_COPY    1  // This is for copy to cpu using replication and reliable
#define TM_REPL_TYPE_HONOR_INGRESS      2  // Normal forwarding, Honor the ingress flow decided rewrites.
