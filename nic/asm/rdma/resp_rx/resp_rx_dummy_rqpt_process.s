#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s1_t0_k k;

#define IN_P t0_s2s_rqcb_to_wqe_info
#define IN_TO_S_P to_s1_dummy_rqpt_info

#define GLOBAL_FLAGS    r7
#define CQCB_ADDR       r6

#define K_WQE_PTR CAPRI_KEY_RANGE(IN_P, curr_wqe_ptr_sbit0_ebit7, curr_wqe_ptr_sbit56_ebit63)
#define K_CQ_ID CAPRI_KEY_FIELD(IN_TO_S_P, cq_id)
#define K_CQCB_BASE_ADDR_HI CAPRI_KEY_RANGE(IN_TO_S_P, cqcb_base_addr_hi_sbit0_ebit15, cqcb_base_addr_hi_sbit16_ebit23)

%%
    .param  resp_rx_rqwqe_mpu_only_process

.align
resp_rx_dummy_rqpt_process:

    bbeq    K_GLOBAL_FLAG(_completion), 0, skip_cqcb_prefetch
    CQCB_ADDR_GET(CQCB_ADDR, K_CQ_ID, K_CQCB_BASE_ADDR_HI);
    cpref   CQCB_ADDR


skip_cqcb_prefetch:

    // invoke rqwqe mpu only
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqwqe_mpu_only_process, K_WQE_PTR)
