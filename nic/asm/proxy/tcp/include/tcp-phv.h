#ifndef __TCP_PHV_H__
#define __TCP_PHV_H__

#include "tcp-shared-state.h"

#define TCP_PHV_TXDMA_COMMANDS_START    (CAPRI_PHV_START_OFFSET(intrinsic_dma_dma_cmd_type) / 16)
#define TCP_PHV_RXDMA_COMMANDS_START    (CAPRI_PHV_START_OFFSET(pkt_dma_dma_cmd_type) / 16)
#define TCP_PHV_OOQ_TXDMA_COMMANDS_START (CAPRI_PHV_START_OFFSET(intrinsic_dma_cmd_type) / 16)

#endif /* #ifndef __TCP_PHV_H__ */
