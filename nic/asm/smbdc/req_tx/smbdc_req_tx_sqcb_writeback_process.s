#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct sqcb0_t d;
struct smbdc_req_tx_s4_t0_k k;

#define IN_P t0_s2s_writeback_info
#define IN_TO_S4_P to_s4_to_stage

%%

.align

smbdc_req_tx_sqcb_writeback_process:

    // Pin to stage 4
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], bubble_to_next_stage

    #restore flags incr_proxy_cq_cindex, incr_rdma_cq_cindex, incr_sq_unack_pindex, clear_busy, clear_in_progress
    crestore       [c5, c4, c3, c2, c1], CAPRI_KEY_RANGE(IN_TO_S4_P, incr_proxy_cq_cindex, clear_in_progress), 0xF
    #c5 - incr_proxy_cq_cindex
    #c4 - incr_rdma_cq_cindex
    #c3 - incr_sq_unack_pindex
    #c2 - clear_busy
    #c1 - clear_in_progress

    tblwr.c2       d.rdma_cq_processing_busy, 0
    tblwr.c1       d.rdma_cq_processing_in_prog, 0
    tblmincri.c3   d.sq_unack_pindex, d.log_num_wqes, 1
    bcf            [c4], skip_cq_cindex_update
    tblmincri.c5   RDMA_CQ_PROXY_C_INDEX, d.rdma_cq_log_num_wqes, 1 //BD Slot

    //prepare doorbell to set RDMA CQ's cindex. r5 for db_addr and r6 for db_data
    PREPARE_DOORBELL_WRITE_CINDEX(d.rdma_cq_lif, d.rdma_cq_qtype, d.rdma_cq_qid, d.rdma_cq_ring_id, RDMA_CQ_PROXY_C_INDEX, r5, r6)
    phvwr          p.db_data, r6
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_ID_RDMA_CQ)
    DMA_HBM_PHV2MEM_SETUP(r6, db_data, db_data, r5)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2MEM_T, r6)

    nop.e
    nop

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    bcf           [!c1 | c2], exit
    nop           // Branch Delay Slot

    SQCB0_ADDR_GET(r2)
    //invoke the same routine, but with valid d[] vector
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r2)
    nop.e
    nop

skip_cq_cindex_update:

exit:
    nop.e
    nop
