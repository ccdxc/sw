#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s1_t0_k k;
struct sqcb2_t d;

#define SQ_BKTRACK_P t0_s2s_sq_bktrack_info
#define SQCB_WRITE_BACK_P t0_s2s_sqcb_write_back_info

#define TO_S2_BT_P to_s2_bt_info
#define TO_S3_BT_P to_s3_bt_info
#define TO_S4_BT_P to_s4_bt_info
#define TO_S5_BT_P to_s5_bt_info
#define TO_S6_BT_P to_s6_bt_info

#define TO_S7_BT_WB_P to_s7_bt_wb_info

#define IN_P t0_s2s_sqcb0_to_sqcb2_info
#define IN_TO_S_P to_s1_bt_info

#define K_NUM_SGES     CAPRI_KEY_RANGE(IN_P, num_sges_sbit0_ebit5, num_sges_sbit6_ebit7) 
#define K_SQ_C_INDEX   CAPRI_KEY_RANGE(IN_P, sq_c_index_sbit0_ebit7, sq_c_index_sbit8_ebit15)
#define K_SQ_P_INDEX   CAPRI_KEY_RANGE(IN_P, sq_p_index_sbit0_ebit7, sq_p_index_sbit8_ebit15)
#define K_PT_BASE_ADDR CAPRI_KEY_RANGE(IN_P, pt_base_addr_sbit0_ebit3, pt_base_addr_sbit28_ebit31)
#define K_OP_TYPE      CAPRI_KEY_FIELD(IN_P, op_type)
#define K_CURRENT_SGE_ID   CAPRI_KEY_RANGE(IN_P, current_sge_id_sbit0_ebit5, current_sge_id_sbit6_ebit7) 
#define K_CURRENT_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit5, current_sge_offset_sbit30_ebit31) 
#define K_LOG_SQ_PAGE_SIZE CAPRI_KEY_RANGE(IN_TO_S_P, log_sq_page_size_sbit0_ebit2, log_sq_page_size_sbit3_ebit4)
#define K_LOG_NUM_WQES CAPRI_KEY_RANGE(IN_TO_S_P, log_num_wqes_sbit0_ebit0, log_num_wqes_sbit1_ebit4)
#define K_WQE_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, wqe_addr_sbit0_ebit31, wqe_addr_sbit48_ebit63)
%%
    .param req_tx_bktrack_sqwqe_process
    .param req_tx_bktrack_sqsge_process
    .param req_tx_bktrack_sqpt_process
    .param req_tx_bktrack_write_back_process

.align
req_tx_bktrack_sqcb2_process:
    bbeq           d.busy, 1, exit

    // Infinite retries if retry_ctr is set to 7
    //seq            c1, d.err_retry_ctr, 7 // Branch Delay Slot
    //tblsub.!c1     d.err_retry_ctr, 1

    // Check err_retry_ctr for NAK (seq error), implicit NAK 
    //seq            c1, d.err_retry_ctr, 0
    //bcf            [c1], err_completion

    tblwr          d.busy, 1
    // Clear rnr_timeout. 
    // SQ ring will drop all speculations and not make progress when RNR timer is running. Clearing it here after
    // RNR timer expires and triggers bktrack.
    tblwr          d.rnr_timeout, 0 

    phvwrpair CAPRI_PHV_FIELD(TO_S2_BT_P, rexmit_psn), d.rexmit_psn, \
              CAPRI_PHV_FIELD(TO_S3_BT_P, rexmit_psn), d.rexmit_psn
    phvwrpair CAPRI_PHV_FIELD(TO_S4_BT_P, rexmit_psn), d.rexmit_psn, \
              CAPRI_PHV_FIELD(TO_S5_BT_P, rexmit_psn), d.rexmit_psn
    phvwr CAPRI_PHV_FIELD(TO_S6_BT_P, rexmit_psn), d.rexmit_psn

    CAPRI_RESET_TABLE_0_ARG()

    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, sq_c_index), K_SQ_C_INDEX, \
              CAPRI_PHV_FIELD(SQ_BKTRACK_P, sq_p_index_or_imm_data1_or_inv_key1), K_SQ_P_INDEX
    //current_sge_offset, current_sge_id, num_sges
    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, in_progress), CAPRI_KEY_FIELD(IN_P, in_progress), \
              CAPRI_PHV_RANGE(SQ_BKTRACK_P, current_sge_offset, num_sges), \
              CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit5, num_sges_sbit6_ebit7)
    //phvwr CAPRI_PHV_FIELD(SQ_BKTRACK_P, bktrack_in_progress), CAPRI_KEY_FIELD(IN_P, bktrack_in_progress)
    phvwr CAPRI_PHV_FIELD(SQ_BKTRACK_P, ssn), d.ssn
    phvwr CAPRI_PHV_FIELD(SQ_BKTRACK_P, op_type), d.curr_op_type

    seq            c2, CAPRI_KEY_FIELD(IN_P, sq_in_hbm), 1 
    seq            c1, K_WQE_ADDR, r0
    bcf.!c2        [c1],  bktrack_sqpt
    phvwr CAPRI_PHV_FIELD(SQ_BKTRACK_P, tx_psn), d.tx_psn // Branch Delay Slot

    bcf            [!c1], wqe_bktrack
    add            r2, r0, K_WQE_ADDR // Branch Delay Slot
    
    sll            r2, K_SQ_C_INDEX, CAPRI_KEY_FIELD(IN_TO_S_P, log_wqe_size)
    //pt_base_addr is overloaded with sq_hbm_base_addr
    add            r2, r2, K_PT_BASE_ADDR, HBM_SQ_BASE_ADDR_SHIFT

wqe_bktrack:
    seq           c1, CAPRI_KEY_FIELD(IN_P, in_progress), 1
    phvwr.c1 CAPRI_PHV_FIELD(SQ_BKTRACK_P, tx_psn), d.wqe_start_psn

    //for now, use to_stage_args to pass the wqe_addr
    //until we organize better, copy to all stages
    phvwrpair CAPRI_PHV_FIELD(TO_S2_BT_P, wqe_addr), r2, \
              CAPRI_PHV_FIELD(TO_S3_BT_P, wqe_addr), r2

    phvwrpair CAPRI_PHV_FIELD(TO_S4_BT_P, wqe_addr), r2, \
              CAPRI_PHV_FIELD(TO_S5_BT_P, wqe_addr), r2

    phvwrpair CAPRI_PHV_FIELD(TO_S6_BT_P, wqe_addr), r2, \
              CAPRI_PHV_FIELD(TO_S7_BT_WB_P, wqe_addr), r2

    phvwr    CAPRI_PHV_FIELD(TO_S7_BT_WB_P, wqe_start_psn), d.wqe_start_psn

    scwle24       c2, d.tx_psn, d.rexmit_psn
    // sge_addr = wqe_addr + TXWQE_SGE_OFFSET + (sizeof(sge_t) * current_sge_id)
    add.c2        r2,  K_WQE_ADDR, K_CURRENT_SGE_ID, LOG_SIZEOF_SGE_T
    add.c2        r2, r2, TXWQE_SGE_OFFSET
    phvwr.c2      CAPRI_PHV_FIELD(SQ_BKTRACK_P, tx_psn), d.tx_psn
    CAPRI_NEXT_TABLE0_READ_PC_CE(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqsge_process, req_tx_bktrack_sqwqe_process, r2, c2)

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

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqpt_process, r3)

err_completion:
    phvwr.e   p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_VALID(0)

exit:
    SQCB0_ADDR_GET(r1)
    CAPRI_RESET_TABLE_0_ARG()
    phvwr          CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, drop_phv), 1
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_write_back_process, r1)
