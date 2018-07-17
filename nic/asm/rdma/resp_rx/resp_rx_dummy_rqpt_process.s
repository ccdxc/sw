#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s1_t0_k k;

#define IN_P t0_s2s_rqcb_to_wqe_info
#define GLOBAL_FLAGS    r7

#define K_WQE_PTR CAPRI_KEY_RANGE(IN_P, curr_wqe_ptr_sbit0_ebit7, curr_wqe_ptr_sbit56_ebit63)

%%
    .param  resp_rx_rqwqe_mpu_only_process

.align
resp_rx_dummy_rqpt_process:

    // invoke rqwqe mpu only
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_rqwqe_mpu_only_process, K_WQE_PTR)
