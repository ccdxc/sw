#include "capri.h"
#include "req_tx.h"
#include "rqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct req_tx_s1_t0_k k;

#define IN_P t0_s2s_sqcb_to_wqe_info
%%
    .param  req_tx_sqwqe_process

.align
req_tx_dummy_sqpt_process:

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqwqe_process, k.common_te0_phv_table_addr)

    nop.e
    nop
