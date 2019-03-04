#include "capri.h"
#include "aq_rx.h"
#include "aqcb.h"
#include "rqcb.h"    
#include "common_phv.h"
#include "types.h"

struct aq_rx_phv_t p;
struct rqcb1_t d;
struct aq_rx_s5_t2_k k;

#define IN_P t2_s2s_sqcb1_to_rqcb1_info
#define IN_TO_S_P to_s5_info

%%

.align
rdma_aq_rx_query_rqcb1_process:

    phvwr       p.query_rq.rsq_depth, d.log_rsq_size

done:
    nop.e
    nop
    
