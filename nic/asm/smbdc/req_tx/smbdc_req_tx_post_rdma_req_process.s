#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct sqcb2_t d;
struct smbdc_req_tx_s3_t0_k k;

#define IN_P t0_s2s_mr_select_to_rdma_info
#define IN_TO_S3_P to_s3_to_stage

#define K_NUM_MRS CAPRI_KEY_FIELD(IN_P, num_mrs)

%%

.align

smbdc_req_tx_post_rdma_req_process:

    add r2, r0, d.rdma_sq_base_addr
    add r2, d.rdma_sq_pindex, d.rdma_sq_log_wqe_size
    add r3, 1, d.rdma_sq_log_wqe_size

first_wqe:

    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_FRMR_BASE)
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe0, rdma_wqe0, r2)
    
    add r1, r0, 1
    seq c1, r1, K_NUM_MRS
    bcf [c1], post_doorbell
    tblmincri d.rdma_sq_pindex, d.rdma_sq_log_num_wqes, 1

second_wqe:

    add r2, r2, r3
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, (REQ_TX_DMA_CMD_FRMR_BASE + 1))
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe1, rdma_wqe1, r2)
    
    add r1, r1, 1
    seq c1, r1, K_NUM_MRS
    bcf [c1], post_doorbell
    tblmincri d.rdma_sq_pindex, d.rdma_sq_log_num_wqes, 1

third_wqe:

    add r2, r2, r3
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, (REQ_TX_DMA_CMD_FRMR_BASE + 2))
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe2, rdma_wqe2, r2)
    
    add r1, r1, 1
    seq c1, r1, K_NUM_MRS
    bcf [c1], post_doorbell
    tblmincri d.rdma_sq_pindex, d.rdma_sq_log_num_wqes, 1

fourth_wqe:

    add r2, r2, r3
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, (REQ_TX_DMA_CMD_FRMR_BASE + 3))
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe3, rdma_wqe3, r2)
    
    tblmincri d.rdma_sq_pindex, d.rdma_sq_log_num_wqes, 1

    #add r1, r1, 1
    #seq c1, r1, K_NUM_MRS
    #bcf [c1], post_doorbell

post_doorbell:
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, (REQ_TX_DMA_CMD_FRMR_BASE + 4))
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, d.rdma_lif, d.rdma_qtype, r5)
    CAPRI_SETUP_DB_DATA(d.rdma_qid, RDMA_SQ_RING_ID, d.rdma_sq_pindex, r6)
    phvwr   p.db_data, r6.dx
    DMA_HBM_PHV2MEM_SETUP(r6, db_data, db_data, r5)

    //fill the MSN in the wqe context - to be matched when RDMA CQEs are received
    phvwrpair p.smbdc_wqe_context.start_msn, d.rdma_sq_msn, \
              p.smbdc_wqe_context.end_msn, d.rdma_sq_msn
    //do not increment d.rdma_sq_msn for MR requests. Only do it for SEND requests

    #DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)
    CAPRI_SET_TABLE_0_VALID(0)

done:
    nop.e
    nop
    
exit:
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop
