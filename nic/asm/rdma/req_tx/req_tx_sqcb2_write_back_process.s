#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct req_tx_s5_t2_k k;
struct sqcb2_t d;

#define IN_P t2_s2s_sqcb_write_back_info_rd
#define IN_RD_P t2_s2s_sqcb_write_back_info_rd
#define IN_SEND_WR_P t2_s2s_sqcb_write_back_info_send_wr
#define IN_TO_S_P to_s5_sqcb_wb_add_hdr_info

#define K_SPEC_CINDEX CAPRI_KEY_RANGE(IN_TO_S_P, spec_cindex_sbit0_ebit7, spec_cindex_sbit8_ebit15)
#define K_TO_S5_DATA k.{to_stage_5_to_stage_data_sbit0_ebit63...to_stage_5_to_stage_data_sbit112_ebit127}
#define K_WQE_ADDR CAPRI_KEY_FIELD(IN_TO_S_P, wqe_addr)

#define TO_S6_P to_s6_sqcb_wb_add_hdr_info

%%

    .param  req_tx_stats_process
    .param  req_tx_write_back_process

.align
req_tx_sqcb2_write_back_process:

    //invoke stats_process for NPG case here
    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_stats_process, r0)

    // if speculative cindex matches cindex, then this wqe is being
    // processed in the right order and state update is allowed. Otherwise
    // discard and continue with speculation until speculative cindex
    // matches current cindex. Similarly, drop if dcqcn rate enforcement
    // doesn't allow this packet
    seq            c1, K_SPEC_CINDEX, d.sq_cindex
    bcf            [!c1], spec_fail
    nop

    phvwr          p.common.to_stage_6_to_stage_data, K_TO_S5_DATA
    SQCB0_ADDR_GET(r1)
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_write_back_process, r1)

    bbeq           CAPRI_KEY_FIELD(IN_P, poll_failed), 1, poll_fail
    seq            c1, CAPRI_KEY_FIELD(IN_P, last_pkt), 1 // Branch Delay Slot

    bbeq           CAPRI_KEY_FIELD(IN_P, rate_enforce_failed), 1, rate_enforce_fail
    seq            c2, CAPRI_KEY_FIELD(IN_P, first), 1 // Branch Delay Slot

    bbeq           K_GLOBAL_FLAG(_error_disable_qp), 1, error_exit
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

    bbeq           K_GLOBAL_FLAG(_rexmit), 1, end

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
    DMA_SET_END_OF_PKT_END_OF_CMDS_E(DMA_CMD_PHV2PKT_T, r6)
    phvwri         p.p4_to_p4plus.table0_valid, 1 //BD-slot

li_fence:
    tblwr           d.li_fence, 1
    tblwr           d.fence_done, 0
    phvwr           p.common.p4_intr_global_drop, 1
    CAPRI_SET_TABLE_3_VALID(0)
    nop.e
    nop

fence:
    tblwr.e         d.fence, 1
    tblwr           d.fence_done, 0

spec_fail:
    phvwr   p.common.p4_intr_global_drop, 1
    //fall-through
exit:
    CAPRI_SET_TABLE_2_VALID(0)
rate_enforce_fail:
poll_fail:
end:
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


