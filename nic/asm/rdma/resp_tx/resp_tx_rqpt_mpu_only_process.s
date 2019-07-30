#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct resp_tx_s5_t2_k k;

%%

// assumption is that this program gets invoked thru table 2,
// only when resp_tx_rqpt_process eventually needs to be invoked.
// this program keeps bubbling down as mpu_only program 
// till stage_5 and then invokes resp_tx_rqpt_mpu_only_process for stage 6.

    .param  resp_tx_rqpt_process

.align
resp_tx_rqpt_mpu_only_process:

    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_5
    bcf              [!c1], bubble_to_next_stage

    CAPRI_GET_TABLE_2_K(resp_tx_phv_t, r7) // BD Slot

    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_PC_E(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_tx_rqpt_process)
    nop // Exit Slot

bubble_to_next_stage:
    nop.e
    nop
