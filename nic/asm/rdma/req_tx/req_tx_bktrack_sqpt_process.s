#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s2_t0_k k;

#define SQ_BKTRACK_P t0_s2s_sq_bktrack_info

#define TO_S3_P to_s3_bktrack_to_stage
#define TO_S4_P to_s4_bktrack_to_stage
#define TO_S5_P to_s5_bktrack_to_stage
#define TO_S6_P to_s6_bktrack_to_stage
#define TO_S7_P to_s7_bktrack_to_stage

#define IN_P t0_s2s_sq_bktrack_info
#define IN_TO_S_P to_s2_bktrack_to_stage

#define K_SQ_C_INDEX CAPRI_KEY_RANGE(IN_P, sq_c_index_sbit0_ebit7, sq_c_index_sbit8_ebit15)
#define K_SQ_P_INDEX CAPRI_KEY_RANGE(IN_P, sq_p_index_or_imm_data1_or_inv_key1_sbit0_ebit2, sq_p_index_or_imm_data1_or_inv_key1_sbit11_ebit15)
#define K_LOG_SQ_PAGE_SIZE CAPRI_KEY_RANGE(IN_TO_S_P, log_sq_page_size_sbit0_ebit2, log_sq_page_size_sbit3_ebit4)
#define K_LOG_WQE_SIZE CAPRI_KEY_FIELD(IN_TO_S_P, log_wqe_size)
#define K_TX_PSN CAPRI_KEY_RANGE(IN_P, tx_psn_sbit0_ebit6, tx_psn_sbit23_ebit23)
#define K_SSN CAPRI_KEY_RANGE(IN_P, ssn_sbit0_ebit6, ssn_sbit23_ebit23)

%%
    .param    req_tx_bktrack_sqwqe_process

.align
req_tx_bktrack_sqpt_process:

     // log_num_wqe_per_page = (log_sq_page_size - log_wqe_size)
    // page_index = (info_p->sq_c_index >> log_num_wqe_per_page)
    add            r1, r0, K_SQ_C_INDEX
    sub            r2, K_LOG_SQ_PAGE_SIZE, K_LOG_WQE_SIZE
    srlv           r3, r1, r2

    // page_offset = (info_p->sq_c_index & ((1 << log_num_wqe_per_page) - 1)) << info_p->log_wqe_size
    mincr          r1, r2, r0
    sll            r1, r1, K_LOG_WQE_SIZE
    
    // page_seg_offset = page_index & 0x7
    and            r2, r3, CAPRI_SEG_PAGE_MASK

    // big-endian
    sub            r2, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), r2

    // page_addr_p = (u64 *)(d_p + sizeof(u64) * sqcb_to_pt_info_p->page_seg_offset)
    sll            r2, r2, CAPRI_LOG_SIZEOF_U64_BITS
    // big-endian
    tblrdp.dx      r2, r2, 0, CAPRI_SIZEOF_U64_BITS

    //wqe_p = (void *)(*page_addr_p + sqcb_to_pt_info_p->page_offset)
    add            r1, r2, r1

    // populate t0 stage to stage data req_tx_sqpt_to_bktrack_wqe_info_t for next stage
    CAPRI_RESET_TABLE_0_ARG()

    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, tx_psn), K_TX_PSN, CAPRI_PHV_FIELD(SQ_BKTRACK_P, ssn), K_SSN
    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, sq_c_index), K_SQ_C_INDEX, CAPRI_PHV_FIELD(SQ_BKTRACK_P, sq_p_index_or_imm_data1_or_inv_key1), K_SQ_P_INDEX
    phvwrpair CAPRI_PHV_FIELD(SQ_BKTRACK_P, in_progress), CAPRI_KEY_FIELD(IN_P, in_progress), CAPRI_PHV_RANGE(SQ_BKTRACK_P, current_sge_offset, num_sges), CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit6, num_sges_sbit7_ebit7)
    //phvwr CAPRI_PHV_FIELD(SQ_BKTRACK_P, bktrack_in_progress), CAPRI_KEY_FIELD(IN_P, bktrack_in_progress)

    // populate t0 PC and table address
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_sqwqe_process, r1)

    //for now, use to_stage_args to pass the wqe_addr
    //until we organize better, copy to all stages
    phvwr CAPRI_PHV_FIELD(TO_S3_P, wqe_addr), r1

    phvwrpair.e CAPRI_PHV_FIELD(TO_S4_P, wqe_addr), r1, CAPRI_PHV_FIELD(TO_S5_P, wqe_addr), r1

    phvwrpair CAPRI_PHV_FIELD(TO_S6_P, wqe_addr), r1, CAPRI_PHV_FIELD(TO_S7_P, wqe_addr), r1
