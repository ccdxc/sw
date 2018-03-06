#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rqwqe_process_k_t k;

%%
    .param  resp_rx_rqwqe_process

.align
resp_rx_dummy_rqpt_process:

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqwqe_process, k.args.curr_wqe_ptr);

    nop.e
    nop
