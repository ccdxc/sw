#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "common_defines.h"

struct resp_rx_phv_t p;
struct dcqcn_cb_t d;
struct resp_rx_ecn_process_k_t k;

// Note: Below values are constants related to g and alpha.
// TODO: Hardcoding it for now. Check if they have to be fed from HAL.
#define g_max                   65536
#define log_g_max               16
#define g                       256
#define alpha_max               65536
#define log_alpha_max           16
#define ALPHA_TIMER_INTERVAL    55

%%

.align
resp_rx_dcqcn_cnp_process:

    // Pin CNP handling to stage 4
    mfspr   r1, spr_mpuid
    seq     c1, r1[6:2], STAGE_4
    bcf     [!c1], bubble_to_next_stage
    nop

    // Reset all dcqcn params
    tblwr   d.byte_counter_exp_cnt, 0
    tblwr   d.cur_byte_counter, 0
    tblwr   d.timer_exp_cnt, 0

    // Set cur-rate to target-rate. Rt = Rc
    tblwr   d.target_rate, d.rate_enforced

    // cut rate-enforced. 
    // Rc = ((Rc * (alpha_max - (int_alpha >> 1))) >> log_alpha_max)
    srl     r3, d.alpha_value, 1
    sub     r4, alpha_max, r3
    mul     r3, r4, d.rate_enforced
    srl     r4, r3, log_alpha_max
    tblwr   d.rate_enforced, r4 

    // Update alpha value.                         
    // int_alpha =  (((g_max - int_g) * int_alpha) >> log_g_max) + int_g
    sub     r1, g_max, g                       
    mul     r2, d.alpha_value, r1              
    srl     r2, r2, log_g_max                  
    tblwr   d.alpha_value, r2                  

    // Restart alpha timer.
    // TODO: Check if there is special handling to restart timer??
    CAPRI_START_SLOW_TIMER(r1, r6, k.global.lif, k.global.qtype, k.global.qid, DCQCN_TIMER_RING_ID, ALPHA_TIMER_INTERVAL)
    CAPRI_SET_TABLE_2_VALID(0)
    nop.e
    nop

bubble_to_next_stage:
    seq     c1, r1[6:2], STAGE_3
    bcf     [!c1], exit
    nop     // Branch Delay Slot

    CAPRI_GET_TABLE_0_K(resp_rx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS)
    
exit:
    nop.e
    nop
