#include "capri.h"
#include "req_tx.h"
#include "cqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct cqcb_t d;
struct smbdc_req_tx_s3_t0_k k;

#define IN_P smbdc_req_tx_cqcb_info

%%

    .param    smbdc_req_tx_sqcb_writeback_cq_process

.align
smbdc_req_tx_cqcb_process:

    seq             c1, CQ_P_INDEX, 0
    // flip the color if cq is wrap around
    tblmincri.c1    CQ_COLOR, 1, 1

    // set the color in cqe
    phvwr           p.smbdc_cqe.color, CQ_COLOR

    // status, wrid, mr_id[s] should already be updated to phv->smbdc_cqe
    
    add             r1, CQ_P_INDEX, 0

    sll             r2, r1, d.log_wqe_size
    add             r2, r2, d.cq_base_addr

    tblmincri       CQ_P_INDEX, d.log_num_wqes, 1 

       
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_ID_CQ)
    DMA_HBM_PHV2MEM_SETUP(r6, smbdc_cqe, smbdc_cqe, r2)

    //DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)

    CAPRI_RESET_TABLE_0_ARG()

    SQCB0_ADDR_GET(r2)

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_sqcb_writeback_cq_process, r2)

    nop.e
    nop
