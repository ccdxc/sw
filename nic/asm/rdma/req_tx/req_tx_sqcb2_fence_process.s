#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"

struct req_tx_phv_t p;
struct req_tx_s1_t0_k k;
struct sqcb2_t d;

#define IN_TO_S_P to_s1_fence_info

#define TO_S6_SQCB_WB_ADD_HDR_P to_s6_sqcb_wb_add_hdr_info

#define K_WQE_ADDR CAPRI_KEY_RANGE(IN_TO_S_P, wqe_addr_sbit0_ebit31, wqe_addr_sbit56_ebit63)
#define K_BKTRACK_FENCE_MARKER_PHV CAPRI_KEY_FIELD(IN_TO_S_P, bktrack_fence_marker_phv)
#define SQCB_TO_WQE_P t0_s2s_sqcb_to_wqe_info

%%
    .param req_tx_sqwqe_process
    .param req_tx_sqcb2_bktrack_marker_writeback_process
.align
req_tx_sqcb2_fence_process:

    /*
     *  Drop PHV if -
     *  1. Both fence and li_fence is NOT set - fence/li_fence update in add_headers[stage-5] is not visible here[stage-1] yet.
     *  2. fence_done is set - duplicate fence wqe PHV due to speculation reset in stage 0.
     */
    crestore    [c3,c2,c1], d.{fence, li_fence, fence_done}, 0x7
    bbeq        K_BKTRACK_FENCE_MARKER_PHV, 1, bktrack_fence_marker
    nop  // BD-slot
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
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqwqe_process, K_WQE_ADDR)

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
    CAPRI_SET_TABLE_0_VALID_CE(c0, 0)
    phvwr       p.common.p4_intr_global_drop, 1 // BD-slot

bktrack_fence_marker:
    // Set fence bit for write-back stage if fence_done is not set in sqcb2. This is an indication to writeback to set
    // dcqcn-rl-failure to reset spec-cindex and cindex eventually before triggering bktrack.
    // fence-done not being set here means there are no phvs in pipeline, which could make progress, and hence 
    // bktrack is safe to be triggered.
    phvwri.!c1       CAPRI_PHV_FIELD(TO_S6_SQCB_WB_ADD_HDR_P, fence), 1
    CAPRI_SET_TABLE_0_VALID(0)
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_sqcb2_bktrack_marker_writeback_process, r0)
