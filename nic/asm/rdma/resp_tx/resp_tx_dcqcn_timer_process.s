#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct dcqcn_cb_t d;
struct resp_tx_s4_t0_k k;

// Note: Below values are constants related to g.
// TODO: Hardcoding it for now. Check if they have to be fed from HAL.
#define     G_MAX                   65536
#define     LOG_G_MAX               16
#define     ALPHA_TIMER_INTERVAL    55

%%
resp_tx_dcqcn_timer_process:

    // Pin dcqcn timer processing to stage 4
    mfspr       r1, spr_mpuid
    seq         c1, r1[4:2], STAGE_4
    bcf         [!c1], bubble_to_next_stage
    nop

    // Update alpha value.
    // int_alpha =  (((g_max - int_g) * int_alpha) >> log_g_max)
    sub         r1, G_MAX, d.g_val
    mul         r2, d.alpha_value, r1
    srl         r2, r2, LOG_G_MAX
    tblwr       d.alpha_value, r2
   
    // Check if timer T expired. 
    tblmincri   d.num_alpha_exp_cnt, 16, 1
    slt         c1, d.num_alpha_exp_cnt, d.timer_exp_thr
    bcf         [c1], restart_timer
    nop 
    
    // Timer T expired. Ring doorbell to run dcqcn algo. 
    tblmincri   d.timer_exp_cnt, 16, 1
    DOORBELL_INC_PINDEX(K_GLOBAL_LIF,  K_GLOBAL_QTYPE, K_GLOBAL_QID, DCQCN_RATE_COMPUTE_RING_ID, r5, r6)
    tblwr       d.num_alpha_exp_cnt, 0

restart_timer: 
    // Skip timer restart if max rate is reached.
    // This flag will help to avoid race-condition of timer-restart after max rate is reached, if timer-expiry and max-rate is hit simulataneously.
    bbeq        d.max_rate_reached, 1, skip_timer_restart
    nop
    // Restart alpha timer. Alpha timer runs for 55us by default.
    CAPRI_START_SLOW_TIMER(r1, r6, K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, DCQCN_TIMER_RING_ID, ALPHA_TIMER_INTERVAL)

skip_timer_restart:
    CAPRI_SET_TABLE_0_VALID(0)
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
