#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct req_tx_s6_t3_k k;
struct sqwqe_t d;

%%

.align
req_tx_sqwqe_fetch_wrid_process:
    // Update wrid in feedback PHV for posting completion.
    phvwr.e          p.rdma_feedback.completion.wrid, d.base.wrid
    CAPRI_SET_TABLE_3_VALID(0)//BD-slot
