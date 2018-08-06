#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s4_t2_k k;

#define RQCB1_ADDR  r7

%%

// assumption is that this program gets invoked thru table 2,
// only when rqcb1 write back eventually needs to be invoked.
// this program keeps bubbling down as mpu_only program 
// till stage_4 and then invokes rqcb1_write_back_process for stage 5.

    .param  resp_rx_rqcb1_write_back_process
    .param  resp_rx_inv_rkey_validate_process

.align
resp_rx_rqcb1_write_back_mpu_only_process:

    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_4
    bcf              [!c1], bubble_to_next_stage
    RQCB1_ADDR_GET(RQCB1_ADDR)      //BD Slot

    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, resp_rx_rqcb1_write_back_process, RQCB1_ADDR)

bubble_to_next_stage:
    seq              c1, r1[4:2], STAGE_3
    bcf              [!c1], exit
    nop // BD Slot

    // load inv_rkey for ACK/NAK generation
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_inv_rkey_validate_process, r0)

exit:
    nop.e
    nop


