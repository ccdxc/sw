#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_credits_process_k_t k;
struct sqcb2_t d;

%%

.align
req_tx_credits_process:
    // Pin credits_process in the same stage as add_headers to update
    // lsn, in_progress, need_credits and cb1_busy
    mfspr          r1, spr_mpuid
    seq            c1, r1[4:2], STAGE_5
    bcf            [!c1], bubble_to_next_stage

    // set lsn based on received ack; lsn = msn + credits
    DECODE_ACK_SYNDROME_CREDITS(r1, d.credits, c1)
    mincr          r1, 24, d.msn
    tblwr          d.lsn, r1

    // unset need_credits if ssn <= lsn else do not modify as when there
    // is packet to send, add_headers will set need_credits after sending
    // the packet with "AckReq" bit. clear cb1_busy as busy was set in stage0
    // to process credits event
    scwle24        c1, d.ssn, r1
    add            r2, r0, d.in_progress, SQCB0_IN_PROGRESS_BIT_OFFSET 
    or.!c1         r2, r2, d.need_credits, SQCB0_NEED_CREDITS_BIT_OFFSET
    tblwr.c1       d.need_credits, 0
    SQCB0_ADDR_GET(r3)
    add            r3, r3, FIELD_OFFSET(sqcb0_t, cb1_byte)
    memwr.b        r3, r2

    CAPRI_SET_TABLE_3_VALID(0)
    phvwr p.common.p4_intr_global_drop, 1 

    nop.e
    nop

bubble_to_next_stage:
    seq            c1, r1[4:2], STAGE_4
    bcf            [!c1], end
    SQCB2_ADDR_GET(r1)

    // Should be in the same table and stage as add_headers to lock table
    CAPRI_GET_TABLE_3_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)
   
end:
    nop.e
    nop
