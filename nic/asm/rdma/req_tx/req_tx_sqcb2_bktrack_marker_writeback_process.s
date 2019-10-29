#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct req_tx_s5_t2_k k;
struct sqcb2_t d;

%%

    .param  req_tx_bktrack_marker_writeback_process

.align
req_tx_sqcb2_bktrack_marker_writeback_process:

    // Pin to stage 5
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_5
    bcf           [!c1], bubble_to_next_stage
    nop

    // Clear fence bits and set fence_done
    tblwr           d.li_fence, 0
    tblwr           d.fence, 0
    tblwr           d.fence_done, 1 

    SQCB0_ADDR_GET(r1) 
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_bktrack_marker_writeback_process, r1)

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], exit
    SQCB2_ADDR_GET(r2) // BD-slot

    //invoke the same routine, but with valid sqcb2 as d[] vector
    CAPRI_GET_TABLE_2_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR_E(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r2)

exit:
    nop.e
    nop
