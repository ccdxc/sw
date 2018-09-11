#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;

%%

// RXDMA program stage 0 needs table 0 valid bit to be set so 
// that qstate loaded in stage 0 can be offset by 64bytes. So
// invoke dummy_program to check for stage 7 and set table0 valid bit
// Use table 2 for this dummy program as no other program uses
// this table in case of phv recirc

.align
req_rx_recirc_mpu_only_process:
    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_7
    bcf              [!c1], bubble_to_next_stage

    CAPRI_SET_TABLE_2_VALID_CE(c1, 0)   //BD Slot
    CAPRI_SET_TABLE_0_VALID(1)          //Exit Slot

bubble_to_next_stage:
    nop.e
    nop
