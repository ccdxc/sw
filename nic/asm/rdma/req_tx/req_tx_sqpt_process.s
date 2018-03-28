#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s1_t0_k k;

#define SQCB_TO_WQE_P t0_s2s_sqcb_to_wqe_info
#define TO_STAGE_SQ_INFO_P to_s5_sq_info

#define IN_P t0_s2s_sqcb_to_pt_info

#define K_PAGE_OFFSET CAPRI_KEY_RANGE(IN_P, page_offset_sbit0_ebit7, page_offset_sbit8_ebit15)
#define K_REMAINING_PAYLOAD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit4, remaining_payload_bytes_sbit29_ebit31)

#define TO_S5_P to_s5_sq_to_stage
%%
    .param    req_tx_sqwqe_process

.align
req_tx_sqpt_process:

    // big-endian
    sub           r1, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), CAPRI_KEY_FIELD(IN_P, page_seg_offset)
 
    // page_addr_p = (u64 *)(d_p + sizeof(u64) * sqcb_to_pt_info_p->page_seg_offset)
    sll            r1, r1, CAPRI_LOG_SIZEOF_U64_BITS
    // big-endian
    tblrdp.dx      r1, r1, 0, CAPRI_SIZEOF_U64_BITS

    //wqe_p = (void *)(*page_addr_p + sqcb_to_pt_info_p->page_offset)
    add            r1, r1, K_PAGE_OFFSET

    // populate t0 stage to stage data req_tx_sqcb_to_wqe_info_t for next stage
    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair CAPRI_PHV_FIELD(SQCB_TO_WQE_P, log_pmtu), CAPRI_KEY_FIELD(IN_P, log_pmtu), CAPRI_PHV_RANGE(SQCB_TO_WQE_P, poll_in_progress, color), CAPRI_KEY_RANGE(IN_P, poll_in_progress, color) 
    phvwr CAPRI_PHV_FIELD(SQCB_TO_WQE_P, remaining_payload_bytes), K_REMAINING_PAYLOAD_BYTES
    //CAPRI_SET_FIELD2(SQCB_TO_WQE_P, pd, CAPRI_KEY_FIELD(IN_P, pd))
    
    phvwr CAPRI_PHV_FIELD(TO_S5_P, wqe_addr), r1

    // populate t0 PC and table address
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqwqe_process, r1)

    nop.e
    nop
