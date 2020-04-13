#include "../../p4/include/artemis_defines.h"
#include "capri-macros.h"
#include "asic/cmn/asic_common.hpp"

#define ASM_INSTRUCTION_OFFSET_MAX     (64 * 256)

// TxDMA PHV offsets
#define TXDMA_IFLOW_PARENT_FLIT         2
#define TXDMA_IFLOW_LEAF_FLIT           3
#define TXDMA_RFLOW_PARENT_FLIT         4
#define TXDMA_RFLOW_LEAF_FLIT           5
