#include "capri.h"
#include "aq_tx.h"
#include "aqcb.h"
#include "rqcb.h"    
#include "common_phv.h"
#include "p4/common/defines.h"
#include "types.h"

struct aq_tx_phv_t p;
struct rqcb2_t d;
struct aq_tx_s6_t1_k k;

#define IN_P t1_s2s_rqcb0_to_rqcb2_info
#define IN_TO_S_P to_s6_info

#define K_MIN_RNR_TIMER CAPRI_KEY_FIELD(IN_P, rnr_min_timer)
    
%%

.align
rdma_aq_tx_rqcb2_process:

min_rnr_timer:
    bbne        CAPRI_KEY_FIELD(IN_P, rnr_timer_valid), 1, exit
    nop
    
    tblwr       d.rnr_timeout, K_MIN_RNR_TIMER
    
exit: 
    CAPRI_SET_TABLE_1_VALID(0)
    nop.e
    nop
    
