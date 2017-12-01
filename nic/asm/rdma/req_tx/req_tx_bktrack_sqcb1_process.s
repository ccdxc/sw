#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_bktrack_sqcb1_process_k_t k;
struct sqcb1_t d;

#define SQ_BKTRACK_T struct req_tx_sq_bktrack_info_t
#define TO_STAGE_T struct req_tx_to_stage_t
#define SQCB_WRITE_BACK_T struct req_tx_sqcb_write_back_info_t

%%
    .param req_tx_bktrack_sqwqe_process
    .param req_tx_bktrack_sqpt_process
    .param req_tx_bktrack_write_back_process

.align
req_tx_bktrack_sqcb1_process:
    seq            c1, d.rexmit_psn, d.tx_psn
    bcf            [c1], write_back
    
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

    seq            c1, k.to_stage.bktrack.wqe_addr, r0
    bcf            [c1],  bktrack_sqpt
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, tx_psn, d.tx_psn) // Branch Delay Slot

wqe_bktrack:
    seq           c1, k.args.in_progress, 1
    CAPRI_SET_FIELD_C(r7, SQ_BKTRACK_T, tx_psn, d.wqe_start_psn, c1)
    
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_bktrack_sqwqe_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, k.to_stage.bktrack.wqe_addr)
    
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
    add            r3, k.args.pt_base_addr, r3, CAPRI_LOG_SIZEOF_U64

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_bktrack_sqpt_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r3)

    nop.e
    nop

write_back:
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, in_progress, k.args.in_progress)
    CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, empty_rrq_bktrack, 1)
    CAPRI_SET_FIELD(r7, SQCB_WRITE_BACK_T, release_cb1_busy, 1)
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
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_bktrack_write_back_process)
    SQCB0_ADDR_GET(r1)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)

    // drop the phv as there is nothing to retransmit
    phvwr          p.common.p4_intr_global_drop, 1

    nop.e
    nop
