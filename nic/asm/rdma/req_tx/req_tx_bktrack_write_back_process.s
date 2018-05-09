#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s2_t0_k k;
struct sqcb0_t d;

#define IN_P t0_s2s_sqcb_write_back_info
#define IN_TO_S_P to_s2_bktrack_to_stage

#define K_NUM_SGES CAPRI_KEY_RANGE(IN_P, num_sges_sbit0_ebit2, num_sges_sbit3_ebit7)
#define K_CURRENT_SGE_ID CAPRI_KEY_RANGE(IN_P, current_sge_id_sbit0_ebit2, current_sge_id_sbit3_ebit7)
#define K_CURRENT_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit2, current_sge_offset_sbit27_ebit31)
#define K_SQ_C_INDEX CAPRI_KEY_RANGE(IN_P, sq_c_index_sbit0_ebit2, sq_c_index_sbit11_ebit15)
#define K_WQE_ADDR CAPRI_KEY_FIELD(IN_TO_S_P, wqe_addr)

%%

.align
req_tx_bktrack_write_back_process:

     tblwr         d.busy, CAPRI_KEY_FIELD(IN_P, busy)
     //seq           c1, k.args.release_cb1_busy, 1
     //tblwr.c1      d.cb1_busy, 0
     tblwr         d.num_sges, K_NUM_SGES
     tblwr         d.bktrack_in_progress, CAPRI_KEY_FIELD(IN_P, bktrack_in_progress)
     tblwr         d.current_sge_id, K_CURRENT_SGE_ID
     tblwr         d.current_sge_offset, K_CURRENT_SGE_OFFSET
     tblwr         d.curr_wqe_ptr, K_WQE_ADDR

     seq           c2, CAPRI_KEY_FIELD(IN_P, bktrack_in_progress), 1
     bcf           [c2], update_spec_cindex
     // set SQ c_index to the backtracked value
     tblwr         SQ_C_INDEX, K_SQ_C_INDEX // Branch Delay Slot

     // Empty backtrack and retransmit timer rings
     tblwr         SQ_BKTRACK_C_INDEX, SQ_BKTRACK_P_INDEX
     tblwr         SQ_TIMER_C_INDEX, SQ_TIMER_P_INDEX

update_spec_cindex:
 
     // Set speculative cindex to next cindex so that speculative wqe 
     // processing can take place after backtrack 
     tblwr         SPEC_SQ_C_INDEX, SQ_C_INDEX
     bbeq          CAPRI_KEY_FIELD(IN_P, in_progress), 0, end
     CAPRI_SET_TABLE_0_VALID(0) // Branch Delay Slot

     tblmincri     SPEC_SQ_C_INDEX,  d.log_num_wqes, 1
     phvwr  p.common.p4_intr_global_drop, 1


end:
     nop.e
     nop
