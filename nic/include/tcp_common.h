#ifndef __TCP_COMMON_H__
#define __TCP_COMMON_H__

#define TCP_SCHED_FLAG_FT             0x1
#define TCP_SCHED_FLAG_ST             0x2
#define TCP_SCHED_FLAG_FT_ERR         0x4
#define TCP_SCHED_FLAG_ST_ERR         0x8
#define TCP_SCHED_FLAG_PENDING_TX     0x10
#define TCP_SCHED_FLAG_PENDING_TSO    0x20
#define TCP_SCHED_FLAG_PENDING_OOO    0x40

#define TCP_SCHED_RING_SESQ           0x0
#define TCP_SCHED_RING_SEND_ACK       0x1
#define TCP_SCHED_RING_DELACK_TIMER   0x2
#define TCP_SCHED_RING_RTO            0x3
#define TCP_SCHED_RING_ASESQ          0x4
#define TCP_SCHED_RING_PENDING_TX     0x5
#define TCP_SCHED_RING_FAST_RETRANS   0x6
#define TCP_SCHED_RING_CLEAN_RETX     0x7

#define TCP_PROXY_TX_TOTAL_RINGS      8

#define TCP_ESTABLISHED   1
#define TCP_SYN_SENT      2
#define TCP_SYN_RECV      3
#define TCP_FIN_WAIT1     4
#define TCP_FIN_WAIT2     5
#define TCP_TIME_WAIT     6
#define TCP_CLOSE         7
#define TCP_CLOSE_WAIT    8
#define TCP_LAST_ACK      9
#define TCP_LISTEN        10
#define TCP_CLOSING       11
#define TCP_NEW_SYN_RECV  12
/* We've received a rst */
#define TCP_RST           13
#define TCP_MAX_STATES    14 /* Leave at the end! */

#define TCP_PARSED_STATE_HANDLE_IN_CPU      0x01


#define TCP_STATE_MASK    0xF

#define TCP_ACTION_FIN    (1 << 7)

#define TCPF_ESTABLISHED  (1 << 1)
#define TCPF_SYN_SENT     (1 << 2)
#define TCPF_SYN_RECV     (1 << 3)
#define TCPF_FIN_WAIT1    (1 << 4)
#define TCPF_FIN_WAIT2    (1 << 5)
#define TCPF_TIME_WAIT    (1 << 6)
#define TCPF_CLOSE        (1 << 7)
#define TCPF_CLOSE_WAIT   (1 << 8)
#define TCPF_LAST_ACK     (1 << 9)
#define TCPF_LISTEN       (1 << 10)
#define TCPF_CLOSING      (1 << 11)
#define TCPF_NEW_SYN_RECV (1 << 12)

/* debug_dol encodings - need to match defines in tcp_proxy.py */
#define TCP_DDOL_PKT_TO_SERQ            0x1
#define TCP_DDOL_TEST_ATOMIC_STATS      0x2
#define TCP_DDOL_DONT_QUEUE_TO_SERQ     0x4
#define TCP_DDOL_LEAVE_IN_ARQ           0x8
#define TCP_DDOL_DONT_SEND_ACK          0x10
#define TCP_DDOL_DEL_ACK_TIMER          0x20
#define TCP_DDOL_PKT_TO_L7Q             0x40
#define TCP_DDOL_BYPASS_BARCO           0x80
#define TCP_DDOL_TSOPT_SUPPORT          0x0100
// Tx encodings
#define TCP_TX_DDOL_DONT_SEND_ACK       0x1
#define TCP_TX_DDOL_DONT_TX             0x2
#define TCP_TX_DDOL_BYPASS_BARCO        0x4
#define TCP_TX_DDOL_DONT_START_RETX_TIMER 0x8
#define TCP_TX_DDOL_FORCE_TIMER_FULL    0x10
#define TCP_TX_DDOL_FORCE_TBL_SETADDR   0x20
#define TCP_TX_DDOL_TSOPT_SUPPORT       0x40

// Tx endcoding bit position
#define TCP_TX_DDOL_DONT_SEND_ACK_BIT       0
#define TCP_TX_DDOL_DONT_TX_BIT             1
#define TCP_TX_DDOL_BYPASS_BARCO_BIT        2
#define TCP_TX_DDOL_DONT_START_RETX_TIMER_BIT   3
#define TCP_TX_DDOL_FORCE_TIMER_FULL_BIT    4
#define TCP_TX_DDOL_FORCE_TBL_SETADDR_BIT   5
#define TCP_TX_DDOL_TSOPT_SUPPORT_BIT       6

#define TCP_DDOL_TBLADDR_SHIFT_OFFSET   (64 * 15)
#define TCP_DDOL_TBLADDR_VALUE          0x59

#define TCP_GC_CB_SW_PI_OFFSET              32

// Global stats
#define TCP_PROXY_STATS_RNMDR_FULL              (0 << 3)
#define TCP_PROXY_STATS_INVALID_SESQ_DESCR      (1 << 3)
#define TCP_PROXY_STATS_INVALID_RETX_SESQ_DESCR (2 << 3)
#define TCP_PROXY_STATS_RETX_PARTIAL_ACK        (3 << 3)
#define TCP_PROXY_STATS_RETX_NOP_SCHEDULE       (4 << 3)
#define TCP_PROXY_STATS_GC_FULL                 (5 << 3)

#endif /* #ifndef __TCP_COMMON_H__ */
