#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_write_back_process_k_t k;
struct sqcb0_t d;

%%

.align
req_tx_bktrack_write_back_process:

     tblwr         d.busy, k.args.busy
     //seq           c1, k.args.release_cb1_busy, 1
     //tblwr.c1      d.cb1_busy, 0
     tblwr         d.num_sges, k.args.num_sges
     tblwr         d.in_progress, k.args.in_progress
     tblwr         d.bktrack_in_progress, k.args.bktrack_in_progress
     tblwr         d.current_sge_id, k.args.current_sge_id
     tblwr         d.current_sge_offset, k.args.current_sge_offset
     tblwr         d.curr_wqe_ptr, k.to_stage.bktrack.wqe_addr
     //tblwr         d.curr_op_type, k.args.op_type

     seq           c2, k.args.bktrack_in_progress, 1
     bcf           [c2], update_spec_cindex
     // set SQ c_index to the backtracked value
     tblwr         SQ_C_INDEX, k.args.sq_c_index // Branch Delay Slot

     // backtrack RRQ ring TODO with cb_reorg this needs to be done in sqcb1
     //tblwr         RRQ_P_INDEX, RRQ_C_INDEX
     // Empty backtrack and retransmit timer rings
     tblwr         SQ_BKTRACK_C_INDEX, SQ_BKTRACK_P_INDEX
     //tblwr         SQ_TIMER_C_INDEX, SQ_TIMER_P_INDEX

update_spec_cindex:
 
     // Set speculative cindex to next cindex so that speculative wqe 
     // processing can take place after backtrack 
     tblwr         SPEC_SQ_C_INDEX, SQ_C_INDEX
     bbeq          k.args.in_progress, 0, end
     CAPRI_SET_TABLE_0_VALID(0) // Branch Delay Slot

     tblmincri     SPEC_SQ_C_INDEX,  d.log_num_wqes, 1
     phvwr  p.common.p4_intr_global_drop, 1


end:
     nop.e
     nop
