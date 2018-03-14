#include "capri.h"
#include "req_tx.h"
#include "rqcb.h"
#include "common_phv.h"

struct req_tx_phv_t p;
struct req_tx_sqwqe_process_k_t k;

%%
    .param  req_tx_sqwqe_process

.align
req_tx_dummy_sqpt_process:

    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqwqe_process, k.intrinsic.table_addr);

    nop.e
    nop
