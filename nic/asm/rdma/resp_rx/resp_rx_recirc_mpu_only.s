#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;

%%

// assumption is that this program gets invoked thru table 2,
// only when packet need to be recirculated. 
// To get +64 offset in S0 after recirc, we need to set 
// table_0_valid_bit to 1 prior to recirc.
// this program keeps bubbling down as mpu_only program 
// and in stage7 it resets table_2_valid_bit and sets
// table_0_valid_bit.

.align
resp_rx_recirc_mpu_only_process:

    // Pin recirc process to stage 7
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_7
    bcf              [!c1], bubble_to_next_stage

    CAPRI_SET_TABLE_2_VALID_CE(c1, 0)   //BD Slot
    CAPRI_SET_TABLE_0_VALID(1)          //Exit Slot

bubble_to_next_stage:
    nop.e
    nop


