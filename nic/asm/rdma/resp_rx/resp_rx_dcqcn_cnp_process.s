#include "capri.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"
#include "common_defines.h"

struct resp_rx_phv_t p;
struct dcqcn_cb_t d;
struct resp_rx_ecn_process_k_t k;

%%

.align
resp_rx_dcqcn_cnp_process:

    // Pin CNP handling to stage 4
    mfspr   r1, spr_mpuid
    seq     c1, r1[6:2], STAGE_4
    bcf     [!c1], bubble_to_next_stage
    nop

    tblmincri   d.num_cnp_rcvd, 8, 1 //num_cnp_recvd is 8-bit field.
    // Ring doorbell to cut-rate and reset dcqcn params.
    DOORBELL_INC_PINDEX(k.global.lif,  k.global.qtype, k.global.qid, DCQCN_RATE_COMPUTE_RING_ID, r5, r6)

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
