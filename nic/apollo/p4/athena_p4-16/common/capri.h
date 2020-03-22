#ifndef __P4_COMMON_CAPRI_H__
#define __P4_COMMON_CAPRI_H__

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
#define TM_PORT_BITBKT                 15

/*****************************************************************************/
/* TM instance type                                                          */
/*****************************************************************************/
#define TM_INSTANCE_TYPE_NORMAL         0
#define TM_INSTANCE_TYPE_MULTICAST      1
#define TM_INSTANCE_TYPE_CPU            2
#define TM_INSTANCE_TYPE_SPAN           3
#define TM_INSTANCE_TYPE_CPU_AND_DROP   4
#define TM_INSTANCE_TYPE_SPAN_AND_DROP  5

/*****************************************************************************/
/* Header sizes                                                              */
/*****************************************************************************/
#define CAPRI_GLOBAL_INTRINSIC_HDR_SZ  17
#define CAPRI_P4_INTRINSIC_HDR_SZ      5
#define CAPRI_RXDMA_INTRINSIC_HDR_SZ   15
#define CAPRI_TXDMA_INTRINSIC_HDR_SZ   9

/*****************************************************************************/
/* TM reserved queues                                                        */
/*****************************************************************************/
#define TM_P4_RECIRC_QUEUE             23 /* Recirc queue in the P4 ports */

#endif
