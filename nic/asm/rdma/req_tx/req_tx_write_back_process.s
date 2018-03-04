#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_write_back_process_k_t k;
struct sqcb0_t d;

%%
    .param    req_tx_add_headers_process

.align
req_tx_write_back_process:

    // if speculative cindex matches cindex, then this wqe is being
    // processed in the right order and state update is allowed. Otherwise
    // discard and continue with speculation until speculative cindex 
    // matches current cindex. If sepculative cindex doesn't match cindex, then
    // revert speculative cindex to cindex , which would allow next speculation
    // to continue from yet to be processed wqe
    bbeq          k.to_stage.sq.rate_enforce_failed, 1, dcqcn_reset_spec_cindex
    seq           c1, k.to_stage.sq.spec_cindex, SQ_C_INDEX 
    bcf           [!c1], revert_spec_cindex
    nop           // Branch Delay Slot    

write_back:
    tblwr         d.busy, k.args.busy
    seq           c1, k.args.release_cb1_busy, 1
    tblwr.c1      d.cb1_busy, 0
    tblwr         d.num_sges, k.args.num_sges
    tblwr         d.in_progress, k.args.in_progress
    tblwr         d.current_sge_id, k.args.current_sge_id
    tblwr         d.current_sge_offset, k.args.current_sge_offset
    tblwr         d.curr_wqe_ptr, k.to_stage.sq.wqe_addr
    seq           c1, k.args.last, 1
    bbeq          k.to_stage.sq.poll_in_progress, 0, skip_poll_success
    tblwr         d.curr_op_type, k.args.op_type //Branch Delay Slot
    //set poll_success to 1 and poll_in_progress to 0
    tblwr         d.{poll_success...poll_in_progress}, 0x2 
    tblmincri.c1  SPEC_SQ_C_INDEX, d.log_num_wqes, 1
#ifdef CAPRI_IGNORE_TIMESTAMP
#else
    #on non-RTL
    #in case of standalone model, DOL would not have incremented pindex
    #upon success in polling, do it in the program
    tblmincri.c1  SQ_P_INDEX, d.log_num_wqes, 1
#endif
    seq           c2, SQ_P_INDEX, r0
    tblmincri.c2  d.color, 1, 1

skip_poll_success:

    // if (write_back_info_p->last)
    // RING_C_INDEX_INCREMENT(sqcb0_p, SQ_RING_ID)
    tblmincri.c1  SQ_C_INDEX, d.log_num_wqes, 1

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

    SQCB1_ADDR_GET(r1)
    //It is assumed that hdr_template_inline flag is passed untouched to next table-3.
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_add_headers_process, r1)
 
    bbeq          d.retry_timer_on, 1, exit
    tblwr         d.retry_timer_on, 1 //BD slot
    CAPRI_START_SLOW_TIMER(r1, r6, k.global.lif, k.global.qtype, k.global.qid, TIMER_RING_ID, 30)

exit:
     nop.e
     nop

revert_spec_cindex:
    bbeq          k.to_stage.sq.poll_in_progress, 1, skip_revert
    add           r1, SQ_C_INDEX, 0
    mincr         r1, d.log_num_wqes, 1
    tblwr         SPEC_SQ_C_INDEX, r1
skip_revert:
    phvwr         p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_3_VALID(0)

    nop.e
    nop

dcqcn_reset_spec_cindex:                         
    tblwr         SPEC_SQ_C_INDEX, SQ_C_INDEX    
    phvwr         p.common.p4_intr_global_drop, 1                                                   
    CAPRI_SET_TABLE_3_VALID(0)                   
    nop.e               
    nop                 
