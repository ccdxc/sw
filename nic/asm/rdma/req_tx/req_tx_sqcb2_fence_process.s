#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s1_t0_k k;
struct sqcb2_t d;

#define TO_S2_P to_s5_sq_to_stage
#define IN_TO_S_P to_s1_sq_to_stage

#define K_WQE_ADDR CAPRI_KEY_FIELD(IN_TO_S_P, wqe_addr)
#define SQCB_TO_WQE_P t0_s2s_sqcb_to_wqe_info

%%
    .param req_tx_sqwqe_process

.align
req_tx_sqcb2_fence_process:

    /*
     *  Drop PHV if -
     *  1. fence is NOT set - fence update in add_headers[stage-5] is not visible here[stage-1] yet.
     *  2. fence_done is set - duplicate fence wqe PHV due to speculation reset in stage 0.
     */
    crestore    [c3,c2], d.{fence...fence_done}, 0x3 
    bcf         [!c3 | c2], exit

    // There are no outstanding read/atomic reqs if (rrq_pindex == rrq_cindex).    
    seq         c1, d.rrq_pindex, d.rrq_cindex //BD-slot
    bcf         [!c1], fence_exit
    nop //BD-slot

    // Set and send fence_done to wqe stage.
    tblwr       d.fence_done, 1
    phvwr       CAPRI_PHV_FIELD(SQCB_TO_WQE_P, fence_done), d.fence_done 
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqwqe_process, K_WQE_ADDR)
    nop.e
    nop

fence_exit:
    /*
     * DOL only code for model testing. 
     * Model enters in to a loop without below code. Break after 5 interations to simulate (rrq_pindex == rrq_cindex)
     */
    tblmincri   d.timestamp, 16, 1
    seq         c1, d.timestamp, 5
    bcf         [!c1], exit
    nop
    //Set and send fence_done to wqe stage.
    tblwr       d.fence_done, 1
    phvwr       CAPRI_PHV_FIELD(SQCB_TO_WQE_P, fence_done), d.fence_done // BD-slot
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqwqe_process, K_WQE_ADDR)
    nop.e
    nop

exit:
    phvwr       p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_VALID(0)
    nop.e
    nop
