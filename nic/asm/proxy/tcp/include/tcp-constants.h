#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "proxy-constants.h"
#include "tcp_common.h"

#define TCP_OOO_QUEUE_SIZE   256

#define TCPHDR_FIN_BIT 0
#define TCPHDR_SYN_BIT 1
#define TCPHDR_RST_BIT 2
#define TCPHDR_PSH_BIT 3
#define TCPHDR_ACK_BIT 4
#define TCPHDR_URG_BIT 5
#define TCPHDR_ECE_BIT 6
#define TCPHDR_CWR_BIT 7

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

#define FLAG_DATA_BIT               0 /* Incoming frame contained data.          */
#define FLAG_WIN_UPDATE_BIT         1 /* Incoming ACK was a window update.       */
#define FLAG_ECE_BIT                2 /* ECE in this ACK                         */
#define FLAG_SLOWPATH_BIT           3 /* Do not skip RFC checks for window update*/
#define FLAG_UPDATE_TS_RECENT_BIT   4 /* Update ts_recent                        */

#define FLAG_DATA               (1 << FLAG_DATA_BIT)
#define FLAG_WIN_UPDATE         (1 << FLAG_WIN_UPDATE_BIT)
#define FLAG_ECE                (1 << FLAG_ECE_BIT)
#define FLAG_SLOWPATH           (1 << FLAG_SLOWPATH_BIT)
#define FLAG_UPDATE_TS_RECENT   (1 << FLAG_UPDATE_TS_RECENT_BIT)

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

#define TCPOPT_NOP              1   /* Padding */
#define TCPOPT_EOL              0   /* End of options */
#define TCPOPT_MSS              2   /* Segment size negotiating */
#define TCPOPT_WINDOW           3   /* Window scaling */
#define TCPOPT_SACK_PERM        4   /* SACK Permitted */
#define TCPOPT_SACK             5   /* SACK Block */
#define TCPOPT_TIMESTAMP        8   /* Better RTT estimations/PAWS */
#define TCPOPT_MD5SIG           19  /* MD5 Signature (RFC2385) */
#define TCPOPT_FASTOPEN         34  /* Fast open (RFC7413) */
#define TCPOPT_EXP              254 /* Experimental */
/* Magic number to be after the option value for sharing TCP
 * experimental options. See draft-ietf-tcpm-experimental-options-00.txt
 */

/*
 *     TCP option lengths
 */
#define TCPOLEN_MSS            		4
#define TCPOLEN_WINDOW         		3
#define TCPOLEN_SACK_PERM      		2
#define TCPOLEN_TIMESTAMP      		10
#define TCPOLEN_MD5SIG         		18
#define TCPOLEN_FASTOPEN_BASE  		2
#define TCPOLEN_EXP_FASTOPEN_BASE  	4

#define TCP_PENDING_TXDMA_ACK_SEND              0x1
#define TCP_PENDING_TXDMA_SND_UNA_UPDATE        0x2
#define TCP_PENDING_TXDMA_FAST_RETRANS          0x4
#define TCP_PENDING_TXDMA_DEL_ACK               0x8

#define TCP_PENDING_TXDMA_ACK_SEND_BIT          0
#define TCP_PENDING_TXDMA_SND_UNA_UPDATE_BIT    1
#define TCP_PENDING_TXDMA_FAST_RETRANS_BIT      2
#define TCP_PENDING_TXDMA_DEL_ACK_BIT           3

#define PENDING_RETX_CLEANUP                    0x1

/*
 * Congestion Control Related
 */
#define TCP_CC_ACK_SIGNAL_BIT                   0
#define TCP_CC_DUPACK_SIGNAL_BIT                1
#define TCP_CC_PARTIAL_ACK_SIGNAL_BIT           2
#define TCP_CC_ECE_SIGNAL_BIT                   3

#define TCP_CC_ACK_SIGNAL                       (1 << TCP_CC_ACK_SIGNAL_BIT)
#define TCP_CC_DUPACK_SIGNAL                    (1 << TCP_CC_DUPACK_SIGNAL_BIT)
#define TCP_CC_PARTIAL_ACK_SIGNAL               (1 << TCP_CC_PARTIAL_ACK_SIGNAL_BIT)
#define TCP_CC_ECE_SIGNAL                       (1 << TCP_CC_ECE_SIGNAL_BIT)

// CC flags
#define TCP_CCF_FAST_RECOVERY_BIT               0
#define TCP_CCF_CONG_RECOVERY_BIT               1

#define TCP_CCF_FAST_RECOVERY                   (1 << TCP_CCF_FAST_RECOVERY_BIT)
#define TCP_CCF_CONG_RECOVERY                   (1 << TCP_CCF_CONG_RECOVERY_BIT)


#endif /* #ifndef CONSTANTS_H */
