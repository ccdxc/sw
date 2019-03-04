#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "rqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct rqcb2_t d;

#define IN_TO_S_P to_s6_info

%%

.align
rdma_aq_tx_query_rqcb2_process:

    seq         c1, d.syndrome, ACK_SYNDROME
    sub.!c1     r1, d.ack_nak_psn, 1
    cmov        r2, c1, d.ack_nak_psn, r1
    phvwrpair   p.query_sq.rnr_timer, d.rnr_timeout, p.query_sq.rq_psn, r2

exit: 
    CAPRI_SET_TABLE_1_VALID(0)
    nop.e
    nop
