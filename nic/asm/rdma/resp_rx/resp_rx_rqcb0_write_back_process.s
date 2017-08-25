#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rqcb0_write_back_process_k_t k;
struct rqcb0_t d;


%%

.align
resp_rx_rqcb0_write_back_process:
    
    tblwr       d.in_progress, k.args.in_progress
    seq         c1, k.args.incr_nxt_to_go_token_id, 1
    tbladd.c1   d.nxt_to_go_token_id, 1

    seq         c1, k.args.incr_c_index, 1
    bcf         [!c1], incr_c_index_exit
    seq         c2, k.args.cache, 1     //BD slot

    .csbegin
    cswitch     [c2]
    nop
    
    .cscase 0
    // cache disabled
    //TODO: Migrate to Doorbell 
    //TODO: wraparound
    tblmincri   RQ_C_INDEX, d.log_num_wqes, 1
    
    b           incr_c_index_exit
    nop

    .cscase 1
    // TODO: cache enabled
    b           incr_c_index_exit
    nop
    .csend

incr_c_index_exit:

    //assumption is that write back is called with table 2
    CAPRI_SET_TABLE_2_VALID(0)

    nop.e
    nop
