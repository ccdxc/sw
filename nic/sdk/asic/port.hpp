// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __ASIC_PORT_HPP__
#define __ASIC_PORT_HPP__

#ifndef ELBA
#include "platform/capri/capri_p4.hpp"
#else
#include "platform/elba/elba_p4.hpp"
#endif

#ifdef __cplusplus
namespace sdk {
namespace asic {
#endif

#ifndef ELBA

/*****************************************************************************/
/* TM port values                                                            */
/*****************************************************************************/
#define TM_PORT_UPLINK_0               CAPRI_TM_PORT_UPLINK_0
#define TM_PORT_UPLINK_1               CAPRI_TM_PORT_UPLINK_1
#define TM_PORT_UPLINK_2               CAPRI_TM_PORT_UPLINK_2
#define TM_PORT_UPLINK_3               CAPRI_TM_PORT_UPLINK_3
#define TM_PORT_UPLINK_4               CAPRI_TM_PORT_UPLINK_4
#define TM_PORT_UPLINK_5               CAPRI_TM_PORT_UPLINK_5
#define TM_PORT_UPLINK_6               CAPRI_TM_PORT_UPLINK_6
#define TM_PORT_UPLINK_7               CAPRI_TM_PORT_UPLINK_7
#define TM_PORT_NCSI                   CAPRI_TM_PORT_NCSI
#define TM_PORT_DMA                    CAPRI_TM_PORT_DMA /* Tx and Rx DMA */
#define TM_PORT_EGRESS                 CAPRI_TM_PORT_EGRESS
#define TM_PORT_INGRESS                CAPRI_TM_PORT_INGRESS
#define TM_UPLINK_PORT_BEGIN           TM_PORT_UPLINK_0
#define TM_UPLINK_PORT_END             TM_PORT_NCSI
#define TM_DMA_PORT_BEGIN              TM_PORT_DMA
#define TM_DMA_PORT_END                TM_PORT_DMA
#define TM_P4_PORT_BEGIN               TM_PORT_EGRESS
#define TM_P4_PORT_END                 TM_PORT_INGRESS
#define TM_NUM_UPLINK_PORTS            (TM_UPLINK_PORT_END-TM_UPLINK_PORT_BEGIN+1)
#define TM_NUM_PORTS                   (TM_PORT_INGRESS+1)

/*****************************************************************************/
/* TM instance type                                                          */
/*****************************************************************************/
#define TM_INSTANCE_TYPE_NORMAL         CAPRI_TM_INSTANCE_TYPE_NORMAL
#define TM_INSTANCE_TYPE_MULTICAST      CAPRI_TM_INSTANCE_TYPE_MULTICAST
#define TM_INSTANCE_TYPE_CPU            CAPRI_TM_INSTANCE_TYPE_CPU
#define TM_INSTANCE_TYPE_SPAN           CAPRI_TM_INSTANCE_TYPE_SPAN
#define TM_INSTANCE_TYPE_CPU_AND_DROP   CAPRI_TM_INSTANCE_TYPE_CPU_AND_DROP
#define TM_INSTANCE_TYPE_SPAN_AND_DROP  CAPRI_TM_INSTANCE_TYPE_SPAN_AND_DROP

/*****************************************************************************/
/* Header sizes                                                              */
/*****************************************************************************/
#define ASICPD_GLOBAL_INTRINSIC_HDR_SZ  CAPRI_GLOBAL_INTRINSIC_HDR_SZ
#define ASICPD_P4_INTRINSIC_HDR_SZ      CAPRI_P4_INTRINSIC_HDR_SZ
#define ASICPD_RXDMA_INTRINSIC_HDR_SZ   CAPRI_RXDMA_INTRINSIC_HDR_SZ
#define ASICPD_TXDMA_INTRINSIC_HDR_SZ   CAPRI_TXDMA_INTRINSIC_HDR_SZ

/*****************************************************************************/
/* TM reserved queues                                                        */
/*****************************************************************************/
#define TM_P4_RECIRC_QUEUE              CAPRI_TM_P4_RECIRC_QUEUE /* Recirc queue in the P4 ports */

#else

/*****************************************************************************/
/* TM port values                                                            */
/*****************************************************************************/
#define TM_PORT_UPLINK_0               ELBA_TM_PORT_UPLINK_0
#define TM_PORT_UPLINK_1               ELBA_TM_PORT_UPLINK_1
#define TM_PORT_UPLINK_2               ELBA_TM_PORT_UPLINK_2
#define TM_PORT_UPLINK_3               ELBA_TM_PORT_UPLINK_3
#define TM_PORT_NCSI                   ELBA_TM_PORT_NCSI
#define TM_PORT_DMA                    ELBA_TM_PORT_DMA /* Tx and Rx DMA */
#define TM_PORT_EGRESS                 ELBA_TM_PORT_EGRESS
#define TM_PORT_INGRESS                ELBA_TM_PORT_INGRESS
#define TM_UPLINK_PORT_BEGIN           ELBA_TM_UPLINK_PORT_BEGIN
#define TM_UPLINK_PORT_END             ELBA_TM_UPLINK_PORT_END
#define TM_DMA_PORT_BEGIN              ELBA_TM_DMA_PORT_BEGIN
#define TM_DMA_PORT_END                ELBA_TM_DMA_PORT_END
#define TM_P4_PORT_BEGIN               ELBA_TM_P4_PORT_BEGIN
#define TM_P4_PORT_END                 ELBA_TM_P4_PORT_END
#define TM_NUM_UPLINK_PORTS            ELBA_TM_NUM_UPLINK_PORTS
#define TM_NUM_PORTS                   ELBA_TM_NUM_PORTS

/*****************************************************************************/
/* Header sizes                                                              */
/*****************************************************************************/
#define ASICPD_GLOBAL_INTRINSIC_HDR_SZ  ELBA_GLOBAL_INTRINSIC_HDR_SZ
#define ASICPD_P4_INTRINSIC_HDR_SZ      ELBA_P4_INTRINSIC_HDR_SZ
#define ASICPD_RXDMA_INTRINSIC_HDR_SZ   ELBA_RXDMA_INTRINSIC_HDR_SZ
#define ASICPD_TXDMA_INTRINSIC_HDR_SZ   ELBA_TXDMA_INTRINSIC_HDR_SZ

/*****************************************************************************/
/* TM reserved queues                                                        */
/*****************************************************************************/
#define TM_P4_RECIRC_QUEUE              ELBA_TM_P4_RECIRC_QUEUE   /* Recirc queue in the P4 ports */

#endif

#ifdef __cplusplus
}    //    namespace asic
}    //    namespace sdk
#endif


#endif  // __ASIC_PORT_HPP__
