#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "defines.h"

struct req_tx_phv_t p;
struct req_tx_s4_t0_k k;
struct key_entry_aligned_t d;


#define IN_P t0_s2s_sge_to_lkey_info

#define K_SGE_INDEX CAPRI_KEY_FIELD(IN_P, sge_index)
#define K_LKEY_LOCAL_INVALIDATE CAPRI_KEY_FIELD(IN_P, lkey_invalidate)

%%

.align
req_tx_sqlkey_invalidate_process:

    // Set table valid to 0.
    add          r1, K_SGE_INDEX, r0
    CAPRI_SET_TABLE_I_VALID(r1, 0)

    // it is an error to invalidate an MR not eligible for invalidation
    and          r2, d.flags, MR_FLAG_INV_EN
    beq          r2, r0, error_completion

    // it is an error to invalidate an MR in INVALID state
    seq          c1, d.state, KEY_STATE_INVALID // BD-slot 
    bcf          [c1], error_completion
    nop    //BD slot

    // Update state to FREE
    tblwr        d.state, KEY_STATE_FREE

    nop.e
    nop

error_completion:
    // Set completion status to Memory-Management-Operation-Error
    phvwrpair      p.rdma_feedback.feedback_type, RDMA_COMPLETION_FEEDBACK, p.rdma_feedback.completion.status, CQ_STATUS_MEM_MGMT_OPER_ERR
    // Set error-disable-qp. TODO: Using just as a place-holder. Full-blown error_disable_qp code will follow.
    phvwr.e        CAPRI_PHV_FIELD(phv_global_common, error_disable_qp),  1
    nop

exit:
    nop.e
    nop
