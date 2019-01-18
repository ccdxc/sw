#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "proxy-constants.h"
#include "tcp_common.h"

#define TCP_OOO_QUEUE_SIZE   256

#define NIC_PAGE_HDR_SIZE                    0         /* sizeof(nic_page_hdr_t) */

#define NIC_DESC_ENTRY_ADDR_OFFSET           8          /* &((nic_desc_entry_t *)0)->addr */
#define NIC_DESC_ENTRY_OFF_OFFSET            0xc        /* &((nic_desc_entry_t *)0)->offset */
#define NIC_DESC_ENTRY_LEN_OFFSET            0xe        /* &((nic_desc_entry_t *)0)->len */
#define NIC_DESC_NUM_ENTRIES_OFFSET          0x40       /* &((nic_desc_t *)0)->num_entries */
#define NIC_DESC_ENTRY_OFFSET(_i)            &((nic_desc_t *)0)->entry[(_i)]
#define NIC_DESC_SCRATCH_OFFSET              0          /* &((nic_desc_t *)0)->scratch[0] */
#define ETH_IP_HDR_SIZE                      34         /* (sizeof(struct ethhdr) + sizeof(struct iphdr) */
#define ETH_IP_VLAN_HDR_SIZE                 38         /* (sizeof(struct ethhdr) + sizeof(struct iphdr) */
#define ETH_IP_TCP_HDR_SIZE                  54         /* (sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr))*/
#define TCPIP_HDR_SIZE                       40         /* (sizeof(struct iphdr) + sizeof(struct tcphdr)) */
#define TCP_HDR_SIZE                         20
#define ETHHDR_SMAC_OFFSET                   6          /* &((struct ethhdr *)0)->h_source */
#define ETHHDR_PROTO_OFFSET                  12         /* &((struct ethhdr *)0)->h_proto */
#define IPHDR_OFFSET                         14         /* sizeof(struct ethhdr) */
#define ETH_P_IP                             0x0800     /* Internet Protocol packet     */
#define ETH_P_ARP                            0x0806     /* Address Resolution packet    */

#define SCHED_PENDING_BIT_SHIFT              2
#define SCHED_PENDING_BIT_BASE               0xabab0000 

#define RCV_MSS_SHFT_BASE                    1

/* Standard well-defined IP protocols.  */

#define  IPPROTO_IP  0               /* Dummy protocol for TCP               */

#define  IPPROTO_ICMP 1             /* Internet Control Message Protocol    */

#define  IPPROTO_IGMP 2             /* Internet Group Management Protocol   */

#define  IPPROTO_IPIP 4             /* IPIP tunnels (older KA9Q tunnels use 94) */

#define  IPPROTO_TCP  6              /* Transmission Control Protocol        */

#define  IPPROTO_EGP  8              /* Exterior Gateway Protocol            */

#define  IPPROTO_PUP 12             /* PUP protocol                         */

#define  IPPROTO_UDP 17             /* User Datagram Protocol               */

#define  IPPROTO_IDP 22             /* XNS IDP protocol                     */

#define  IPPROTO_TP 29              /* SO Transport Protocol Class 4        */

#define  IPPROTO_DCCP 33            /* Datagram Congestion Control Protocol */

#define  IPPROTO_IPV6 41            /* IPv6-in-IPv4 tunnelling              */

#define  IPPROTO_RSVP 46            /* RSVP Protocol                        */

#define  IPPROTO_GRE 47             /* Cisco GRE tunnels (rfc 1701,1702)    */

#define  IPPROTO_ESP 50             /* Encapsulation Security Payload protocol */

#define  IPPROTO_AH 51              /* Authentication Header protocol       */

#define  IPPROTO_MTP 92             /* Multicast Transport Protocol         */

#define  IPPROTO_BEETPH  94          /* IP option pseudo header for BEET     */

#define  IPPROTO_ENCAP  98           /* Encapsulation Header                 */

#define  IPPROTO_PIM 103            /* Protocol Independent Multicast       */

#define  IPPROTO_COMP 108           /* Compression Header Protocol          */

#define  IPPROTO_SCTP  132           /* Stream Control Transport Protocol    */

#define  IPPROTO_UDPLITE  136        /* UDP-Lite (RFC 3828)                  */

#define  IPPROTO_MPLS 137           /* MPLS in IP (RFC 4023)                */

#define  IPPROTO_RAW  255            /* Raw IP packets                       */


#define CACHE_LINE_SIZE 64
#define NIC_PAGE_SIZE (128 * CACHE_LINE_SIZE)
#define NIC_PAGE_SIZE_SHFT 13
#define NIC_PAGE_HEADROOM (17 * CACHE_LINE_SIZE)
#define NIC_CPU_HDR_SIZE 328
#define NIC_CPU_HDR_SIZE_BYTES 41 /* NIC_CPU_HDR_SIZE/8 */
#define NIC_CPU_HDR_OFFSET 1047 /* (NIC_PAGE_HEADROOM - NIC_CPU_HDR_SIZE_BYTES) */
#define PKT_DESC_AOL_OFFSET 64


#define ETH_ALEN 6

#define ACK_RATIO_SHIFT	4
#define TCP_ECN_OK              1
#define TCP_ECN_QUEUE_CWR       2
#define TCP_ECN_DEMAND_CWR      4
#define TCP_ECN_SEEN            8

#define  INET_ECN_NOT_ECT  0
#define  INET_ECN_ECT_1  1
#define  INET_ECN_ECT_0  2
#define  INET_ECN_CE 3
#define  INET_ECN_MASK 3

#define TCPHDR_FIN 0x01
#define TCPHDR_SYN 0x02
#define TCPHDR_RST 0x04
#define TCPHDR_PSH 0x08
#define TCPHDR_ACK 0x10
#define TCPHDR_URG 0x20
#define TCPHDR_ECE 0x40
#define TCPHDR_CWR 0x80
#define TCPHDR_RSVD 0xF00
#define TCPHDR_HLEN_MASK 0xF000

#define TCPHDR_HP_FLAG_BITS (~TCPHDR_PSH)
#define TCPHDR_HP_BITS (~(TCPHDR_RSVD | TCPHDR_PSH))

#define TCPHDR_SYN_ECN  (TCPHDR_SYN | TCPHDR_ECE | TCPHDR_CWR)


#define FLAG_DATA               0x01 /* Incoming frame contained data.          */
#define FLAG_WIN_UPDATE         0x02 /* Incoming ACK was a window update.       */
#define FLAG_ECE                0x04 /* ECE in this ACK                         */
#define FLAG_SLOWPATH           0x08 /* Do not skip RFC checks for window update.*/
#define FLAG_SND_UNA_ADVANCED   0x10 /* Snd_una was changed (!= FLAG_DATA_ACKED) */

// Without SACK DATA_ACKED is same as SND_UNA_ADVANCED.
#define FLAG_DATA_ACKED         FLAG_SND_UNA_ADVANCED
#define FLAG_SYN_ACKED          0 // not handled in datapath

#define FLAG_UPDATE_TS_RECENT   0x20 /* tcp_replace_ts_recent() */
#define FLAG_RETRANS_DATA_ACKED 0x40 /* This ACK acknowledged new data.         */
#define FLAG_LOST_RETRANS       0x80 /* This ACK acknowledged new data.         */

#define FLAG_ACKED              (FLAG_DATA_ACKED|FLAG_SYN_ACKED)
#define FLAG_NOT_DUP            (FLAG_DATA|FLAG_WIN_UPDATE|FLAG_ACKED)
#define FLAG_CA_ALERT           (FLAG_DATA_SACKED|FLAG_ECE)
#define FLAG_FORWARD_PROGRESS   (FLAG_ACKED|FLAG_DATA_SACKED)

/*
 * Never offer a window over 32767 without using window scaling. Some
 * poor stacks do signed 16bit maths!
 */
#define MAX_TCP_WINDOW          32767U

/* Minimal accepted MSS. It is (60+60+8) - (20+20). */
#define TCP_MIN_MSS             88U

/* The least MTU to use for probing */
#define TCP_BASE_MSS            1024

/* probing interval, default to 10 minutes as per RFC4821 */
#define TCP_PROBE_INTERVAL      600

/* Specify interval when tcp mtu probing will stop */
#define TCP_PROBE_THRESHOLD     8

/* After receiving this amount of duplicate ACKs fast retransmit starts. */
#define TCP_FASTRETRANS_THRESH 3

/* Maximal number of ACKs sent quickly to accelerate slow-start. */
#define TCP_MAX_QUICKACKS       16


#define LOW_WINDOW 14
#define MAX_INCREMENT 16
#define MAX_INCREMENT_SHIFT 4
#define BETA 819
#define SMOOTH_PART 20
#define BICTCP_B 4
#define BICTCP_B_SHIFT 2

#define ACK_RATIO_SHIFT 4

/*
 *  TCP option
 */

#define TCPOPT_NOP      1   /* Padding */
#define TCPOPT_EOL      0   /* End of options */
#define TCPOPT_MSS      2   /* Segment size negotiating */
#define TCPOPT_WINDOW       3   /* Window scaling */
#define TCPOPT_SACK_PERM        4       /* SACK Permitted */
#define TCPOPT_SACK             5       /* SACK Block */
#define TCPOPT_TIMESTAMP    8   /* Better RTT estimations/PAWS */
#define TCPOPT_MD5SIG       19  /* MD5 Signature (RFC2385) */
#define TCPOPT_FASTOPEN     34  /* Fast open (RFC7413) */
#define TCPOPT_EXP      254 /* Experimental */
/* Magic number to be after the option value for sharing TCP
 * experimental options. See draft-ietf-tcpm-experimental-options-00.txt
 */

/*
 *     TCP option lengths
 */

#define TCPOLEN_MSS            4
#define TCPOLEN_WINDOW         3
#define TCPOLEN_SACK_PERM      2
#define TCPOLEN_TIMESTAMP      10
#define TCPOLEN_MD5SIG         18
#define TCPOLEN_FASTOPEN_BASE  2
#define TCPOLEN_EXP_FASTOPEN_BASE  4

#define TCP_PENDING_TXDMA_ACK_SEND          0x1
#define TCP_PENDING_TXDMA_SND_UNA_UPDATE    0x2
#define TCP_PENDING_TXDMA_FAST_RETRANS      0x4
#define TCP_PENDING_TXDMA_DEL_ACK           0x8

#define TCP_PENDING_TXDMA_ACK_SEND_BIT         0
#define TCP_PENDING_TXDMA_SND_UNA_UPDATE_BIT   1
#define TCP_PENDING_TXDMA_FAST_RETRANS_BIT     2
#define TCP_PENDING_TXDMA_DEL_ACK_BIT          3

#define PENDING_RETX_CLEANUP                0x1

/*
 * Congestion Control Related
 */
#define TCP_CC_ACK_BIT                      0
#define TCP_CC_DUPACK_BIT                   1
#define TCP_CC_PARTIAL_ACK_BIT              2

#define TCP_CC_ACK                          (1 << TCP_CC_ACK_BIT)
#define TCP_CC_DUPACK                       (1 << TCP_CC_DUPACK_BIT)
#define TCP_CC_PARTIAL_ACK                  (1 << TCP_CC_PARTIAL_ACK_BIT)

// CC flags
#define TCP_CCF_FAST_RECOVERY_BIT           0
#define TCP_CCF_CONG_RECOVERY_BIT           1

#define TCP_CCF_FAST_RECOVERY               (1 << TCP_CCF_FAST_RECOVERY_BIT)
#define TCP_CCF_CONG_RECOVERY               (1 << TCP_CCF_CONG_RECOVERY_BIT)


#endif /* #ifndef CONSTANTS_H */
