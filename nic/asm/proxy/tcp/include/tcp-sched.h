#ifndef __TCP_SCHED_H__
#define __TCP_SCHED_H__

#define TCP_SCHED_FLAG_FT             0x1
#define TCP_SCHED_FLAG_ST             0x2
#define TCP_SCHED_FLAG_FT_ERR         0x4
#define TCP_SCHED_FLAG_ST_ERR         0x8
#define TCP_SCHED_FLAG_PENDING_TX     0x10
#define TCP_SCHED_FLAG_PENDING_TSO    0x20
#define TCP_SCHED_FLAG_PENDING_OOO    0x40

#define TCP_SCHED_RING_SESQ           0x0
#define TCP_SCHED_RING_PENDING        0x1
#define TCP_SCHED_RING_FT             0x2
#define TCP_SCHED_RING_ST             0x3

#endif /* #ifndef __TCP_SCHED_H__ */
