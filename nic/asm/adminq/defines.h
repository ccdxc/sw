
#include "capri-macros.h"
#include "../../asm/eth/defines.h"
#include "../../p4/common/defines.h"

/*
 * ADMINQ
 */

#define LG2_ADMINQ_QSTATE_SIZE			6

#define LG2_ADMINQ_CMD_DESC_SIZE		6
#define LG2_ADMINQ_COMP_DESC_SIZE		4

#define ADMINQ_DMA_CMD_START_OFFSET		(CAPRI_PHV_START_OFFSET(dma_dma_cmd_type) / 16)
#define ADMINQ_DMA_CMD_START_FLIT		((offsetof(p, dma_dma_cmd_type) / 512) + 1)
#define ADMINQ_DMA_CMD_START_INDEX		0

/*
 * NICMGR
 */

#define LG2_NICMGR_QSTATE_SIZE			6

#define LG2_NICMGR_REQ_DESC_SIZE		7
#define LG2_NICMGR_RESP_DESC_SIZE 		7
#define LG2_NICMGR_RESP_DESC_HDR_SIZE	6
#define LG2_NICMGR_RESP_DESC_DATA_SIZE	6

#define NICMGR_DMA_CMD_START_OFFSET		(CAPRI_PHV_START_OFFSET(dma_dma_cmd_type) / 16)
#define NICMGR_DMA_CMD_START_FLIT		((offsetof(p, dma_dma_cmd_type) / 512) + 1)
#define NICMGR_DMA_CMD_START_INDEX		0
