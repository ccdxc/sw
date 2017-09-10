#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "proxy-constants.h"

#define TCPCB_SACKED_ACKED	0x01	/* SKB ACK'd by a SACK block	*/
#define TCPCB_SACKED_RETRANS	0x02	/* SKB retransmitted		*/
#define TCPCB_LOST		0x04	/* SKB is lost			*/
#define TCPCB_TAGBITS		0x07	/* All tag bits			*/
#define TCPCB_REPAIRED		0x10	/* SKB repaired (no skb_mstamp)	*/
#define TCPCB_EVER_RETRANS	0x80	/* Ever retransmitted frame	*/
#define TCPCB_RETRANS		(TCPCB_SACKED_RETRANS|TCPCB_EVER_RETRANS| \
				TCPCB_REPAIRED)
#define IP_LEN_OFFSET 16
#define TCP_SEQ_OFFSET 40

#define MAX_ENTRIES_PER_DESC 16
#define MAX_ENTRIES_PER_DESC_MASK 0xF

#define NIC_PAGE_HDR_SIZE                    52         /* sizeof(nic_page_hdr_t) */
#define NIC_DESC_ENTRY_0_OFFSET              64         /* &((nic_desc_t *)0)->entry[0]*/
#define NIC_DESC_ENTRY_L_OFFSET              96         /* &((nic_desc_t *)0)->entry[1]*/

#define NIC_DESC_ENTRY_ADDR_OFFSET           8          /* &((nic_desc_entry_t *)0)->addr */
#define NIC_DESC_ENTRY_OFF_OFFSET            0xc        /* &((nic_desc_entry_t *)0)->offset */
#define NIC_DESC_ENTRY_LEN_OFFSET            0xe        /* &((nic_desc_entry_t *)0)->len */
#define NIC_DESC_NUM_ENTRIES_OFFSET          0x40       /* &((nic_desc_t *)0)->num_entries */
#define NIC_DESC_ENTRY_OFFSET(_i)            &((nic_desc_t *)0)->entry[(_i)]
#define NIC_DESC_SCRATCH_OFFSET              0          /* &((nic_desc_t *)0)->scratch[0] */
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



#define PAGE_CELL_SIZE 32 /* A page is divided into 32B cells */
#define PAGE_CELL_SIZE_SHFT 5 

#define CACHE_LINE_SIZE 64
#define NIC_PAGE_SIZE (128 * CACHE_LINE_SIZE)
#define NIC_PAGE_SIZE_SHFT 13
#define NIC_PAGE_HEADROOM (17 * CACHE_LINE_SIZE)

#define ETH_ALEN 6

/* This is the max number of SACKS that we'll generate and process. It's safe
 * to increase this, although since:
 *   size = TCPOLEN_SACK_BASE_ALIGNED (4) + n * TCPOLEN_SACK_PERBLOCK (8)
 * only four options will fit in a standard TCP header */
#define TCP_NUM_SACKS 4

/*These are used to set the sack_ok field in struct tcp_options_received */
#define TCP_SACK_SEEN     (1 << 0)   /*1 = peer is SACK capable, */
#define TCP_FACK_ENABLED  (1 << 1)   /*1 = FACK is enabled locally*/
#define TCP_DSACK_SEEN    (1 << 2)   /*1 = DSACK was received from peer*/

#define TCP_MSS_DEFAULT 536U

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

#define TCP_INFINITE_SSTHRESH   0x7fffffff

#define  TCP_CA_Open 0
#define TCPF_CA_Open    (1<<TCP_CA_Open)
#define  TCP_CA_Disorder 1
#define TCPF_CA_Disorder (1<<TCP_CA_Disorder)
#define  TCP_CA_CWR 2
#define TCPF_CA_CWR     (1<<TCP_CA_CWR)
#define  TCP_CA_Recovery 3
#define TCPF_CA_Recovery (1<<TCP_CA_Recovery)
#define  TCP_CA_Loss 4
#define TCPF_CA_Loss    (1<<TCP_CA_Loss)

#define TCPHDR_FIN 0x01
#define TCPHDR_SYN 0x02
#define TCPHDR_RST 0x04
#define TCPHDR_PSH 0x08
#define TCPHDR_ACK 0x10
#define TCPHDR_URG 0x20
#define TCPHDR_ECE 0x40
#define TCPHDR_CWR 0x80

#define TCPHDR_SYN_ECN  (TCPHDR_SYN | TCPHDR_ECE | TCPHDR_CWR)



#define FLAG_DATA               0x01 /* Incoming frame contained data.          */
#define FLAG_WIN_UPDATE         0x02 /* Incoming ACK was a window update.       */
#define FLAG_DATA_ACKED         0x04 /* This ACK acknowledged new data.         */
#define FLAG_RETRANS_DATA_ACKED 0x08 /* "" "" some of which was retransmitted.  */
#define FLAG_SYN_ACKED          0x10 /* This ACK acknowledged SYN.              */
#define FLAG_DATA_SACKED        0x20 /* New SACK.                               */
#define FLAG_ECE                0x40 /* ECE in this ACK                         */
#define FLAG_LOST_RETRANS       0x80 /* This ACK marks some retransmission lost */
#define FLAG_SLOWPATH           0x100 /* Do not skip RFC checks for window update.*/
#define FLAG_ORIG_SACK_ACKED    0x200 /* Never retransmitted data are (s)acked  */
#define FLAG_SND_UNA_ADVANCED   0x400 /* Snd_una was changed (!= FLAG_DATA_ACKED) */
#define FLAG_DSACKING_ACK       0x800 /* SACK blocks contained D-SACK info */
#define FLAG_SACK_RENEGING      0x2000 /* snd_una advanced to a sacked seq */
#define FLAG_UPDATE_TS_RECENT   0x4000 /* tcp_replace_ts_recent() */

#define FLAG_ACKED              (FLAG_DATA_ACKED|FLAG_SYN_ACKED)
#define FLAG_NOT_DUP            (FLAG_DATA|FLAG_WIN_UPDATE|FLAG_ACKED)
#define FLAG_CA_ALERT           (FLAG_DATA_SACKED|FLAG_ECE)
#define FLAG_FORWARD_PROGRESS   (FLAG_ACKED|FLAG_DATA_SACKED)

#define REXMIT_NONE     0 /* no loss recovery to do */
#define REXMIT_LOST     1 /* retransmit packets marked lost */
#define REXMIT_NEW      2 /* FRTO-style transmit of unsent/new packets */

/* Events passed to congestion control interface */

#define   CA_EVENT_TX_START 0      /* first transmit when no packets in flight */
#define   CA_EVENT_CWND_RESTART 1  /* congestion window restart */
#define   CA_EVENT_COMPLETE_CWR 2  /* end of congestion recovery */
#define   CA_EVENT_LOSS 4          /* loss timeout */
#define   CA_EVENT_ECN_NO_CE 5     /* ECT set, but not CE marked */
#define   CA_EVENT_ECN_IS_CE 6     /* received CE marked IP packet */
#define   CA_EVENT_DELAYED_ACK 7   /* Delayed ack is sent */
#define   CA_EVENT_NON_DELAYED_ACK 8


 /* Information about inbound ACK, passed to cong_ops->in_ack_event() */
#define   CA_ACK_SLOWPATH         (1 << 0)     /* In slow path processing */
#define   CA_ACK_WIN_UPDATE       (1 << 1)     /* ACK updated window */
#define   CA_ACK_ECE              (1 << 2)     /* ECE bit is set on ack */

 /*
  * Interface for adding new TCP congestion control handlers
  */
#define TCP_CA_NAME_MAX 16
#define TCP_CA_MAX      128
#define TCP_CA_BUF_MAX  (TCP_CA_NAME_MAX*TCP_CA_MAX)

#define TCP_CA_UNSPEC   0

 /* Algorithm can be set on socket without CAP_NET_ADMIN privileges */
#define TCP_CONG_NON_RESTRICTED 0x1
 /* Requires ECN/ECT set on all packets */
#define TCP_CONG_NEEDS_ECN      0x2

#define HZ (1)

#define TCP_DELACK_MAX  ((HZ/5))      /* maximal time to delay before sending an ACK */
#if HZ >= 100
#define TCP_DELACK_MIN  ((HZ/25))     /* minimal time to delay before sending an ACK */
#define TCP_ATO_MIN     ((HZ/25))
#else
#define TCP_DELACK_MIN  4
#define TCP_ATO_MIN     4
#endif

#define TCP_RTO_MAX     ((120*HZ)) /* 120s */
#define TCP_RTO_MIN     ((HZ/5))   /* 200ms */

#define TCP_KTO_MIN     ((75*HZ)) /* 75s */

#define TCP_RESOURCE_PROBE_INTERVAL ((HZ/2U)) /* Maximal interval between probes
                                                         * for local resources.
                                                         */

#define USEC_PER_SEC 1000000
#define ICSK_TIME_RETRANS       1       /* Retransmit timer */
#define ICSK_TIME_DACK          2       /* Delayed ack timer */
#define ICSK_TIME_PROBE0        3       /* Zero window probe timer */
#define ICSK_TIME_EARLY_RETRANS 4       /* Early retransmit timer */
#define ICSK_TIME_LOSS_PROBE    5       /* Tail loss probe timer */

#define  ICSK_ACK_SCHED  1
#define  ICSK_ACK_TIMER  2
#define  ICSK_ACK_PUSHED 4
#define  ICSK_ACK_PUSHED2  8

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

/* debug_dol encodings - need to match defines in tcp_proxy.py */
#define TCP_DDOL_PKT_TO_SERQ            1
#define TCP_DDOL_TEST_ATOMIC_STATS      2

#endif /* #ifndef CONSTANTS_H */
