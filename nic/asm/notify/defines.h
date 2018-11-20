
#include "capri-macros.h"
#include "../asm/eth/defines.h"
#include "../p4/common/defines.h"


#define LG2_NOTIFY_EVENT_DESC_SIZE        6   // 64 B

#define NOTIFY_DMA_CMD_START_OFFSET       (CAPRI_PHV_START_OFFSET(dma_dma_cmd_type) / 16)
#define NOTIFY_DMA_CMD_START_FLIT         ((offsetof(p, dma_dma_cmd_type) / 512) + 1)
#define NOTIFY_DMA_CMD_START_INDEX        0
