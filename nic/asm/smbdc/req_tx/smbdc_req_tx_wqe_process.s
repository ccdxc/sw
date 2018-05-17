#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct smbdc_sqwqe_t d;
struct smbdc_req_tx_s1_t0_k k;

#define WQE_TO_SELECT_MR_P t0_s2s_wqe_to_mr_select_info
#define WQE_TO_SGE_P t0_s2s_wqe_to_sge_info

#define TO_S2_P       to_s2_to_stage
#define TO_S4_P       to_s4_to_stage_sq

#define IN_P t0_s2s_sqcb_to_wqe_info

#define K_MAX_FRAGMENTED_SIZE CAPRI_KEY_RANGE(IN_P, max_fragmented_size_sbit0_ebit7, max_fragmented_size_sbit24_ebit31)
#define K_MAX_SEND_SIZE CAPRI_KEY_FIELD(IN_P, max_send_size)
#define K_CURRENT_SGE_ID CAPRI_KEY_FIELD(IN_P, current_sge_id)
#define K_CURRENT_SGE_OFFSET CAPRI_KEY_FIELD(IN_P, current_sge_offset)
#define K_IN_PROGRESS CAPRI_KEY_FIELD(IN_P, in_progress)
#define K_SEND_CREDITS_AVAILABLE CAPRI_KEY_FIELD(IN_P, send_credits_available)

%%
    .param    smbdc_req_tx_mr_select_process
    .param    smbdc_req_tx_sge_process
    .param    smbdc_req_tx_sqcb_writeback_sq_process

.align
smbdc_req_tx_wqe_process:

    seq     c1, d.type, SMBDC_SQWQE_OP_TYPE_SEND
    bcf     [c1], process_send

process_mr:
    CAPRI_RESET_TABLE_0_ARG() //BD Slot

    add     r1, d.mr.num_sg_lists, r0
    
    phvwr CAPRI_PHV_FIELD(WQE_TO_SELECT_MR_P, num_mrs), d.mr.num_sg_lists

    //sqcb1 has the MR bitmap associated with this connection
    SQCB1_ADDR_GET(r2)    

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_mr_select_process, r2)
    
    add     r2, r0, r0
    setcf   c1, [c0] //first sg_list
    add     r1, r0, offsetof(struct req_tx_phv_t, rdma_wqe3)
    add     r3, r0, offsetof(struct smbdc_sqwqe_mr_t, sg3.num_pages)

#go thru each of the SG list and prepare one RDMA FRMR wqe on PHV
# - this stage will setup wrid, frmr_type, dma_src_address, num_pages
# - next stage(mr_select) will setup mr_id associated with each wqe
# - one after that(post_rdma) will setup DMA instructions to post WQEs to RDMA queue
for_each_sg_list:

    phvwrp  r1, offsetof(struct sqwqe_t, base.wrid), sizeof(p.rdma_wqe0.base.wrid), d.mr.wrid
    phvwrp  r1, offsetof(struct sqwqe_t, base.op_type), sizeof(p.rdma_wqe0.base.op_type), OP_TYPE_FRMR
    phvwrp  r1, offsetof(struct sqwqe_t, frmr.type), sizeof(p.rdma_wqe0.frmr.type), FRMR_TYPE_REGULAR
    tblrdp  r4, r3, 0, sizeof(d.mr.sg0.num_pages)
    phvwrp  r1, offsetof(struct sqwqe_t, frmr.num_pages), sizeof(p.rdma_wqe0.frmr.num_pages), r4
    tblrdp  r4, r3, sizeof(d.mr.sg0.num_pages), sizeof(d.mr.sg0.base_addr)
    phvwrp  r1, offsetof(struct sqwqe_t, frmr.dma_src_address), sizeof(p.rdma_wqe0.frmr.dma_src_address), r4
    
    add     r1, r1, sizeof(p.rdma_wqe0)
    add     r2, r2, 1
    seq     c1, r2, d.mr.num_sg_lists
    bcf     [!c1], for_each_sg_list
    add     r3, r3, sizeof(d.mr.sg0) //jump to next sg element - BD Slot
    
    //fill context
    phvwr   p.smbdc_wqe_context.wrid, d.mr.wrid
    
    nop.e
    nop

process_send:
    
    bbeq    K_SEND_CREDITS_AVAILABLE, 0, skip_send_processing
    
    sle     c1, K_MAX_FRAGMENTED_SIZE, d.send.total_len  //BD Slot
    bcf     [c1], invalid_request
    nop
  
    bbeq    K_IN_PROGRESS, 1, in_progress
    nop

    //fill smbdc_wqe context
    phvwr   p.smbdc_wqe_context.wrid, d.send.wrid
    
    mfspr   r2, spr_tbladdr //Branch Delay Slot
    add     r2, r2, offsetof(struct smbdc_sqwqe_send_t, sg0)

    CAPRI_RESET_TABLE_0_ARG()

    slt     c1, d.send.num_sges, SMBDC_NUM_SGES_PER_CACHELINE
    cmov    r1, c1, d.send.num_sges, SMBDC_NUM_SGES_PER_CACHELINE

    phvwr     CAPRI_PHV_FIELD(TO_S2_P, total_data_length), d.send.total_len

    phvwrpair CAPRI_PHV_FIELD(WQE_TO_SGE_P, current_sge_id), r0, \
              CAPRI_PHV_FIELD(WQE_TO_SGE_P, num_valid_sges), r1
    phvwr     CAPRI_PHV_FIELD(WQE_TO_SGE_P, wrid), d.send.wrid
    phvwr     CAPRI_PHV_FIELD(WQE_TO_SGE_P, current_sge_offset), r0
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_sge_process, r2)

    nop.e
    nop
    
in_progress:

    mfspr   r2, spr_tbladdr //Branch Delay Slot
    add     r2, r2, offsetof(struct smbdc_sqwqe_send_t, sg0)
    sll     r5, sizeof(struct smbdc_sge_t), K_CURRENT_SGE_ID
    add     r2, r2, r5

    sub     r1, d.send.num_sges, K_CURRENT_SGE_ID

    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair CAPRI_PHV_FIELD(WQE_TO_SGE_P, current_sge_id), K_CURRENT_SGE_ID, \
              CAPRI_PHV_FIELD(WQE_TO_SGE_P, num_valid_sges), r1
    phvwr     CAPRI_PHV_FIELD(WQE_TO_SGE_P, wrid), d.send.wrid
    phvwr     CAPRI_PHV_FIELD(WQE_TO_SGE_P, current_sge_offset), K_CURRENT_SGE_OFFSET

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_sge_process, r2)

    nop.e
    nop

invalid_request:
exit:
    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop

skip_send_processing:

    phvwr     CAPRI_PHV_FIELD(TO_S4_P, clear_busy_and_exit), 1
    CAPRI_RESET_TABLE_0_ARG()
    SQCB0_ADDR_GET(r2)
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, smbdc_req_tx_sqcb_writeback_sq_process, r2)
