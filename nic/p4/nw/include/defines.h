#define TRUE                           1
#define FALSE                          0

#define LIF_BIT_WIDTH                  11

#define CPU_VFID                       1023

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

/*****************************************************************************/
/* TM reserved queues                                                        */
/*****************************************************************************/
#define TM_P4_IG_RECIRC_QUEUE          31 /* Recirc queue in the P4 IG port */

/*****************************************************************************/
/* TM instance type - TBD: Need to get correct values from HW Spec           */
/*****************************************************************************/
#define TM_INSTANCE_TYPE_NORMAL        0
#define TM_INSTANCE_TYPE_MULTICAST     1
#define TM_INSTANCE_TYPE_SPAN          2
#define TM_INSTANCE_TYPE_CPU           3

/*****************************************************************************/
/* flow key types                                                            */
/*****************************************************************************/
#define FLOW_KEY_LOOKUP_TYPE_NONE              0
#define FLOW_KEY_LOOKUP_TYPE_IPV4              1
#define FLOW_KEY_LOOKUP_TYPE_IPV6              2
#define FLOW_KEY_LOOKUP_TYPE_MAC               3
#define FLOW_KEY_LOOKUP_TYPE_FROM_VM_BOUNCE    4
#define FLOW_KEY_LOOKUP_TYPE_TO_VM_BOUNCE      5

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
#define DROP_EGRESS_POLICER            6
#define DROP_NACL                      7
#define DROP_MALFORMED_PKT             8
#define DROP_PING_OF_DEATH             9
#define DROP_FRAGMENT_TOO_SMALL        10
#define DROP_IP_NORMALIZATION          11
#define DROP_TCP_NORMALIZATION         12
#define DROP_TCP_XMAS_TREE_PKT         13
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
#define DROP_TCP_UNEXPECTED_SYN        30

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
#define TCP_UNEXPECTED_SYN             0x1000


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
#define ICMP_ECHO_REQ_TYPE_CODE        0x0800
#define ICMP_ECHO_REPLY_TYPE_CODE      0x0000

/*****************************************************************************/
/* Policer color                                                           */
/*****************************************************************************/
#define POLICER_COLOR_GREEN            0
#define POLICER_COLOR_YELLOW           1
#define POLICER_COLOR_RED              2

/*****************************************************************************/
/* Recirc reason codes                                                       */
/*****************************************************************************/
#define RECIRC_FLOW_HASH_OVERFLOW      0
#define RECIRC_VM_BOUNCE               1

// IPSEC
#define IPSEC_HEADER_AH                1
#define IPSEC_HEADER_ESP               2

//Normalization
#define NORMALIZATION_ACTION_NONE      0
#define NORMALIZATION_ACTION_DROP      1
#define NORMALIZATION_ACTION_EDIT      2

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

#define IP_FLAGS_RSVD_MASK             1
#define IP_FLAGS_DF_MASK               2
#define IP_FLAGS_MF_MASK               4

// Different types of DDoS attacks that can be configured.
#define DDOS_TYPE_ANY                  1
#define DDOS_TYPE_SYN                  2
#define DDOS_TYPE_SYN_ACK              3
#define DDOS_TYPE_RST                  4
#define DDOS_TYPE_FIN                  5
#define DDOS_TYPE_ICMP                 6
#define DDOS_TYPE_UDP                  7
#define DDOS_TYPE_IP_FRAG              8

#define DDOS_TYPE_MASK_ANY             (1 << (DDOS_TYPE_ANY - 1))
#define DDOS_TYPE_MASK_SYN             (1 << (DDOS_TYPE_SYN - 1))
#define DDOS_TYPE_MASK_SYN_ACK         (1 << (DDOS_TYPE_SYN_ACK - 1))
#define DDOS_TYPE_MASK_RST             (1 << (DDOS_TYPE_RST - 1))
#define DDOS_TYPE_MASK_FIN             (1 << (DDOS_TYPE_FIN - 1))
#define DDOS_TYPE_MASK_ICMP            (1 << (DDOS_TYPE_ICMP - 1))
#define DDOS_TYPE_MASK_UDP             (1 << (DDOS_TYPE_UDP - 1))
#define DDOS_TYPE_MASK_IP_FRAG         (1 << (DDOS_TYPE_IP_FRAG - 1))

/*****************************************************************************/
/* P4+ app types                                                             */
/*****************************************************************************/
#define P4PLUS_APPTYPE_CLASSIC_NIC  1
#define P4PLUS_APPTYPE_RDMA         2
#define P4PLUS_APPTYPE_TCPTLS       3
#define P4PLUS_APPTYPE_IPSEC        4
#define P4PLUS_APPTYPE_NDE          5
#define P4PLUS_APPTYPE_STORAGE      6
#define P4PLUS_APPTYPE_TELEMETRY    7
#define P4PLUS_APP_TYPE_MIN         P4PLUS_APPTYPE_CLASSIC_NIC
#define P4PLUS_APP_TYPE_MAX         P4PLUS_APPTYPE_TELEMETRY

/*****************************************************************************/
/* P4+ header size constants                                                 */
/*****************************************************************************/
#define CAPRI_GLOBAL_INTRINSIC_HDR_SZ  17
#define CAPRI_RXDMA_INTRINSIC_HDR_SZ   15
#define P4PLUS_ROCE_HDR_SZ             19
#define P4PLUS_TCP_PROXY_HDR_SZ        61

#define IPSEC_MODE_TUNNEL              1
#define IPSEC_MODE_TRANSPORT           2

#define IPSEC_ENCAP_TYPE_AH            1
#define IPSEC_ENCAP_TYPE_ESP           2
#define IPSEC_ENCAP_TYPE_AH_ESP        3

#define IPSEC_L3_PROTOCOL_IPV4         1
