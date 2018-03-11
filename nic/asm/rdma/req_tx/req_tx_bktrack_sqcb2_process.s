#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_bktrack_sqcb2_process_k_t k;
struct sqcb2_t d;

#define SQ_BKTRACK_T struct req_tx_sq_bktrack_info_t
#define TO_STAGE_T struct req_tx_to_stage_t
#define SQCB_WRITE_BACK_T struct req_tx_sqcb_write_back_info_t

%%
    .param req_tx_bktrack_sqwqe_process
    .param req_tx_bktrack_sqpt_process
    .param req_tx_bktrack_write_back_process

.align
req_tx_bktrack_sqcb2_process:
    // if rexmit_psn is same as tx_psn, then this is a spurious timer expiry
    // event. Just write back to release busy bits
    seq            c1, d.rexmit_psn, d.tx_psn
    bcf            [c1], sqcb_write_back
    
    CAPRI_GET_STAGE_2_ARG(req_tx_phv_t, r7) // Branch Delay Slot
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.rexmit_psn, d.rexmit_psn)

    CAPRI_GET_STAGE_3_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.rexmit_psn, d.rexmit_psn)
 
    CAPRI_GET_STAGE_4_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.rexmit_psn, d.rexmit_psn)

    CAPRI_GET_STAGE_5_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.rexmit_psn, d.rexmit_psn)

    CAPRI_GET_STAGE_6_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.rexmit_psn, d.rexmit_psn)

    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)

    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, sq_c_index, k.args.sq_c_index)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, sq_p_index, k.args.sq_p_index)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, in_progress, k.args.in_progress)
    //CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, bktrack_in_progress, k.args.bktrack_in_progress)
    CAPRI_SET_FIELD_RANGE(r7, SQ_BKTRACK_T, current_sge_offset, num_sges, k.{args.current_sge_offset...args.num_sges})
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, ssn, d.ssn)

    seq            c2, k.args.sq_in_hbm, 1 
    seq            c1, k.to_stage.bktrack.wqe_addr, r0
    bcf.!c2        [c1],  bktrack_sqpt
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, tx_psn, d.tx_psn) // Branch Delay Slot

    bcf            [c2 & !c1], wqe_bktrack
    add            r2, r0, k.to_stage.bktrack.wqe_addr // Branch Delay Slot
    
    sll            r2, k.args.sq_c_index, k.to_stage.bktrack.log_wqe_size
    //pt_base_addr is overloaded with sq_hbm_base_addr
    add            r2, r2, k.args.pt_base_addr, HBM_SQ_BASE_ADDR_SHIFT
    
wqe_bktrack:
    seq           c1, k.args.in_progress, 1
    CAPRI_SET_FIELD_C(r7, SQ_BKTRACK_T, tx_psn, d.wqe_start_psn, c1)
    
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqwqe_process, r2)
 
    //for now, use to_stage_args to pass the wqe_addr
    //until we organize better, copy to all stages
    CAPRI_GET_STAGE_2_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r2)

    CAPRI_GET_STAGE_3_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r2)
    
    CAPRI_GET_STAGE_4_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r2)
    
    CAPRI_GET_STAGE_5_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r2)
    
    CAPRI_GET_STAGE_6_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r2)
   
    CAPRI_GET_STAGE_7_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r2)
   
    nop.e
    nop

bktrack_sqpt:
    // log_num_wqe_per_page = (ssqcb0_to_sqcb1_info_p->log_sq_page_size - sqcb0_to_sqcb1_info_p->log_wqe_size) 
    sub            r1, k.to_stage.bktrack.log_sq_page_size, k.to_stage.bktrack.log_wqe_size

    // page_index = (sqcb0_to_sqcb1_info_p->sq_c_index >> log_num_wqe_per_page)
    srlv           r1, k.args.sq_c_index, r1

    // pt_seg_addr = (pt_base_addr + ((page_index & ~0x7) * sizeof(u64)))
    and            r2, r1, CAPRI_SEG_PAGE_MASK
    sub            r3, r1, r2
    sll            r5, k.args.pt_base_addr, PT_BASE_ADDR_SHIFT
    add            r3, r5, r3, CAPRI_LOG_SIZEOF_U64

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqpt_process, r3)

    nop.e
    nop

sqcb_write_back:
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, in_progress, k.args.in_progress)
    CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, empty_rrq_bktrack, 1)
    //CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, release_cb1_busy, 1)
    CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, num_sges, k.args.num_sges)
    CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, current_sge_id, k.args.current_sge_id)
    CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, current_sge_offset, k.args.current_sge_offset)
    CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, op_type, k.args.op_type)
    bbeq           k.args.in_progress, 1, set_pc
    CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, sq_c_index, k.args.sq_c_index) // Branch Delay Slot
    
    add            r1, k.args.sq_c_index, 1
    mincr          r1, 24, r0
    CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, sq_c_index, r1) 

set_pc:
    SQCB0_ADDR_GET(r1)
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_write_back_process, r1)

    // drop the phv as there is nothing to retransmit
    phvwr          p.common.p4_intr_global_drop, 1

    nop.e
    nop
