#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s1_t0_k k;

#define IN_P t0_s2s_rqcb_to_wqe_info
#define GLOBAL_FLAGS    r7

%%
    .param  resp_rx_rqwqe_process
    .param  resp_rx_rqwqe_wrid_process

.align
resp_rx_dummy_rqpt_process:

    add         GLOBAL_FLAGS, r0, K_GLOBAL_FLAGS

    ARE_ALL_FLAGS_SET(c1, GLOBAL_FLAGS, RESP_RX_FLAG_WRITE|RESP_RX_FLAG_IMMDT)

    CAPRI_NEXT_TABLE0_READ_PC_CE(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_wrid_process, resp_rx_rqwqe_process, CAPRI_KEY_RANGE(IN_P, curr_wqe_ptr_sbit0_ebit7, curr_wqe_ptr_sbit56_ebit63), c1);
