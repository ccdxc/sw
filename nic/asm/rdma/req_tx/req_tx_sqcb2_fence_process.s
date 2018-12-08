#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s1_t0_k k;
struct sqcb2_t d;

#define IN_TO_S_P to_s1_fence_info

#define K_WQE_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, wqe_addr_sbit0_ebit31, wqe_addr_sbit48_ebit63)
#define SQCB_TO_WQE_P t0_s2s_sqcb_to_wqe_info

%%
    .param req_tx_sqwqe_process

.align
req_tx_sqcb2_fence_process:

    /*
     *  Drop PHV if -
     *  1. Both fence and li_fence is NOT set - fence/li_fence update in add_headers[stage-5] is not visible here[stage-1] yet.
     *  2. fence_done is set - duplicate fence wqe PHV due to speculation reset in stage 0.
     */
    crestore    [c3,c2,c1], d.{fence, li_fence, fence_done}, 0x7
    bcf         ![c3 | c2], exit
    nop // BD-slot
    bcf         [c1], exit
    
    // Fence - There are no outstanding read/atomic reqs if (rrq_pindex == rrq_cindex).    
    seq.c3       c4, d.rrq_pindex, d.rrq_cindex //BD-slot

    // LI-Fence - There are no outstanding reqs if (tx_psn == rexmit_psn).    
    seq.c2       c4, d.tx_psn, d.rexmit_psn
    
    bcf         [!c4], fence_exit
    nop //BD-slot

    // Set and send fence_done to wqe stage.
    tblwr       d.fence_done, 1
    phvwr       CAPRI_PHV_FIELD(SQCB_TO_WQE_P, fence_done), d.fence_done 
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqwqe_process, K_WQE_ADDR)
    nop.e
    nop

fence_exit:

#if !(defined (HAPS) || defined (HW))

    /*
     * DOL only code for model testing. 
     * Model enters in to a loop without below code. Break after 5 interations to simulate fence-done.
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
#endif

exit:
    phvwr       p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_0_VALID(0)
    nop.e
    nop
