// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_P4_HPP__
#define __CAPRI_P4_HPP__

#ifdef __cplusplus
namespace sdk {
namespace platform {
namespace capri {
#endif
/*****************************************************************************/
/* TM port values                                                            */
/*****************************************************************************/
#define CAPRI_TM_PORT_UPLINK_0               0
#define CAPRI_TM_PORT_UPLINK_1               1
#define CAPRI_TM_PORT_UPLINK_2               2
#define CAPRI_TM_PORT_UPLINK_3               3
#define CAPRI_TM_PORT_UPLINK_4               4
#define CAPRI_TM_PORT_UPLINK_5               5
#define CAPRI_TM_PORT_UPLINK_6               6
#define CAPRI_TM_PORT_UPLINK_7               7
#define CAPRI_TM_PORT_NCSI                   8
#define CAPRI_TM_PORT_DMA                    9 /* Tx and Rx DMA */
#define CAPRI_TM_PORT_EGRESS                 10
#define CAPRI_TM_PORT_INGRESS                11
#define CAPRI_TM_UPLINK_PORT_BEGIN           CAPRI_TM_PORT_UPLINK_0
#define CAPRI_TM_UPLINK_PORT_END             CAPRI_TM_PORT_NCSI
#define CAPRI_TM_DMA_PORT_BEGIN              CAPRI_TM_PORT_DMA
#define CAPRI_TM_DMA_PORT_END                CAPRI_TM_PORT_DMA
#define CAPRI_TM_P4_PORT_BEGIN               CAPRI_TM_PORT_EGRESS
#define CAPRI_TM_P4_PORT_END                 CAPRI_TM_PORT_INGRESS
#define CAPRI_TM_NUM_UPLINK_PORTS            (CAPRI_TM_UPLINK_PORT_END-CAPRI_TM_UPLINK_PORT_BEGIN+1)
#define CAPRI_TM_NUM_PORTS                   (CAPRI_TM_PORT_INGRESS+1)

/*****************************************************************************/
/* CAPRI_TM instance type                                                   */
/*****************************************************************************/
#define CAPRI_TM_INSTANCE_TYPE_NORMAL         0
#define CAPRI_TM_INSTANCE_TYPE_MULTICAST      1
#define CAPRI_TM_INSTANCE_TYPE_CPU            2
#define CAPRI_TM_INSTANCE_TYPE_SPAN           3
#define CAPRI_TM_INSTANCE_TYPE_CPU_AND_DROP   4
#define CAPRI_TM_INSTANCE_TYPE_SPAN_AND_DROP  5

/*****************************************************************************/
/* Header sizes                                                              */
/*****************************************************************************/
#define CAPRI_GLOBAL_INTRINSIC_HDR_SZ  17
#define CAPRI_P4_INTRINSIC_HDR_SZ      5
#define CAPRI_RXDMA_INTRINSIC_HDR_SZ   15
#define CAPRI_TXDMA_INTRINSIC_HDR_SZ   9

/*****************************************************************************/
/* CAPRI_TM reserved queues                                                  */
/*****************************************************************************/
#define CAPRI_TM_P4_RECIRC_QUEUE             23 /* Recirc queue in the P4 ports */

#ifdef __cplusplus
}    // namespace capri
}    // namespace platform
}    // namespace sdk
#endif

#endif
