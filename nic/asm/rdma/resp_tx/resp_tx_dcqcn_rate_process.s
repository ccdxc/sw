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

// Note: Below values are constants related to g and alpha.
// TODO: Hardcoding it for now. Check if they have to be fed from HAL.
#define G_MAX                   65536
#define LOG_G_MAX               16
#define ALPHA_MAX               65536
#define LOG_ALPHA_MAX           16
#define ALPHA_TIMER_INTERVAL    55

// Max-rate of QP in Mbps to shutdown DCQCN algorithm. DCQCN algo will be restarted again on receiving CNP.
// TODO: This can be fed as a loader-param from HAL.
#define QP_MAX_RATE             100000 

%%
resp_tx_dcqcn_rate_process:

    // Pin dcqcn_algo to stage 4
    mfspr   r1, spr_mpuid
    seq     c1, r1[4:2], STAGE_4
    bcf     [!c1], bubble_to_next_stage
    nop

    // Check if doorbell is for processing CNP packet.
    seq     c2, d.num_cnp_rcvd, d.num_cnp_processed 
    bcf     [!c2], cnp_recv_process

    // Load F with 5 iterations. Check should F be configurable??
    addi    F, r0, 5
    add     TARGET_RATE, d.target_rate, r0

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
    // Check target-rate has reached MAX-QP-RATE. If yes, stop further target-rate-increase.
    slt     c1, TARGET_RATE, QP_MAX_RATE
    bcf     [!c1], skip_target_rate_inc
    nop

    // Rt = Rt + Ri. 
    //TODO: Check if Ri should be configurable?? Setting it to 5 Mbps by default.
    addi    Ri, r0, 5
    add     TARGET_RATE, d.target_rate, Ri

    // Rc = ((Rt + Rc) / 2)
    add     r1, d.rate_enforced, TARGET_RATE
    srl     RATE_ENFORCED, r1, 1

    tblwr   d.rate_enforced, RATE_ENFORCED
    tblwr   d.target_rate, TARGET_RATE

    // Check enforced rate has reached MAX-QP-RATE. If yes, shutdown DCQCN timers to stop further rate-increase.
    slt     c1, RATE_ENFORCED, QP_MAX_RATE
    bcf     [c1], exit
    nop   // BD-slot
    tblwr   d.max_rate_reached, 1
    DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, DCQCN_TIMER_RING_ID, k.to_stage.s4.dcqcn.new_timer_cindex, r1, r2)
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
    // Check target-rate has reached MAX-QP-RATE. If yes, stop further target-rate-increase.
    slt     c1, d.target_rate, QP_MAX_RATE
    bcf     [!c1], skip_target_rate_inc
    nop
    // Rt = Rt + (i * Ri). 
    //TODO:Check if Ri and i should be configurable?? Setting it to 5 Mbps and 3 by default.
    addi    Ri, r0, 5
    muli    r1, Ri, 3
    add     TARGET_RATE, d.target_rate, r1

skip_target_rate_inc:
    // Rc = ((Rt + Rc) / 2)
    add     r1, d.rate_enforced, TARGET_RATE
    srl     RATE_ENFORCED, r1, 1

    tblwr   d.rate_enforced, RATE_ENFORCED
    tblwr   d.target_rate, TARGET_RATE

    // Check enforced rate has reached MAX-QP-RATE. If yes, shutdown DCQCN timers to stop further rate-increase.
    slt     c1, RATE_ENFORCED, QP_MAX_RATE
    bcf     [c1], exit
    nop   // BD-slot
    tblwr   d.max_rate_reached, 1
    DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, DCQCN_TIMER_RING_ID, k.to_stage.s4.dcqcn.new_timer_cindex, r1, r2)
    nop.e
    nop

cnp_recv_process:
    // Reset all dcqcn params
    tblwr   d.byte_counter_exp_cnt, 0
    tblwr   d.cur_byte_counter, 0
    tblwr   d.timer_exp_cnt, 0

    // Set cur-rate to target-rate. Rt = Rc
    tblwr   d.target_rate, d.rate_enforced

    // cut rate-enforced. 
    // Rc = ((Rc * (alpha_max - (int_alpha >> 1))) >> log_alpha_max)
    srl     r3, d.alpha_value, 1
    sub     r4, ALPHA_MAX, r3
    mul     r3, r4, d.rate_enforced
    srl     r4, r3, LOG_ALPHA_MAX
    tblwr   d.rate_enforced, r4

    // Update alpha value.                         
    // int_alpha =  (((g_max - int_g) * int_alpha) >> log_g_max) + int_g
    sub     r1, G_MAX, d.g_val
    mul     r2, d.alpha_value, r1
    srl     r2, r2, LOG_G_MAX
    add     r2, r2, d.g_val
    tblwr   d.alpha_value, r2

    // Update num-cnp-processed.
    tblmincri   d.num_cnp_processed, 8, 1
    CAPRI_SET_TABLE_0_VALID(0)
 
    bbeq    d.max_rate_reached, 0, exit
    tblwr   d.max_rate_reached, 0 // BD-slot
    /* 
     * Timer will be started only when CNP is received after max_rate_reached is hit. 
     * Init value of max_rate_reached will be set to 1 to kick-start dcqcn timers when first CNP is received.
     * Also, Timers will NOT be restarted everytime CNP is received.Impact of this should be minimal since 
     * timer T runs as a multiplicative factor to alpha-timer. So not restarting alpha timer here 
     * should have minimal impact on timer T and subsequent dcqcn rate-increase.
     */
    CAPRI_START_SLOW_TIMER(r1, r6, k.global.lif, k.global.qtype, k.global.qid, DCQCN_TIMER_RING_ID, ALPHA_TIMER_INTERVAL)
    nop.e
    nop

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    bcf           [!c1], exit
    nop           // Branch Delay Slot

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS)

exit:
    nop.e
    nop

