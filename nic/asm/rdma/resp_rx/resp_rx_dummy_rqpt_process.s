#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rqwqe_process_k_t k;

#define INFO_OUT1_T struct resp_rx_rqcb_to_wqe_info_t

#define TBL_KEY_P    r4
#define GLOBAL_FLAGS r7

%%
    .param  resp_rx_rqwqe_wrid_process
    .param  resp_rx_rqwqe_process

.align
resp_rx_dummy_rqpt_process:

    add         GLOBAL_FLAGS, r0, k.global.flags

    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, TBL_KEY_P)

    ARE_ALL_FLAGS_SET(c1, GLOBAL_FLAGS, RESP_RX_FLAG_WRITE|RESP_RX_FLAG_IMMDT)

    // if write_with_imm, load resp_rx_rqwqe_wrid_process, 
    // else load resp_rx_rqwqe_process
    CAPRI_NEXT_TABLE_I_READ_PC_C(TBL_KEY_P, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_wrid_process, resp_rx_rqwqe_process, k.args.curr_wqe_ptr, c1)

    nop.e
    nop
