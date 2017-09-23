#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rqcb1_write_back_process_k_t k;
struct rqcb1_t d;


%%

.align
resp_rx_rqcb1_write_back_process:

    tblwr       d.current_sge_id, k.args.current_sge_id;
    tblwr       d.current_sge_offset, k.args.current_sge_offset;
    seq         c1, k.args.update_wqe_ptr, 1
    tblwr.c1    d.curr_wqe_ptr, k.args.curr_wqe_ptr;
    seq         c1, k.args.update_num_sges, 1
    tblwr.c1    d.num_sges, k.args.num_sges

    //assumption is that write back is called with table 2
    CAPRI_SET_TABLE_3_VALID(0)

    nop.e
    nop
