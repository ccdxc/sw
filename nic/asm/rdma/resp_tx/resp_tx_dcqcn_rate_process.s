#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct dcqcn_cb_t d;
struct resp_tx_rqcb1_process_k_t k;

#define  RATE_ENFORCED  r3
#define  TARGET_RATE    r4
#define  F              r5
#define  Ri             r6

%%
resp_tx_dcqcn_rate_process:

    // Pin dcqcn_algo to stage 4
    mfspr   r1, spr_mpuid
    seq     c1, r1[6:2], STAGE_4
    bcf     [!c1], bubble_to_next_stage
    nop

    // Update cindex and ring-doorbell.
    DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, DCQCN_RING_ID, k.to_stage.s4.dcqcn.new_cindex, r1, r2) //BD-slot

    // Load F with 5 iterations. Check should F be configurable??
    addi     F, r0, 5

    //  Find if Max(T, BC) < F. If yes, jump to fast_recovery
    slt     c2, d.byte_counter_exp_cnt, d.timer_exp_cnt
    cmov    r1, c2, d.timer_exp_cnt, d.byte_counter_exp_cnt
    ble     r1, F, fast_recovery
    CAPRI_SET_TABLE_0_VALID(0) //BD-slot

    // Find if Min(T, BC) > F. If yes, jump to hyper_increase.
    cmov    r1, c2, d.byte_counter_exp_cnt, d.timer_exp_cnt
    sle     c3, r1, F
    bcf     [!c3], hyper_increase
    nop

additive_increase:
    // Rt = Rt + Ri. 
    //TODO: Check if Ri should be configurable?? Setting it to 5 Mbps by default.
    addi    Ri, r0, 5
    add     TARGET_RATE, d.target_rate, Ri

    // Rc = ((Rt + Rc) / 2)
    add     r1, d.rate_enforced, TARGET_RATE
    srl     RATE_ENFORCED, r1, 1

    tblwr   d.rate_enforced, RATE_ENFORCED
    tblwr   d.target_rate, TARGET_RATE
    nop.e
    nop

fast_recovery:
    // Rc = ((Rt + Rc) / 2)
    add     r1, d.target_rate, d.rate_enforced
    srl     RATE_ENFORCED, r1, 1

    tblwr   d.rate_enforced, RATE_ENFORCED
    nop.e
    nop

hyper_increase:
    // Rt = Rt + (i * Ri). 
    //TODO:Check if Ri and i should be configurable?? Setting it to 5 Mbps and 3 by default.
    addi    Ri, r0, 5
    muli    r1, Ri, 3
    add     TARGET_RATE, d.target_rate, r1

    // Rc = ((Rt + Rc) / 2)
    add     r1, d.rate_enforced, TARGET_RATE
    srl     RATE_ENFORCED, r1, 1

    tblwr   d.rate_enforced, RATE_ENFORCED
    tblwr   d.target_rate, TARGET_RATE

    nop.e
    nop

bubble_to_next_stage:
    seq           c1, r1[6:2], STAGE_3
    bcf           [!c1], exit
    nop           // Branch Delay Slot

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS)

exit:
    nop.e
    nop

