
#include "capri-macros.h"

#define LG2_RX_DESC_SIZE        (4)
#define LG2_TX_DESC_SIZE        (4)
#define LG2_RX_CMPL_DESC_SIZE   (5)
#define LG2_TX_CMPL_DESC_SIZE   (4)

#define ETH_DMA_CMD_PTR         (CAPRI_PHV_START_OFFSET(dma_cmd0_dma_cmd_type) / 16)
