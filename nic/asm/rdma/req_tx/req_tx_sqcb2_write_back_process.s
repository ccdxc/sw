#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "defines.h"

struct req_tx_phv_t p;
struct req_tx_s5_t3_k k;
struct sqcb2_t d;

#define IN_P t3_s2s_sqcb_write_back_info_rd
#define IN_RD_P t3_s2s_sqcb_write_back_info_rd
#define IN_SEND_WR_P t3_s2s_sqcb_write_back_info_send_wr
#define IN_TO_S_P to_s5_sqcb_wb_info

#define K_SPEC_CINDEX CAPRI_KEY_FIELD(IN_TO_S_P, spec_cindex)

%%

.align
req_tx_sqcb2_write_back_process:
    // if speculative cindex matches cindex, then this wqe is being
    // processed in the right order and state update is allowed. Otherwise
    // discard and continue with speculation until speculative cindex
    // matches current cindex. Similarly, drop if dcqcn rate enforcement
    // doesn't allow this packet
    seq            c1, K_SPEC_CINDEX, d.sq_cindex
    bcf            [!c1], spec_fail
    CAPRI_SET_TABLE_3_VALID(0) // BD-slot

    bbeq           CAPRI_KEY_FIELD(IN_P, poll_failed), 1, poll_fail
    seq            c1, CAPRI_KEY_FIELD(IN_P, last_pkt), 1 // Branch Delay Slot

    bbeq           CAPRI_KEY_FIELD(IN_P, rate_enforce_failed), 1, rate_enforce_fail
    seq            c2, CAPRI_KEY_FIELD(IN_P, first), 1 // Branch Delay Slot

    bbeq           K_GLOBAL_FLAG(error_disable_qp), 1, error_exit
    nop  // BD-slot

    bbeq           CAPRI_KEY_FIELD(IN_P, set_li_fence), 1, li_fence
    nop

    bbeq          CAPRI_KEY_FIELD(IN_TO_S_P, fence), 1, fence
    nop // Branch Delay Slot

    // sqcb2 maintains copy of sq_cindex to enable speculation check. Increment
    //the copy on completion of wqe and write it into sqcb2
    tblmincri.c1    d.sq_cindex, d.log_sq_size, 1 

    tblwr          d.fence, 0
    tblwr          d.li_fence, 0
    tblwr          d.fence_done, 1

    // Send Feedback PHV to RxDMA to post completion.
    phvwrpair      p.rdma_feedback.feedback_type, RDMA_COMPLETION_FEEDBACK, \
                   p.rdma_feedback.completion.status, CQ_STATUS_SUCCESS
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_FEEDBACK)

generate_completion:
    add            r1, r0, offsetof(struct req_tx_phv_t, p4_to_p4plus)
    phvwrp         r1, 0, CAPRI_SIZEOF_RANGE(struct req_tx_phv_t, p4_intr_global, p4_to_p4plus), r0
    DMA_PHV2PKT_SETUP_MULTI_ADDR_0(r6, p4_intr_global, p4_to_p4plus, 2)
    DMA_PHV2PKT_SETUP_MULTI_ADDR_N(r6, rdma_feedback, rdma_feedback, 1)

    phvwrpair      p.p4_intr_global.tm_iport, TM_PORT_INGRESS, p.p4_intr_global.tm_oport, TM_PORT_DMA
    phvwrpair      p.p4_intr_global.tm_iq, 0, p.p4_intr_global.lif, K_GLOBAL_LIF
    SQCB0_ADDR_GET(r1)
    phvwrpair      p.p4_intr_rxdma.intr_qid, K_GLOBAL_QID, p.p4_intr_rxdma.intr_qstate_addr, r1
    phvwri         p.p4_intr_rxdma.intr_rx_splitter_offset, RDMA_FEEDBACK_SPLITTER_OFFSET

    phvwrpair      p.p4_intr_rxdma.intr_qtype, K_GLOBAL_QTYPE, p.p4_to_p4plus.p4plus_app_id, P4PLUS_APPTYPE_RDMA
    phvwri         p.p4_to_p4plus.raw_flags, REQ_RX_FLAG_RDMA_FEEDBACK
    phvwri         p.p4_to_p4plus.table0_valid, 1
    DMA_SET_END_OF_PKT(DMA_CMD_PHV2PKT_T, r6)
    DMA_SET_END_OF_CMDS(DMA_CMD_PHV2PKT_T, r6)

    nop.e
    nop

li_fence:
    tblwr           d.li_fence, 1
    tblwr           d.fence_done, 0
    phvwr           p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_3_VALID(0)
    nop.e
    nop

fence:
    tblwr           d.fence, 1
    tblwr           d.fence_done, 0
    //fall-through
poll_fail:
rate_enforce_fail:
spec_fail:
    phvwr   p.common.p4_intr_global_drop, 1
    //fall-through
exit:
    nop.e
    nop

error_exit:
    /*
     *  TODO: Incrementing sq_cindex copy to satisfy model. Ideally, on error disabling we should just exit and be
     *  in the same state which caused the error.
     */
    tblmincri.c1    d.sq_cindex, d.log_sq_size, 1

    phvwr          p.rdma_feedback.feedback_type, RDMA_COMPLETION_FEEDBACK
    b              generate_completion
    DMA_CMD_STATIC_BASE_GET(r6, REQ_TX_DMA_CMD_START_FLIT_ID, REQ_TX_DMA_CMD_RDMA_ERR_FEEDBACK)  // BD Slot


