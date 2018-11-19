
#include "capri-macros.h"
#include "../asm/eth/defines.h"
#include "../p4/common/defines.h"


#define LG2_EDMA_CMD_DESC_SIZE          5   // 32 B
#define LG2_EDMA_COMP_DESC_SIZE         4   // 16 B

#define EDMA_DMA_CMD_START_OFFSET       (CAPRI_PHV_START_OFFSET(dma_dma_cmd_type) / 16)
#define EDMA_DMA_CMD_START_FLIT         ((offsetof(p, dma_dma_cmd_type) / 512) + 1)
#define EDMA_DMA_CMD_START_INDEX        0

#define EDMA_OPCODE_HOST_TO_LOCAL       0
#define EDMA_OPCODE_LOCAL_TO_HOST       1
#define EDMA_OPCODE_LOCAL_TO_LOCAL      2
#define EDMA_OPCODE_HOST_TO_HOST        3
