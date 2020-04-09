// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __ELBA_P4_HPP__
#define __ELBA_P4_HPP__

#ifdef __cplusplus
namespace sdk {
namespace platform {
namespace elba {
#endif

/*****************************************************************************/
/* TM port values                                                            */
/*****************************************************************************/
#define ELBA_TM_PORT_UPLINK_0               0
#define ELBA_TM_PORT_UPLINK_1               1
#define ELBA_TM_PORT_UPLINK_2               2
#define ELBA_TM_PORT_UPLINK_3               3
#define ELBA_TM_PORT_NCSI                   4
#define ELBA_TM_PORT_DMA                    5 /* Tx and Rx DMA */
#define ELBA_TM_PORT_EGRESS                 6
#define ELBA_TM_PORT_INGRESS                7
#define ELBA_TM_UPLINK_PORT_BEGIN           ELBA_TM_PORT_UPLINK_0
#define ELBA_TM_UPLINK_PORT_END             ELBA_TM_PORT_NCSI
#define ELBA_TM_DMA_PORT_BEGIN              ELBA_TM_PORT_DMA
#define ELBA_TM_DMA_PORT_END                ELBA_TM_PORT_DMA
#define ELBA_TM_P4_PORT_BEGIN               ELBA_TM_PORT_EGRESS
#define ELBA_TM_P4_PORT_END                 ELBA_TM_PORT_INGRESS
#define ELBA_TM_NUM_UPLINK_PORTS            (ELBA_TM_UPLINK_PORT_END-ELBA_TM_UPLINK_PORT_BEGIN+1)
#define ELBA_TM_NUM_PORTS                   (ELBA_TM_PORT_INGRESS+1)

/*****************************************************************************/
/* Header sizes                                                              */
/*****************************************************************************/
#define ELBA_GLOBAL_INTRINSIC_HDR_SZ  17
#define ELBA_P4_INTRINSIC_HDR_SZ      5
#define ELBA_RXDMA_INTRINSIC_HDR_SZ   15
#define ELBA_TXDMA_INTRINSIC_HDR_SZ   9

/*****************************************************************************/
/* TM reserved queues                                                        */
/*****************************************************************************/
#define ELBA_TM_P4_RECIRC_QUEUE             23 /* Recirc queue in the P4 ports */

#ifdef __cplusplus
}    // namespace elba
}    // namespace platform
}    // namespace sdk
#endif

#endif
