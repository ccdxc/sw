#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "sqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct sqcb0_t d;

#define IN_TO_S_P to_s6_info
    
%%

.align
rdma_aq_tx_query_sqcb0_process:

    seq         c1, d.service, RDMA_SERV_TYPE_UD
    phvwr.c1    p.query_sq.qkey_dest_qpn, d.q_key
    phvwr.!c1   p.query_sq.retry_timeout, d.local_ack_timeout

exit:
    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop
