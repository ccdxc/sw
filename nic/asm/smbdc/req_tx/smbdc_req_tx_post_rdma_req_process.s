#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct sqcb2_t d;
struct smbdc_req_tx_s3_t0_k k;

#define IN_P t0_s2s_mr_select_to_rdma_info
#define IN_TO_S3_P to_s3_to_stage

#define TO_S4_P       to_s4_to_stage_sq

#define K_NUM_WQES CAPRI_KEY_FIELD(IN_P, num_wqes)
#define K_FILL_SMBD_HDR_SGE CAPRI_KEY_FIELD(IN_P, fill_smbd_hdr_sge)

%%
    .param    smbdc_req_tx_sqcb_writeback_sq_process

.align

smbdc_req_tx_post_rdma_req_process:

    crestore [c5], K_FILL_SMBD_HDR_SGE, 0x1

first_wqe:
   
    bcf [!c5], first_wqe_skip_send_actions

    sll    r4, d.rdma_sq_pindex, (1 << RDMA_SQ_WQE_CONTEXT_LOG_SIZE) //BD Slot
    add    r4, r4, d.rdma_sq_wqe_context_base_addr

    //point to smbd hdr in rdma wqe context as first SGE in RDMA request
    phvwrpair     p.rdma_wqe0.sge0.va, r4, \
                  p.rdma_wqe0.sge0.len, sizeof(struct rdma_smbdc_header_t)
    //use separate lkeys if needed
    phvwrpair     p.rdma_wqe0.sge0.lkey, d.rdma_local_dma_lkey, \
                  p.rdma_wqe0.sge1.lkey, d.rdma_local_dma_lkey

    //Pass credits_granted only for the first rdma_wqe
    //Assumption: Make sure d.new_credits_offered is updated/set in the same stage in a locked manner
    phvwrpair     p.rdma_wqe_ctx0.smbdc_hdr.credits_requested, d.send_credit_target, \
                  p.rdma_wqe_ctx0.smbdc_hdr.credits_granted, d.new_credits_offered
    //remaining_data_len field is set by sge_process
    //data_len field is set by sge_process
    phvwrpair     p.rdma_wqe_ctx0.smbdc_hdr.data_offset, sizeof(struct rdma_smbdc_header_t), \
                  p.rdma_wqe_ctx0.smbdc_hdr.padding, r0
    .assert(sizeof(struct rdma_smbdc_header_t) % 8 == 0)
    tblwr         d.new_credits_offered, 0

    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_REQ_BASE + 3)
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe_ctx0, rdma_wqe_ctx0, r4)
    
first_wqe_skip_send_actions:

    sll r2, d.rdma_sq_pindex, d.rdma_sq_log_wqe_size
    add r2, r2, d.rdma_sq_base_addr

    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_REQ_BASE)
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe0, rdma_wqe0, r2)
 
    add r1, r0, 1
    seq c1, r1, K_NUM_WQES
    bcf [c1], post_doorbell
    tblmincri d.rdma_sq_pindex, d.rdma_sq_log_num_wqes, 1

second_wqe:

    bcf [!c5], second_wqe_skip_send_actions

    sll    r4, d.rdma_sq_pindex, (1 << RDMA_SQ_WQE_CONTEXT_LOG_SIZE) //BD Slot
    add    r4, r4, d.rdma_sq_wqe_context_base_addr

    //point to smbd hdr in rdma wqe context as first SGE in RDMA request
    phvwrpair     p.rdma_wqe1.sge0.va, r4, \
                  p.rdma_wqe1.sge0.len, sizeof(struct rdma_smbdc_header_t)
    phvwrpair     p.rdma_wqe1.sge0.lkey, d.rdma_local_dma_lkey, \
                  p.rdma_wqe1.sge1.lkey, d.rdma_local_dma_lkey

    //Pass credits_granted as r0 for non-first rdma_wqe
    //Assumption: Make sure d.new_credits_offered is updated/set in the same stage in a locked manner
    phvwrpair     p.rdma_wqe_ctx1.smbdc_hdr.credits_requested, d.send_credit_target, \
                  p.rdma_wqe_ctx1.smbdc_hdr.credits_granted, r0
    //remaining_data_len field is set by sge_process
    //data_len field is set by sge_process
    phvwrpair     p.rdma_wqe_ctx1.smbdc_hdr.data_offset, sizeof(struct rdma_smbdc_header_t), \
                  p.rdma_wqe_ctx1.smbdc_hdr.padding, r0
    .assert(sizeof(struct rdma_smbdc_header_t) % 8 == 0)

    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_REQ_BASE + 4)
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe_ctx1, rdma_wqe_ctx1, r4)
    
second_wqe_skip_send_actions:

    sll r2, d.rdma_sq_pindex, d.rdma_sq_log_wqe_size
    add r2, r2, d.rdma_sq_base_addr

    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, (REQ_TX_DMA_CMD_RDMA_REQ_BASE + 1))
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe1, rdma_wqe1, r2)
    
    add r1, r1, 1
    seq c1, r1, K_NUM_WQES
    bcf [c1], post_doorbell
    tblmincri d.rdma_sq_pindex, d.rdma_sq_log_num_wqes, 1

third_wqe:

    bcf [!c5], third_wqe_skip_send_actions

    sll    r4, d.rdma_sq_pindex, (1 << RDMA_SQ_WQE_CONTEXT_LOG_SIZE) //BD Slot
    add    r4, r4, d.rdma_sq_wqe_context_base_addr

    //point to smbd hdr in rdma wqe context as first SGE in RDMA request
    phvwrpair     p.rdma_wqe2.sge0.va, r4, \
                  p.rdma_wqe2.sge0.len, sizeof(struct rdma_smbdc_header_t)
    phvwrpair     p.rdma_wqe2.sge0.lkey, d.rdma_local_dma_lkey, \
                  p.rdma_wqe2.sge1.lkey, d.rdma_local_dma_lkey

    //Pass credits_granted as r0 for non-first rdma_wqe
    phvwrpair     p.rdma_wqe_ctx2.smbdc_hdr.credits_requested, d.send_credit_target, \
                  p.rdma_wqe_ctx2.smbdc_hdr.credits_granted, r0
    //remaining_data_len field is set by sge_process
    //data_len field is set by sge_process
    phvwrpair     p.rdma_wqe_ctx2.smbdc_hdr.data_offset, sizeof(struct rdma_smbdc_header_t), \
                  p.rdma_wqe_ctx2.smbdc_hdr.padding, r0
    .assert(sizeof(struct rdma_smbdc_header_t) % 8 == 0)

    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_REQ_BASE + 5)
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe_ctx2, rdma_wqe_ctx2, r4)
    
third_wqe_skip_send_actions:

    sll r2, d.rdma_sq_pindex, d.rdma_sq_log_wqe_size
    add r2, r2, d.rdma_sq_base_addr

    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, (REQ_TX_DMA_CMD_RDMA_REQ_BASE + 2))
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe2, rdma_wqe2, r2)
    
    add r1, r1, 1
    seq c1, r1, K_NUM_WQES
    bcf [c1], post_doorbell
    tblmincri d.rdma_sq_pindex, d.rdma_sq_log_num_wqes, 1

fourth_wqe:
    
#if 0
    #right now, fourth_wqe should not be true for SEND wqe type
    //assert(c5 == FALSE)

    bcf [!c5], fourth_wqe_skip_send_actions

    sll    r4, d.rdma_sq_pindex, (1 << RDMA_SQ_WQE_CONTEXT_LOG_SIZE) //BD Slot
    add    r4, r4, d.rdma_sq_wqe_context_base_addr

    //point to smbd hdr in rdma wqe context as first SGE in RDMA request
    phvwrpair.c5  p.rdma_wqe3.sge0.va, r4, \
                  p.rdma_wqe3.sge0.len, sizeof(struct rdma_smbdc_header_t)
    phvwrpair.c5  p.rdma_wqe3.sge0.lkey, d.rdma_local_dma_lkey, \
                  p.rdma_wqe3.sge1.lkey, d.rdma_local_dma_lkey

    //Pass credits_granted as r0 for non-first rdma_wqe
    phvwrpair.c5  p.rdma_wqe_ctx3.smbdc_hdr.credits_requested, d.send_credit_target, \
                  p.rdma_wqe_ctx3.smbdc_hdr.credits_granted, r0
    //remaining_data_len field is set by sge_process
    //data_len field is set by sge_process
    phvwrpair.c5  p.rdma_wqe_ctx3.smbdc_hdr.data_offset, sizeof(struct rdma_smbdc_header_t), \
                  p.rdma_wqe_ctx3.smbdc_hdr.padding, r0
    .assert(sizeof(struct rdma_smbdc_header_t) % 8 == 0)

    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_REQ_BASE + 6)
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe_ctx2, rdma_wqe_ctx2, r4)

fourth_wqe_skip_send_actions:
#endif

    sll r2, d.rdma_sq_pindex, d.rdma_sq_log_wqe_size
    add r2, r2, d.rdma_sq_base_addr

    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, (REQ_TX_DMA_CMD_RDMA_REQ_BASE + 3))
    DMA_HBM_PHV2MEM_SETUP(r6, rdma_wqe3, rdma_wqe3, r2)
    
    tblmincri d.rdma_sq_pindex, d.rdma_sq_log_num_wqes, 1

    #add r1, r1, 1
    #seq c1, r1, K_NUM_WQES
    #bcf [c1], post_doorbell

post_doorbell:
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_ID_RDMA_DOORBELL)
    CAPRI_SETUP_DB_ADDR(DB_ADDR_BASE, DB_SET_PINDEX, DB_SCHED_WR_EVAL_RING, d.rdma_lif, d.rdma_qtype, r5)
    CAPRI_SETUP_DB_DATA(d.rdma_qid, RDMA_SQ_RING_ID, d.rdma_sq_pindex, r6)
    phvwr   p.db_data, r6.dx
    DMA_HBM_PHV2MEM_SETUP(r6, db_data, db_data, r5)

    //fill the MSN in the wqe context - to be matched when RDMA CQEs are received
    phvwrpair p.smbdc_wqe_context.start_msn, d.rdma_sq_msn, \
              p.smbdc_wqe_context.end_msn, d.rdma_sq_msn
    //do not increment d.rdma_sq_msn for MR requests. Only do it for SEND requests

    CAPRI_RESET_TABLE_0_ARG()
    phvwr     CAPRI_PHV_FIELD(TO_S4_P, incr_sq_cindex), 1
    SQCB0_ADDR_GET(r6)
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_sqcb_writeback_sq_process, r6)

done:
    nop.e
    nop
    
exit:
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop
