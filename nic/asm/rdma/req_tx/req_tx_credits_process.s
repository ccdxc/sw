#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_credits_process_k_t k;
struct sqcb0_t d;

%%

.align
req_tx_credits_process:
    //  if ((sqcb1_to_credits_info_p->need_credits == FALSE) &&
    //      (sqcb0_p->need_credits != info_p->need_credits))
    //      sqcb0_p->need_credits = sqcb1_to_credits_info_p->need_credits     
    seq            c1, k.args.need_credits, 1
    seq            c2, d.need_credits, k.args.need_credits 
    bcf            [c1 | c2], skip_credit_flag_update
    nop
    tblwr          d.need_credits, k.args.need_credits

    // Do no set need_credits flag in sqcb0 if credits are needed. This way
    // stage0 can allow first packet to go through with "ack_req" bit 
skip_credit_flag_update:
    // sqcb0_p->busy = False
    tblwr          d.busy, 0
    tblwr          d.cb1_busy, 0

    CAPRI_SET_TABLE_0_VALID(0)
    phvwr  p.common.p4_intr_global_drop, 1

    nop.e
    nop
