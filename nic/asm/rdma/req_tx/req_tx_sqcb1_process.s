#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_sqcb1_process_k_t k;
struct sqcb1_t d;

#define SQCB1_TO_CREDITS_T struct req_tx_sqcb1_to_credits_info_t

%%
    .param req_tx_credits_process

.align
req_tx_sqcb1_process:
    // if (sqcb0_to_sqcb1_info_p->update_credits)
    seq            c1, k.args.update_credits, 1
    bcf            [!c1], chk_bktrack
    nop            // Branch Delay Slot

    //  if (sqcb1_p->credits)
    //      credits = 1 << (sqcb1_p->credits >> 1); 
    //  sqcb1_p->lsn = sqcb1_p->msn + credits
    DECODE_NAK_SYNDROME_CREDITS(r1, d.credits, c1)
    add            r1, d.msn, r1
    tblwr          d.lsn, r1
    
    CAPRI_GET_TABLE_0_ARG(req_tx_phv_t, r7)
    // sqcb1_to_credits_info_p->need_credits = TRUE
    // if (sqcb1_p->ssn <= sqcb1_p->lsn)
    //     sqcb1_to_credits_info_p->need_credits = FALSE
    scwle24        c1, d.ssn, r1
    CAPRI_SET_FIELD_C(r7, SQCB1_TO_CREDITS_T, need_credits, 1, !c1)

    SQCB0_ADDR_GET(r1)
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_SET_RAW_TABLE_PC(r6, req_tx_credits_process)
    CAPRI_NEXT_TABLE_I_READ(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, r6, r1)

    nop.e
    nop

chk_bktrack:


    nop.e
    nop
