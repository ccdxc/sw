#include "capri.h"
#include "aq_rx.h"
#include "aqcb.h"
#include "rqcb.h"    
#include "common_phv.h"
#include "types.h"

struct aq_rx_phv_t p;
struct rqcb1_t d;
struct aq_rx_s5_t2_k k;

%%

.align
rdma_aq_rx_destroy_rqcb1_process:

    tblwr       d.state, QP_STATE_RESET
    phvwrpair   p.mod_qp.rq_state, QP_STATE_RESET, p.mod_qp.log_rsq_size, d.log_rsq_size

done:
    nop.e
    nop
