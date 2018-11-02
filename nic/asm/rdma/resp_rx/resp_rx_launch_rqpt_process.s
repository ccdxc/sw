#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s1_t0_k k;

#define IN_P t0_s2s_rqcb_to_launch_rqpt_info     
#define K_C_INDEX CAPRI_KEY_RANGE(IN_P, c_index_sbit0_ebit7, c_index_sbit8_ebit15)
#define K_RQ_IN_HBM CAPRI_KEY_FIELD(IN_P, rq_in_hbm)
#define K_LOG_NUM_WQES CAPRI_KEY_FIELD(IN_P, log_num_wqes)
#define K_LOG_RQ_PAGE_SIZE CAPRI_KEY_FIELD(IN_P, log_rq_page_size)
#define K_LOG_WQE_SIZE CAPRI_KEY_RANGE(IN_P, log_wqe_size_sbit0_ebit2, log_wqe_size_sbit3_ebit4)
#define K_LOG_PMTU CAPRI_KEY_FIELD(IN_P, log_pmtu)
#define K_PT_BASE_ADDR CAPRI_KEY_RANGE(IN_P, pt_base_or_hbm_rq_base_addr_sbit0_ebit7, pt_base_or_hbm_rq_base_addr_sbit24_ebit31)
#define K_REM_PYLD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit15)
#define K_CQ_ID CAPRI_KEY_RANGE(IN_P, cq_id_sbit0_ebit7, cq_id_sbit16_ebit23)

#define IN_TO_S_P to_s1_launch_rqpt_info
#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_RANGE(IN_TO_S_P, cqcb_base_addr_hi_sbit0_ebit15, cqcb_base_addr_hi_sbit16_ebit23)

#define CQCB_ADDR       r6

#define OUT_P t0_s2s_launch_rqpt_to_rqpt_info

%%
    .param      resp_rx_rqpt_process

.align
resp_rx_launch_rqpt_process:

    bbeq        K_GLOBAL_FLAG(_completion), 0, skip_cqcb_prefetch
    CAPRI_RESET_TABLE_0_ARG()   //BD Slot

    CQCB_ADDR_GET(CQCB_ADDR, K_CQ_ID, K_CQCB_BASE_ADDR_HI)
    cpref       CQCB_ADDR

skip_cqcb_prefetch:
    bbne        K_RQ_IN_HBM, 1, host_q_process
    add         r1, r0, K_C_INDEX   //BD Slot

hbm_q_process:
    sll         r2, r1, K_LOG_WQE_SIZE 
    add         r2, r2, K_PT_BASE_ADDR, HBM_SQ_BASE_ADDR_SHIFT
    // r2 now has hbm_wqe addr
    
    //prefetch next wqe
    mincr       r1, K_LOG_NUM_WQES, 1
    sll         r3, r1, K_LOG_WQE_SIZE
    add         r3, r3, K_PT_BASE_ADDR, HBM_SQ_BASE_ADDR_SHIFT
    // r3 now has next hbm_wqe addr, prefetch it
    cpref       r3
    
    phvwrpair   CAPRI_PHV_FIELD(OUT_P, hbm_wqe_ptr), r2, \
                CAPRI_PHV_FIELD(OUT_P, rq_in_hbm), K_RQ_IN_HBM
    phvwrpair   CAPRI_PHV_FIELD(OUT_P, log_pmtu), K_LOG_PMTU, \
                CAPRI_PHV_FIELD(OUT_P, remaining_payload_bytes), K_REM_PYLD_BYTES

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqpt_process, r3)

host_q_process:

    sub         r2, K_LOG_RQ_PAGE_SIZE, K_LOG_WQE_SIZE
    // page_index = c_index >> (log_rq_page_size - log_wqe_size)
    srlv        r3, r1, r2

    // page_offset = c_index & ((1 << (log_rq_page_size - log_wqe_size))-1) << log_wqe_size
    mincr       r1, r2, r0
    sll         r1, r1, K_LOG_WQE_SIZE

    // r3 has page_index, r1 has page_offset by now

    // page_seg_offset = page_index & 0x7
    and         r5, r3, CAPRI_SEG_PAGE_MASK
    // page_index = page_index & ~0x7
    sub         r3, r3, r5
    // page_index = page_index * sizeof(u64)
    // page_index += rqcb_p->pt_base_addr
    sll         r4, K_PT_BASE_ADDR, PT_BASE_ADDR_SHIFT
    add         r3, r4, r3, CAPRI_LOG_SIZEOF_U64
    // now r3 has page_p to load
    
    phvwrpair   CAPRI_PHV_FIELD(OUT_P, page_seg_offset), r5, \
                CAPRI_PHV_FIELD(OUT_P, page_offset), r1
    phvwrpair   CAPRI_PHV_FIELD(OUT_P, log_pmtu), K_LOG_PMTU, \
                CAPRI_PHV_FIELD(OUT_P, remaining_payload_bytes), K_REM_PYLD_BYTES

    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqpt_process, r3)
