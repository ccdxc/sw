#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s3_t1_k k;

%%

// assumption is that this program gets invoked thru table 1,
// only when rqlkey_process eventually needs to be invoked.
// this program keeps bubbling down as mpu_only program 
// till stage_3 and then invokes rqlkey_process for stage 4.

    .param  resp_rx_rqlkey_process
    .param  resp_rx_inv_rkey_process

.align
resp_rx_rqlkey_mpu_only_process:

    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_3
    bcf              [!c1], bubble_to_next_stage
    CAPRI_GET_TABLE_1_K(resp_rx_phv_t, r7) // BD Slot

    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_PC(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_256_BITS, resp_rx_rqlkey_process)
    // load inv_rkey as mpu only
    CAPRI_NEXT_TABLE3_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, resp_rx_inv_rkey_process, r0)

bubble_to_next_stage:
    nop.e
    nop

