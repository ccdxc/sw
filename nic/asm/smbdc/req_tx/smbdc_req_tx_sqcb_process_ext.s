#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct sqcb0_t d;
struct smbdc_req_tx_s0_t0_k k;

%%

.align
smbdc_req_tx_sqcb_process_ext:

    // copy intrinsic to global
    add            r1, r0, offsetof(struct phv_, common_global_global_data) 

    // enable below code after spr_tbladdr special purpose register is available in capsim
    #mfspr         r1, spr_tbladdr
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR_WITH_SHIFT(SQCB_ADDR_SHIFT))
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)

    //set dma_cmd_ptr in phv
    TXDMA_DMA_CMD_PTR_SET(REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_START_FLIT_CMD_ID) // Branch Delay Slot

exit:
    nop.e
    nop
