#ifndef __TLS_COMMON_H__
#define __TLS_COMMON_H__

#define TLS_SCHED_RING_SERQ           0x0
#define TLS_SCHED_RING_BSQ            0x1
#define TLS_SCHED_RING_BSQ_2PASS      0x2
#define TLS_SCHED_RING_PENDING        0x3
#define TLS_SCHED_RING_TIMER          0x4
#define TLS_SCHED_RING_ARM            0x5

#define TLS_PROXY_GLOBAL_STATS_IDX(x)       (x << 3)
#define TLS_PROXY_GLOBAL_STATS_TNMDPR_EMPTY TLS_PROXY_GLOBAL_STATS_IDX(0)

#endif /* #ifndef __TLS_COMMON_H__ */
