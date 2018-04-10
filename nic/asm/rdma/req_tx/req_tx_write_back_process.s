#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s5_t2_k k;
struct sqcb0_t d;

#define IN_P t2_s2s_sqcb_write_back_info
#define IN_TO_S_P to_s5_sq_to_stage

#define K_SPEC_CINDEX        CAPRI_KEY_RANGE(IN_TO_S_P, spec_cindex_sbit0_ebit7, spec_cindex_sbit8_ebit15)
#define K_NUM_SGES           CAPRI_KEY_RANGE(IN_P, num_sges_sbit0_ebit2, num_sges_sbit3_ebit7)
#define K_CURRENT_SGE_ID     CAPRI_KEY_RANGE(IN_P, current_sge_id_sbit0_ebit2, current_sge_id_sbit3_ebit7)
#define K_CURRENT_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit2, current_sge_offset_sbit27_ebit31)
#define K_WQE_ADDR           CAPRI_KEY_FIELD(IN_TO_S_P, wqe_addr)

%%

.align
req_tx_write_back_process:

    // if speculative cindex matches cindex, then this wqe is being
    // processed in the right order and state update is allowed. Otherwise
    // discard and continue with speculation until speculative cindex
    // matches current cindex. If sepculative cindex doesn't match cindex, then
    // revert speculative cindex to cindex , which would allow next speculation
    // to continue from yet to be processed wqe
    seq           c1, K_SPEC_CINDEX, SQ_C_INDEX 
    bcf           [!c1], spec_fail
    nop           // Branch Delay Slot    

    bbeq          CAPRI_KEY_FIELD(IN_TO_S_P, rate_enforce_failed), 1, rate_enforce_fail
    nop           // Branch Delay Slot
    
    bbeq          CAPRI_KEY_FIELD(IN_P, poll_failed), 1, poll_fail
    nop           // Branch Delay Slot

write_back:
    tblwr         d.busy, CAPRI_KEY_FIELD(IN_P, busy)
    tblwr         d.num_sges, K_NUM_SGES
    tblwr         d.current_sge_id, K_CURRENT_SGE_ID
    tblwr         d.current_sge_offset, K_CURRENT_SGE_OFFSET
    tblwr         d.curr_wqe_ptr, K_WQE_ADDR
    bbeq          CAPRI_KEY_FIELD(IN_P, poll_in_progress), 0, skip_poll_success
    seq           c1, CAPRI_KEY_FIELD(IN_P, last_pkt), 1 // Branch Delay Slot
    tblwr         d.poll_in_progress, 0
    tblmincri.c1  SPEC_SQ_C_INDEX, d.log_num_wqes, 1
    DOORBELL_NO_UPDATE(K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, r2, r3)
#ifdef CAPRI_IGNORE_TIMESTAMP
#else
    #on non-RTL
    #in case of standalone model, DOL would not have incremented pindex
    #upon success in polling, do it in the program
    tblmincri     SQ_P_INDEX, d.log_num_wqes, 1
#endif

skip_poll_success:
    // if (write_back_info_p->last)
    // RING_C_INDEX_INCREMENT(sqcb0_p, SQ_RING_ID)
    tblmincri.c1  SQ_C_INDEX, d.log_num_wqes, 1
    seq           c2, SQ_C_INDEX, r0
    tblmincri.c2  d.color, 1, 1

    // Ordering rules:
    // We decided NOT to ring doorbell to update the c_index and there by re-evaluate scheduler.
    // The decision is taken because of the ordering complexity between cb0/cb1 busy bit release
    // and cindex increments.
    // Rather, we simply use above tblmincri to increment c_index using table writes.
    // Since doorbell is not re-evaluated, scheduler may invoke us one more time.
    // Then in stage0 we check r7 register (which has a bitmap of rings not empty). If this register
    // is 0, then we invoke doorbell to turn off scheduler.
    // This way, we don't need to invoke doorbell invocation (which is 4-5 instructions) for every
    // wqe consumption. As long as qp has enough work posted, we should be fine. Disadvantage is
    // that if wqes are posted sporadically, we may encounter one more scheduler invocation for
    // every wqe that is posted.
    //DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, SQ_RING_ID, SQ_C_INDEX, r2, r3)

    // TODO Comemnt out for now until backtracking logic is fixed
    #bbeq          d.retry_timer_on, 1, exit
    #tblwr         d.retry_timer_on, 1 //BD slot
    #CAPRI_START_SLOW_TIMER(r1, r6, k.global.lif, k.global.qtype, k.global.qid, TIMER_RING_ID, 30)

exit:
    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop

poll_fail:
    DOORBELL_NO_UPDATE(K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, r1, r2)
    tblwr        d.poll_in_progress, 0 
    phvwr.e      p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_2_VALID(0)

rate_enforce_fail:
    bbeq        d.dcqcn_rl_failure, 1, spec_fail
    nop // BD-slot
    tblwr       d.dcqcn_rl_failure, 1
    // fall-through

spec_fail:
    phvwr.e      p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_2_VALID(0)
