#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s1_t0_k k;
struct sqcb2_t d;

#define SQ_BKTRACK_P t0_s2s_sq_bktrack_info
#define SQCB_WRITE_BACK_P t0_s2s_sqcb_write_back_info

#define TO_S2_P to_s2_bktrack_to_stage
#define TO_S3_P to_s3_bktrack_to_stage
#define TO_S4_P to_s4_bktrack_to_stage
#define TO_S5_P to_s5_bktrack_to_stage
#define TO_S6_P to_s6_bktrack_to_stage
#define TO_S7_P to_s6_bktrack_to_stage

#define IN_P t0_s2s_sqcb0_to_sqcb2_info
#define IN_TO_S_P to_s1_bktrack_to_stage

#define K_NUM_SGES     CAPRI_KEY_RANGE(IN_P, num_sges_sbit0_ebit5, num_sges_sbit6_ebit7) 
#define K_SQ_C_INDEX   CAPRI_KEY_RANGE(IN_P, sq_c_index_sbit0_ebit7, sq_c_index_sbit8_ebit15)
#define K_SQ_P_INDEX   CAPRI_KEY_RANGE(IN_P, sq_p_index_sbit0_ebit7, sq_p_index_sbit8_ebit15)
#define K_PT_BASE_ADDR CAPRI_KEY_RANGE(IN_P, pt_base_addr_sbit0_ebit3, pt_base_addr_sbit28_ebit31)
#define K_OP_TYPE      CAPRI_KEY_FIELD(IN_P, op_type) 
#define K_CURRENT_SGE_ID   CAPRI_KEY_RANGE(IN_P, current_sge_id_sbit0_ebit5, current_sge_id_sbit6_ebit7) 
#define K_CURRENT_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit5, current_sge_offset_sbit30_ebit31) 
#define K_LOG_SQ_PAGE_SIZE CAPRI_KEY_RANGE(IN_TO_S_P, log_sq_page_size_sbit0_ebit2, log_sq_page_size_sbit3_ebit4)
#define K_LOG_NUM_WQES CAPRI_KEY_RANGE(IN_TO_S_P, log_num_wqes_sbit0_ebit0, log_num_wqes_sbit1_ebit4)
%%
    .param req_tx_bktrack_sqwqe_process
    .param req_tx_bktrack_sqpt_process
    .param req_tx_bktrack_write_back_process
    .param req_tx_bktrack_sqcb1_write_back_process

.align
req_tx_timer_process:

    // TODO- restart the timer if needed; currently doing it unconditionally
    CAPRI_START_SLOW_TIMER(r1, r2, K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, TIMER_RING_ID, 10)
 
retrans_timer_expiry:
    // Check if at this expiry event retransmit time has reached
    slt            c1, r4, d.last_ack_or_req_ts
    sub.c1         r1, d.last_ack_or_req_ts, r4
    sub.!c1        r1, r4, d.last_ack_or_req_ts

check_rnr_timeout:
    seq            c1, d.rnr_timeout, 0
    bcf            [c1], check_local_ack_timeout

rnr_timeout:
    sll            r2, 1, d.rnr_timeout // Branch Delay Slot
    // Ignore expiry event if retransmit time has not reached
    // TODO comment out this check for now as in model cur_timestamp
    // is not populated in r4
    //blt            r1, r2, sqcb_write_back
 
    // Infinite retries if retry_ctr is set to 7
    seq            c1, d.rnr_retry_ctr, 7 // Branch Delay Slot
    tblsub.!c1     d.rnr_retry_ctr, 1

    // Check rnr_retry_ctr for RNR
    seq            c1, d.rnr_retry_ctr, 0 // Branch Delay Slot
    bcf            [c1], err_completion
    nop            // Branch Delay Slot

    b              sq_bktrack
 
check_local_ack_timeout:
    // if rexmit_psn is more than exp_rsp_psn, then this is a spurious timer
    // expiry event. Just write back to release busy bits
    // TODO - check if timer can be stopped using rexmit_psn and tx_psn
    seq            c1, d.local_ack_timeout, 0
    scwle24        c2, d.rexmit_psn, d.exp_rsp_psn
    bcf            [c1 | !c2], sqcb_write_back

local_ack_timeout:
    // 4.096 * (2 ^ local_ack_timeout) usec  = 1 << (12 + local_ack_timeout) nsec
    // Minimum local_ack_timeout that can be supported in capri is 2 (> 10usec)
    // local_ack_timeout in sqcb2 is programmed to include multiplication factor
    // of 4096 as well
    sll            r2, 1, d.local_ack_timeout
    // Ignore expiry event if retransmit time has not reached
    // TODO comment out this check for now as in model cur_timestamp
    // is not populated in r4
    //blt            r1, r2, sqcb_write_back

    // Infinite retries if retry_ctr is set to 7
    seq            c1, d.err_retry_ctr, 7 // Branch Delay Slot
    tblsub.!c1     d.err_retry_ctr, 1

    // Check err_retry_ctr for retransmit timeout
    seq            c1, d.err_retry_ctr, 0 // Branch Delay Slot
    bcf            [c1], err_completion


sq_bktrack:
    phvwrpair CAPRI_PHV_FIELD(TO_S2_P, rexmit_psn), d.rexmit_psn, \
              CAPRI_PHV_FIELD(TO_S3_P, rexmit_psn), d.rexmit_psn

    phvwrpair CAPRI_PHV_FIELD(TO_S4_P, rexmit_psn), d.rexmit_psn, \
              CAPRI_PHV_FIELD(TO_S5_P, rexmit_psn), d.rexmit_psn

    phvwr CAPRI_PHV_FIELD(TO_S6_P, rexmit_psn), d.rexmit_psn

    CAPRI_RESET_TABLE_1_ARG()
    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, tx_psn), d.tx_psn, \
              CAPRI_PHV_FIELD(SQ_BKTRACK_P, ssn), d.ssn

    SQCB1_ADDR_GET(r1)
    CAPRI_NEXT_TABLE1_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqcb1_write_back_process, r1)

    CAPRI_RESET_TABLE_0_ARG()

    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, sq_c_index), K_SQ_C_INDEX, \
              CAPRI_PHV_FIELD(SQ_BKTRACK_P, sq_p_index_or_imm_data1_or_inv_key1), K_SQ_P_INDEX
    //current_sge_offset, current_sge_id, num_sges
    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, in_progress), CAPRI_KEY_FIELD(IN_P, in_progress), \
              CAPRI_PHV_RANGE(SQ_BKTRACK_P, current_sge_offset, num_sges), \
              CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit5, num_sges_sbit6_ebit7)
    //phvwr CAPRI_PHV_FIELD(SQ_BKTRACK_P, bktrack_in_progress), CAPRI_KEY_FIELD(IN_P, bktrack_in_progress)
    phvwr CAPRI_PHV_FIELD(SQ_BKTRACK_P, ssn), d.ssn

    seq            c2, CAPRI_KEY_FIELD(IN_P, sq_in_hbm), 1 
    seq            c1, CAPRI_KEY_FIELD(IN_TO_S_P, wqe_addr), r0
    bcf.!c2        [c1],  bktrack_sqpt
    phvwr CAPRI_PHV_FIELD(SQ_BKTRACK_P, tx_psn), d.tx_psn // Branch Delay Slot

    bcf            [c2 & !c1], wqe_bktrack
    add            r2, r0, CAPRI_KEY_FIELD(IN_TO_S_P, wqe_addr) // Branch Delay Slot
    
    sll            r2, K_SQ_C_INDEX, CAPRI_KEY_FIELD(IN_TO_S_P, log_wqe_size)
    //pt_base_addr is overloaded with sq_hbm_base_addr
    add            r2, r2, K_PT_BASE_ADDR, HBM_SQ_BASE_ADDR_SHIFT
    
wqe_bktrack:
    seq           c1, CAPRI_KEY_FIELD(IN_P, in_progress), 1
    phvwr.c1 CAPRI_PHV_FIELD(SQ_BKTRACK_P, tx_psn), d.wqe_start_psn

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqwqe_process, r2)
 
    //for now, use to_stage_args to pass the wqe_addr
    //until we organize better, copy to all stages
    phvwrpair CAPRI_PHV_FIELD(TO_S2_P, wqe_addr), r2, \
              CAPRI_PHV_FIELD(TO_S3_P, wqe_addr), r2

    phvwrpair CAPRI_PHV_FIELD(TO_S4_P, wqe_addr), r2, \
              CAPRI_PHV_FIELD(TO_S5_P, wqe_addr), r2

    phvwr.e CAPRI_PHV_FIELD(TO_S6_P, wqe_addr), r2
    phvwr CAPRI_PHV_FIELD(TO_S7_P, wqe_addr), r2

bktrack_sqpt:
    // log_num_wqe_per_page = (ssqcb0_to_sqcb1_info_p->log_sq_page_size - sqcb0_to_sqcb1_info_p->log_wqe_size) 
    sub            r1, K_LOG_SQ_PAGE_SIZE, CAPRI_KEY_FIELD(IN_TO_S_P, log_wqe_size)

    // page_index = (sqcb0_to_sqcb1_info_p->sq_c_index >> log_num_wqe_per_page)
    srlv           r1, K_SQ_C_INDEX, r1

    // pt_seg_addr = (pt_base_addr + ((page_index & ~0x7) * sizeof(u64)))
    and            r2, r1, CAPRI_SEG_PAGE_MASK
    sub            r3, r1, r2
    sll            r5, K_PT_BASE_ADDR, PT_BASE_ADDR_SHIFT
    add            r3, r5, r3, CAPRI_LOG_SIZEOF_U64

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqpt_process, r3)

    nop.e
    nop

sqcb_write_back:
    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, in_progress), CAPRI_KEY_FIELD(IN_P, in_progress), \
              CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, empty_rrq_bktrack), 1
    phvwrpair CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, current_sge_id), K_CURRENT_SGE_ID, \
              CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, num_sges), K_NUM_SGES
    phvwrpair CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, op_type), K_OP_TYPE, \
              CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, current_sge_offset), K_CURRENT_SGE_OFFSET
    bbeq           CAPRI_KEY_FIELD(IN_P, in_progress), 1, set_pc
    phvwr CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, sq_c_index), K_SQ_C_INDEX // Branch Delay Slot
    
    add            r1, K_SQ_C_INDEX, 1
    mincr          r1, K_LOG_NUM_WQES, r0
    phvwr CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, sq_c_index), r1

set_pc:
    SQCB0_ADDR_GET(r1)
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_write_back_process, r1)

    // drop the phv as there is nothing to retransmit
    phvwr          p.common.p4_intr_global_drop, 1

    // clear cb1_busy retaining in_progress and need_credits as before
    add            r1, r0, d.need_credits, SQCB0_NEED_CREDITS_BIT_OFFSET
    or             r1, r1, d.in_progress, SQCB0_IN_PROGRESS_BIT_OFFSET
    SQCB0_ADDR_GET(r2)
    add            r2, r2, FIELD_OFFSET(sqcb0_t, cb1_byte)
    memwr.b        r2, r1

    nop.e
    nop

err_completion:
    phvwr   p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_VALID(0)
    nop.e
    nop
    
