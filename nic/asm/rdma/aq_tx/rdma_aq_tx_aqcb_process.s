#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "common_phv.h"
#include "defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct aqcb_t d;
struct aq_tx_s0_t0_k k;

%%

.align
rdma_aq_tx_aqcb_process:

    phvwr   p.common.p4_intr_global_drop, 1
    nop.e
    nop //Exit Slot
