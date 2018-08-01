#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s4_t1_k k;

%%

// assumption is that this program gets invoked thru table 1,
// only when ptseg eventually needs to be invoked by write/atomic.
// this program keeps bubbling down as mpu_only program 
// till stage_4 and then invokes ptseg_process for stage 5.

    .param  resp_rx_ptseg_process

.align
resp_rx_ptseg_mpu_only_process:

    mfspr            r1, spr_mpuid
    seq              c1, r1[4:2], STAGE_4
    bcf              [!c1], bubble_to_next_stage

    CAPRI_GET_TABLE_1_K(resp_rx_phv_t, r7) // BD Slot
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_PC_E(r7, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, resp_rx_ptseg_process)
    nop // Exit Slot


bubble_to_next_stage:
exit:
    nop.e
    nop


