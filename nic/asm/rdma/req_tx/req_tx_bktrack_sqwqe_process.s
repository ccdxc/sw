#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct sqwqe_t d;
struct req_tx_bktrack_sqwqe_process_k_t k;

#define SQ_BKTRACK_T struct req_tx_sq_bktrack_info_t
#define SQCB0_WRITE_BACK_T struct req_tx_sqcb_write_back_info_t
#define SQCB1_WRITE_BACK_T struct req_tx_bktrack_sqcb1_write_back_info_t
#define TO_STAGE_T struct req_tx_to_stage_t

#define WQE_OP_TO_NUM_PKTS(_num_pkts_r, _wqe, _log_pmtu, _cf1, _cf2)   \
    seq            _cf1,  _wqe.base.op_type, OP_TYPE_FETCH_N_ADD;        \
    seq            _cf2,  _wqe.base.op_type, OP_TYPE_CMP_N_SWAP;         \
    bcf            [_cf1 | _cf2], _wqe_op_to_num_pkts_end;               \
    add            _num_pkts_r, r0, 1;                                   \
    sllv           _num_pkts_r, 1, _log_pmtu;                            \
    add            _num_pkts_r, _wqe.send.length, _num_pkts_r;           \
    sub            _num_pkts_r, _num_pkts_r, 1;                          \
    srlv           _num_pkts_r, _num_pkts_r, _log_pmtu;                  \
_wqe_op_to_num_pkts_end:

%%

    .param    req_tx_bktrack_sqsge_process
    .param    req_tx_bktrack_write_back_process
    .param    req_tx_bktrack_sqcb1_write_back_process

.align
req_tx_bktrack_sqwqe_process:
    // if (in_progress == TRUE)
    //     tx_psn = k.args.tx_psn
    // else
    //     tx_psn = k.args.tx_psn - wqe_op_to_num_pkts()
    add            r6, k.args.ssn, r0
    seq            c1, k.args.in_progress, 1 
    bcf            [c1], wqe_bktrack
    add            r1, k.args.tx_psn, r0 // Branch Delay Slot
    WQE_OP_TO_NUM_PKTS(r2, d, k.to_stage.bktrack.log_pmtu, c1, c2)
    sub            r1, k.args.tx_psn, r2
    mincr          r1, 24, r0 
    sub            r6, k.args.ssn, 1
    mincr          r6, 24, r0

wqe_bktrack:
    //if (rexmit_psn < tx_psn)
    slt            c2, k.to_stage.bktrack.rexmit_psn, r1
    bcf            [!c2], wqe_match
    // set empty_rrq to false as bktracking is in progress
    setcf          c7, [!c0] // Branch Delay Slot

    // rexmit psn range is lower than current wqe's start psn. Need to go to
    // previous wqe. Compute page_index for (cindex - 1) and see if its
    // in the same page as current wqe or previous page. If previous page,
    // complete this pass and come back to get physical address of previous
    // page using pt entry

    // log_num_wqe_per_page = log_sq_page_size - log_wqe_size
    sub            r2, k.to_stage.bktrack.log_sq_page_size, k.to_stage.bktrack.log_wqe_size

    // prev_page_index = sq_c_index >> log_num_wqe_per_page
    srlv           r3, k.args.sq_c_index, r2
    // c_index = (c_index - 1) % num_wqes
    add            r4, k.args.sq_c_index, r0
    mincr          r4, k.to_stage.bktrack.log_num_wqes, -1
    seq            c2, r4, k.args.sq_p_index
    bcf            [c2], invalid_rexmit_psn
    // page_index = (c_index >> log_num_wqe_per_page)
    srlv           r5, r4, r2  // Branch Delay Slot

    // is previous wqe in the current page?
    //if (page_index == prev_page_index)
    bne            r5, r3, wqe_page_bktrack
    // set r5 - wqe_addr to zero to fetch wqe in a different page
    add            r5, r0, r0 // Branch Delay Slot 
    // wqe_addr = wqe_addr - (1 << log_wqe_size) 
    add            r5, k.to_stage.bktrack.log_wqe_size, r0
    sllv           r5, 1, r5
    // check for c_index wrap around upon backtracking and based on that
    // add or subtract c_index*wqe_size to the previous wqe address
    slt            c1, k.args.sq_c_index, r4
    sub.!c1        r5, k.to_stage.bktrack.wqe_addr, r5
    sll.c1         r5, r4, k.to_stage.bktrack.log_wqe_size
    add.c1         r5, k.to_stage.bktrack.wqe_addr, r5

    // Upate wqe addr to the previos wqe
    CAPRI_GET_STAGE_3_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r5)

    CAPRI_GET_STAGE_4_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r5)

    CAPRI_GET_STAGE_5_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r5)

    CAPRI_GET_STAGE_6_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r5)

    CAPRI_GET_STAGE_7_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, TO_STAGE_T, bktrack.wqe_addr, r5)

    mfspr          r7, spr_mpuid
    seq            c1, r7[6:2], CAPRI_STAGE_LAST-1
    bcf            [c1], sqcb_writeback
    
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, tx_psn, r1)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, ssn, r6)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, sq_c_index, r4)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, sq_p_index, k.args.sq_p_index)
  
    // label and pc cannot be same, so calculate cur pc using bal 
    // and compute start pc deducting the current instruction position
    bal            r6, calculate_raw_table_pc_1
    nop            //BD Slot

calculate_raw_table_pc_1:
    // "$" here denores releative current instruction position
    sub            r6, r6, $
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r5)

    nop.e
    nop 

read_or_sge_bktrack:
    seq            c2, d.base.op_type, OP_TYPE_READ
    bcf            [!c2], sge_bktrack
    nop            // Branch Delay Slot
    
    // num_pkts = rexmit_psn - wqe_start_psn
    sub           r2, k.to_stage.bktrack.rexmit_psn, r1
    // wqe_p->read.va += (num_pkts << log_pmtu)
    // wqe_p->read.len -= (num_pkts << log_pmtu)
    add           r3, k.to_stage.bktrack.log_pmtu, r0
    sllv          r2, r2, r3
    tbladd        d.read.va, r2
    tblsub        d.read.length, r2

    // wqe_start_psn and tx_psn set to rexmit_psn as wqe's va
    // is modified to start from the rexmit_psn. if there's retransmission
    // again, then it should use this rexmit_psn as the wqe_start_psn
    add           r1, k.to_stage.bktrack.rexmit_psn, r0 

    b            sqcb_writeback
    // set empty_rrq to true as bktrack is completed
    setcf          c7, [c0] // Branch Delay Slot

sge_bktrack:
    mfspr        r7, spr_mpuid
    seq          c1, r7[6:2], CAPRI_STAGE_LAST-1
    bcf          [c1], sqcb_writeback
    
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7) // Branch Delay Slot

    // sge_addr = wqe_addr + TXWQE_SGE_OFFSET + (sizeof(sge_t) * current_sge_id)
    add          r3,  k.to_stage.bktrack.wqe_addr, k.args.current_sge_id, LOG_SIZEOF_SGE_T
    add          r3, r3, TXWQE_SGE_OFFSET

    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, tx_psn, r1)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, ssn, r6)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, sq_c_index, k.args.sq_c_index)
    //CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, sq_p_index, k.args.sq_p_index)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, in_progress, k.args.in_progress)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, current_sge_id, 0)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, current_sge_offset, 0)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, num_sges, d.base.num_sges)
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, op_type, d.base.op_type)
    // Always copy imm_data assuming op_type to be send. imm_data is ignored
    // if op_type is not send
    CAPRI_SET_FIELD(r7, SQ_BKTRACK_T, imm_data, d.send.imm_data)
    // Set inv_key only if op_type is send_inv as imm_data & inv_key are unions
    seq            c1, d.base.op_type, OP_TYPE_SEND_INV
    CAPRI_SET_FIELD_C(r7, SQ_BKTRACK_T, inv_key, d.send.inv_key, c1)
     
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_bktrack_sqsge_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r3)

    nop.e
    nop
    
wqe_match:
    // rexmit psn is in the current wqe's psn range. It can either match
    // start psn or somewhere in between, which requires forward sge
    // walk to setup current_sge_id, current_sge_offset in the sqcb
    seq            c2, k.to_stage.bktrack.rexmit_psn, r1
    bcf            [c1 | !c2], read_or_sge_bktrack
    // first wqe itself matches, no change to c_index
    add            r4, r0, k.args.sq_c_index // Branch Delay Slot

    // set empty_rrq to true as bktrack is completed
    setcf          c7, [c0]

    // Fall throguh to retransmit from this matching wqe's start psn

wqe_page_bktrack:
    // need to go to previous page, drop the packet and reschedule
    // fall through to cb writeback

sqcb_writeback:
    phvwr          p.common.p4_intr_global_drop, 1

    CAPRI_GET_TABLE_1_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, wqe_start_psn, r1)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, tx_psn, r1)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, ssn, r6)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, tbl_id, 1)
    // Assume send and copy imm_data, inv_key. These fields are looked into
    // only if op_type is send/write
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, imm_data, d.send.imm_data)
    CAPRI_SET_FIELD(r7, SQCB1_WRITE_BACK_T, inv_key, d.send.inv_key)

    CAPRI_GET_TABLE_1_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_bktrack_sqcb1_write_back_process)
    SQCB1_ADDR_GET(r5)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r5)

    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    CAPRI_SET_FIELD(r7, SQCB0_WRITE_BACK_T, current_sge_offset, k.args.current_sge_offset)
    CAPRI_SET_FIELD(r7, SQCB0_WRITE_BACK_T, current_sge_id, k.args.current_sge_id)
    CAPRI_SET_FIELD(r7, SQCB0_WRITE_BACK_T, num_sges, k.args.num_sges)
    CAPRI_SET_FIELD(r7, SQCB0_WRITE_BACK_T, op_type, d.base.op_type)
    CAPRI_SET_FIELD(r7, SQCB0_WRITE_BACK_T, sq_c_index, r4)
    CAPRI_SET_FIELD_C(r7, SQCB0_WRITE_BACK_T, empty_rrq_bktrack, 1, c7)

    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_bktrack_write_back_process)
    SQCB0_ADDR_GET(r5)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r5)


invalid_rexmit_psn:
    nop.e
    nop
