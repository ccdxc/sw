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
     tblwr         d.num_sges, k.args.num_sges
     tblwr         d.in_progress, k.args.in_progress
     tblwr         d.current_sge_id, k.args.current_sge_id
     tblwr         d.current_sge_offset, k.args.current_sge_offset
     tblwr         d.curr_wqe_ptr, k.to_stage.bktrack.wqe_addr
     tblwr         d.curr_op_type, k.args.op_type

     seq           c2, k.args.empty_rrq_bktrack, 1
     bcf           [!c2], end
     // set SQ c_index to the backtracked value
     tblwr         SQ_C_INDEX, k.args.sq_c_index // Branch Delay Slot

     // Empty RRQ ring
     tblwr         RRQ_C_INDEX, RRQ_P_INDEX
     // Empty backtrack and retransmit timer rings
     tblwr         SQ_BKTRACK_C_INDEX, SQ_BKTRACK_P_INDEX
     tblwr         SQ_TIMER_C_INDEX, SQ_TIMER_P_INDEX

end:
     CAPRI_SET_TABLE_0_VALID(0)
 
     // Set speculative cindex to next cindex so that speculative wqe 
     // processing can take place after backtrack 
     tblwr         SPEC_SQ_C_INDEX, SQ_C_INDEX
     nop.e
     nop
