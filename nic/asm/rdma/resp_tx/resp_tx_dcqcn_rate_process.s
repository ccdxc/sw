#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct dcqcn_cb_t d;
struct resp_tx_s4_t0_k k;

#define  RATE_ENFORCED  r3
#define  TARGET_RATE    r4
#define  F              r5
#define  Ri             r6

#define IN_P t0_s2s_cfg_to_dcqcn_info

#define K_G_VAL	CAPRI_KEY_FIELD(IN_P, g_val)
#define K_ALPHA_TIMER_INTERVAL CAPRI_KEY_RANGE(IN_P, alpha_timer_interval_sbit0_ebit7, alpha_timer_interval_sbit24_ebit31)
#define K_Ai CAPRI_KEY_RANGE(IN_P, ai_rate_sbit0_ebit7, ai_rate_sbit16_ebit17)
#define K_HAi CAPRI_KEY_RANGE(IN_P, hai_rate_sbit0_ebit5, hai_rate_sbit14_ebit17)
#define K_THRESHOLD CAPRI_KEY_RANGE(IN_P, threshold_sbit0_ebit3, threshold_sbit4_ebit4)
#define K_BYTE_RESET CAPRI_KEY_RANGE(IN_P, byte_reset_sbit0_ebit6, byte_reset_sbit31_ebit31)
#define K_CLAMP_TGT_RATE CAPRI_KEY_FIELD(IN_P, clamp_tgt_rate)

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

#define IN_TO_S4_P to_s4_dcqcn_rate_timer_info
#define K_NEW_TIMER_CINDEX CAPRI_KEY_FIELD(IN_TO_S4_P, new_timer_cindex)

#define K_MIN_QP_RATE CAPRI_KEY_FIELD(IN_TO_S4_P, min_qp_rate)
#define K_MIN_QP_TARGET_RATE CAPRI_KEY_RANGE(IN_TO_S4_P, min_qp_target_rate_sbit0_ebit7, min_qp_target_rate_sbit24_ebit31)

#define TO_S_STATS_INFO_P to_s7_stats_info

%%
    .param  resp_tx_stats_process

resp_tx_dcqcn_rate_process:

    // Pin dcqcn_algo to stage 4
    mfspr   r1, spr_mpuid
    seq     c1, r1[4:2], STAGE_4
    bcf     [!c1], bubble_to_next_stage
    nop

    // invoke stats as mpu only
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_tx_stats_process, r0)

    // Check if doorbell is for processing CNP packet.
    seq     c2, d.num_cnp_rcvd, d.num_cnp_processed 
    bcf     [!c2], cnp_recv_process
    phvwr   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, dcqcn_rate), 1   // BD Slot

    bbeq    d.max_rate_reached, 1, exit
    // Update dcqcn_cb values from config
    tblwr   d.byte_counter_thr, K_BYTE_RESET    // BD Slot

    add     F, r0, K_THRESHOLD
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
    // Update DCQCN debug counters
    phvwr   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, rp_num_additive_increase), 1 // BD Slot

    // Rt = Rt + Ai. 
    add     TARGET_RATE, d.target_rate, K_Ai

    // Rc = ((Rt + Rc) / 2)
    add     r1, d.rate_enforced, TARGET_RATE
    srl     RATE_ENFORCED, r1, 1

    tblwr   d.rate_enforced, RATE_ENFORCED

    // Check enforced rate has reached MAX-QP-RATE. If yes, shutdown DCQCN timers to stop further rate-increase.
    slt     c1, RATE_ENFORCED, QP_MAX_RATE
    bcf     [c1], exit
    tblwr   d.target_rate, TARGET_RATE  // BD Slot

    tblwr.e d.max_rate_reached, 1
    phvwr   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, rp_num_max_rate_reached), 1  // Exit Slot	

fast_recovery:
    // Update DCQCN debug counters
    phvwr   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, rp_num_fast_recovery), 1

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
    // Update DCQCN debug counters
    phvwr   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, rp_num_hyper_increase), 1    // BD Slot

    // Rt = Rt + HAi. 
    add     TARGET_RATE, d.target_rate, K_HAi

skip_target_rate_inc:
    // Rc = ((Rt + Rc) / 2)
    add     r1, d.rate_enforced, TARGET_RATE
    srl     RATE_ENFORCED, r1, 1

    tblwr   d.rate_enforced, RATE_ENFORCED

    // Check enforced rate has reached MAX-QP-RATE. If yes, shutdown DCQCN timers to stop further rate-increase.
    slt     c1, RATE_ENFORCED, QP_MAX_RATE
    bcf     [c1], exit
    tblwr   d.target_rate, TARGET_RATE  // BD Slot

    tblwr.e d.max_rate_reached, 1
    phvwr   CAPRI_PHV_FIELD(TO_S_STATS_INFO_P, rp_num_max_rate_reached), 1  // Exit Slot

cnp_recv_process:
    // Reset all dcqcn params
    tblwr   d.byte_counter_exp_cnt, 0
    tblwr   d.cur_byte_counter, 0
    tblwr   d.timer_exp_cnt, 0

    // Set cur-rate to target-rate only if clamp is not set. Rt = Rc
    sne         c2, K_CLAMP_TGT_RATE, 1
    tblwr.c2    d.target_rate, d.rate_enforced

    // cut rate-enforced. 
    // Rc = ((Rc * (alpha_max - (int_alpha >> 1))) >> log_alpha_max)
    srl     r3, d.alpha_value, 1
    sub     r4, ALPHA_MAX, r3
    mul     r3, r4, d.rate_enforced
    srl     r4, r3, LOG_ALPHA_MAX

    // If rate_enforced goes below min_rate after cut, set it to the configured min rate with target rate atleast twice min_rate
    slt         c1, r4, K_MIN_QP_RATE
    add.c1      r4, r0, K_MIN_QP_RATE
    add.c1      r5, r0, K_MIN_QP_TARGET_RATE
    tblwr       d.rate_enforced, r4
    setcf       c3, [c1 & c2]
    tblwr.c3    d.target_rate, r5

    // Update alpha value.                         
    // int_alpha =  (((g_max - int_g) * int_alpha) >> log_g_max) + int_g
    sub     r1, G_MAX, K_G_VAL
    mul     r2, d.alpha_value, r1
    srl     r2, r2, LOG_G_MAX
    add     r2, r2, K_G_VAL
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
    CAPRI_START_FAST_TIMER(r1, r6, K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, DCQCN_TIMER_RING_ID, K_ALPHA_TIMER_INTERVAL)
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

