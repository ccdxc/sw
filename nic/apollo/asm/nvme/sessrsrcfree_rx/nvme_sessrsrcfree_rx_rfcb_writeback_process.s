#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t0_nvme_sessrsrcfree_rx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s6_t0_nvme_sessrsrcfree_rx_k_ k;
struct s6_t0_nvme_sessrsrcfree_rx_rfcb_writeback_process_d d;

#define DMA_CMD_BASE r7

%%

.align
nvme_sessrsrcfree_rx_rfcb_writeback_process:

    tblmincri.f    SESSRSRCFREERX_C_INDEX, d.log_num_entries, 1
    
    // DMA to release busy lock and fence with page refcnt decrement.
    add            r1, r0, d.wb_r0_busy
    mincr          r1, 1, 1
    DMA_CMD_BASE_GET(DMA_CMD_BASE, wb_r0_busy_dma)
    DMA_HBM_PHV2MEM_SETUP(DMA_CMD_BASE, wb_r0_busy_data, wb_r0_busy_data, r1)
    DMA_SET_WR_FENCE_E(DMA_CMD_PHV2MEM_T, DMA_CMD_BASE)    

    CAPRI_SET_TABLE_0_VALID(0)  //Exit Slot
