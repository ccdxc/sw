#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_bktrack_sqsge_process_k_t k;

#define SQ_BKTRACK_T struct req_tx_sq_bktrack_info_t
#define SQCB0_WRITE_BACK_T struct req_tx_sqcb_write_back_info_t
#define SQCB1_WRITE_BACK_T struct req_tx_bktrack_sqcb1_write_back_info_t
#define TO_STAGE_T struct req_tx_to_stage_t

%%

    .param    req_tx_bktrack_write_back_process
    .param    req_tx_bktrack_sqcb1_write_back_process

.align
req_tx_bktrack_sqsge_process:
    // sge_t are accessed from bottom to top in big-endian, hence go to
    // the bottom of the SGE_T
    //sge_t[0]
    add            r1, r0, (HBM_NUM_SGES_PER_CACHELINE - 1), LOG_SIZEOF_SGE_T_BITS
    add            r2, k.args.current_sge_offset, r0
    add            r3, k.args.tx_psn, r0
    add            r4, k.to_stage.bktrack.log_pmtu, r0
    // To start with, set empty_rrq_backtrack to false
    setcf          c7, [!c0]

sge_loop:
    CAPRI_TABLE_GET_FIELD(r5, r1, SGE_T, len)
    //if (sge_p->len > current_sge_offset)
    blt            r5, r2, next_sge_start
    nop            // Branch Delay Slot

    // length = sge_p->len - current_sge_offset
    sub            r5, r5, r2 

    // num_pkts = (length + (1 << log_pmtu) - 1) >> log_pmtu
    sllv           r7, 1, r4
    add            r6, r7, r5
    sub            r6, r6, 1
    srlv           r6, r6, r4
    
    // if (rexmit_psn < (tx_psn + num_pkts))
    add            r7, r3, r6
    slt            c1, k.to_stage.bktrack.rexmit_psn, r7 
    bcf            [!c1], next_sge

    // current_sge_offset = ((rexmit_psn  - tx_psn) << log_pmtu) + current_sge_offset
    sub            r7, k.to_stage.bktrack.rexmit_psn, r3
    sllv           r7, r7, r4 
    add            r2, r7, r2

    // tx_psn = rexmit_psn
    add            r3, k.to_stage.bktrack.rexmit_psn, r0

    b              sqcb_writeback
    // set empty_rrq_bktrack to true on completion of bktracking
    setcf          c7, [c0] // Branch Delay Slot

next_sge:
    // current_sge_offset = partial_pkty_bytes ? ((1 << log_pmtu) - partial_pkt_bytes) : 0
    mincr          r5, r4, r0  
    seq            c1, r5, r0
    sllv.!c1       r7, 1, r4
    sub.!c1        r2, r7, r5
    add.c1         r2, r5, r0

    // current_sge_id = current_sge_id + 1
    sub            r1, r1, 1, LOG_SIZEOF_SGE_T_BITS

    bne            r1, r0, sge_loop
    // tx_psn = tx_psn + num_pkts
    add            r3, r6, r3 // Branch Delay Slot
     
next_sge_start:
    // current_sge_id = current_sge_id + 1
    sub            r1, r1, 1, LOG_SIZEOF_SGE_T_BITS

    bne            r1, r0, sge_loop
    // current_sge_offset -= sge_p->len
    sub            r2, r2, r5 // Branch Delay Slot
    

    mfspr          r5, spr_mpuid
    seq            c1, r5[4:2], CAPRI_STAGE_LAST-1
    bcf            [c1], sqcb_writeback

    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7) // Branch Delay Slot

    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, tx_psn, r3)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, ssn, k.args.ssn)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, sq_c_index, k.args.sq_c_index)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, sq_p_index, k.args.sq_p_index)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, in_progress, 1)
    srl            r1, r1, LOG_SIZEOF_SGE_T_BITS
    sub            r1, (HBM_NUM_SGES_PER_CACHELINE - 1), r1
    add            r1, k.args.current_sge_id, r1
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, current_sge_id, r1)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, current_sge_offset, r2)
    add            r2, k.args.num_sges, r1
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, num_sges, r2)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, op_type, k.args.op_type)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, imm_data, k.args.imm_data)

    // sge_addr = wqe_addr + TXWQE_SGE_OFFSET + (sizeof(sge_t) * current_sge_id)
    add          r5,  k.to_stage.bktrack.wqe_addr, r1, LOG_SIZEOF_SGE_T
    add          r5, r5, TXWQE_SGE_OFFSET

    // label and pc cannot be same, so calculate cur pc using bal 
    // and compute start pc deducting the current instruction position
    bal     r6, calculate_raw_table_pc
    nop     //BD Slot

calculate_raw_table_pc:
    // "$" here denores releative current instruction position
    sub     r6, r6, $
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r5)

    nop.e
    nop

sqcb_writeback:
    phvwr          p.common.p4_intr_global_drop, 1

    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)

    CAPRI_SET_FIELD(r7, SQCB0_WRITE_BACK_T, in_progress, 1)
    srl            r1, r1, LOG_SIZEOF_SGE_T_BITS
    sub            r1, (HBM_NUM_SGES_PER_CACHELINE - 1), r1
    add            r1, k.args.current_sge_id, r1

    CAPRI_SET_FIELD(r7, SQCB0_WRITE_BACK_T, current_sge_offset, r2)
    CAPRI_SET_FIELD(r7, SQCB0_WRITE_BACK_T, current_sge_id, r1)
    //sub            r2, k.args.num_sges, r1
    CAPRI_SET_FIELD(r7, SQCB0_WRITE_BACK_T, num_sges, k.args.num_sges)
    CAPRI_SET_FIELD(r7, SQCB0_WRITE_BACK_T, op_type, k.args.op_type)
    CAPRI_SET_FIELD(r7, SQCB0_WRITE_BACK_T, sq_c_index, k.args.sq_c_index)
    CAPRI_SET_FIELD_C(r7, SQCB0_WRITE_BACK_T, empty_rrq_bktrack, 1, c7)

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_bktrack_write_back_process)
    SQCB0_ADDR_GET(r5)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r5)

    CAPRI_GET_TABLE_1_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, skip_wqe_start_psn, 1)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, tx_psn, r3)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, ssn, k.args.ssn)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, tbl_id, 1)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, imm_data, k.args.imm_data)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, inv_key, k.args.inv_key)

    CAPRI_GET_TABLE_1_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_bktrack_sqcb1_write_back_process)
    SQCB1_ADDR_GET(r5)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r5)

    nop.e
    nop
