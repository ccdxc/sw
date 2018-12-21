#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct req_tx_s6_t3_k k;
struct sqwqe_t d;

%%

    .param  req_tx_stats_process

.align
req_tx_sqwqe_fetch_wrid_process:

    //invoke stats process from here - for packet generating, error-disabled case
    SQCB4_ADDR_GET(r5)
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_stats_process, r5)

    // Update wrid in feedback PHV for posting completion.
    phvwr.e          p.rdma_feedback.completion.wrid, d.base.wrid
    nop
