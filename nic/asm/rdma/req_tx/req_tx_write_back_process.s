#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s6_t2_k k;
struct sqcb0_t d;

#define IN_P t2_s2s_sqcb_write_back_info
#define IN_TO_S_P to_s6_sqcb_wb_add_hdr_info

#define K_SPEC_CINDEX        CAPRI_KEY_RANGE(IN_TO_S_P, spec_cindex_sbit0_ebit7, spec_cindex_sbit8_ebit15)
#define K_NUM_SGES           CAPRI_KEY_RANGE(IN_P, num_sges_sbit0_ebit2, num_sges_sbit3_ebit7)
#define K_CURRENT_SGE_ID     CAPRI_KEY_RANGE(IN_P, current_sge_id_sbit0_ebit2, current_sge_id_sbit3_ebit7)
#define K_CURRENT_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, current_sge_offset_sbit0_ebit2, current_sge_offset_sbit27_ebit31)
#define K_WQE_ADDR           CAPRI_KEY_FIELD(IN_TO_S_P, wqe_addr)

%%

.align
req_tx_write_back_process:

    // Speculation is validated in add_headers/sqcb2_write_back, so
    // no need to validate here again.

    bbeq          CAPRI_KEY_FIELD(IN_P, rate_enforce_failed), 1, rate_enforce_fail
    nop           // Branch Delay Slot
    
    bbeq          CAPRI_KEY_FIELD(IN_P, poll_failed), 1, poll_fail
    nop           // Branch Delay Slot

    bbeq          K_GLOBAL_FLAG(_error_disable_qp), 1, error_disable_exit
    nop

    bbeq          CAPRI_KEY_FIELD(IN_TO_S_P, fence), 1, fence
    nop           // Branch Delay Slot

write_back:
    tblwr         d.busy, CAPRI_KEY_FIELD(IN_P, busy)
    tblwr         d.in_progress, CAPRI_KEY_FIELD(IN_P, in_progress)
    tblwr         d.num_sges, K_NUM_SGES
    tblwr         d.current_sge_id, K_CURRENT_SGE_ID
    tblwr         d.current_sge_offset, K_CURRENT_SGE_OFFSET
    tblwr         d.curr_wqe_ptr, K_WQE_ADDR
    tblwr         d.fence, 0
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
    // Update drain cindex every time first/only pkt is transmitted.
    // If state changes to SQD through admin Q, stage0 sends a sq_drain
    // feedback as a flush marker beyond which no phvs are sent from stage0
    // Whenever this feedback phv is seen by writebac (sq_drained == 1),
    // it can stop updating sqd_cindex.
    seq           c2, CAPRI_KEY_FIELD(IN_P, first), 1
    sne.c2        c2, d.sq_drained, 1
    tblwr.c2      d.sqd_cindex, SQ_C_INDEX

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

fence:
    // Set fence bit and curr_wqe_ptr for stage0.
    tblwr        d.fence, 1
    tblwr        d.curr_wqe_ptr, K_WQE_ADDR
    phvwr.e      p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_2_VALID(0)

poll_fail:
    DOORBELL_NO_UPDATE(K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, r1, r2)
    tblwr        d.poll_in_progress, 0 
    phvwr.e      p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_2_VALID(0)

rate_enforce_fail:
    bbeq        d.dcqcn_rl_failure, 1, drop_phv
    nop // BD-slot
    tblwr       d.dcqcn_rl_failure, 1
    // fall-through

drop_phv:
    phvwr.e      p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_2_VALID(0)

error_disable_exit:
    // Update sqd_cindex to one before the cindex for which error is
    // encountered. Flush feedback is sent to RXDMA to change state
    // to SQD_ON_ERR and responses are awaited till sqd_cindex. This
    // ensures that even though out of order error is encountered
    // all completions are posted until the errored wqe before error
    // disabling QP
    add            r1, SQ_C_INDEX, 0
    mincr          r1, d.log_num_wqes, -1
    tblwr          d.sqd_cindex, r1

#if !(defined (HAPS) || defined (HW))
    /*
     *  TODO: Incrementing cindex to satisfy model. Ideally, on error disabling we should just exit and be
     *  in the same state which caused the error.
     */
    tblmincri      SQ_C_INDEX, d.log_num_wqes, 1
#endif

    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop

