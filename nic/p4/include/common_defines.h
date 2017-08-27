#ifndef __COMMON_DEFINES_H__
#define __COMMON_DEFINES_H__

//Queue Types used on a regular LIF
//where RDMA and Ethernet can be enabled at the same time
#define Q_TYPE_RDMA_SQ 0
#define Q_TYPE_RDMA_RQ 1
#define Q_TYPE_TXQ     2
#define Q_TYPE_ADMINQ  3
#define Q_TYPE_RXQ     4
#define Q_TYPE_RDMA_CQ 5
#define Q_TYPE_RDMA_EQ 6

#endif //__COMMON_DEFINES_H__

