#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct req_rx_s4_t2_k k;
struct sqcb1_t d;

#define RRQWQE_TO_CQ_P t2_s2s_rrqwqe_to_cq_info
#define IN_P t2_s2s_sqcb1_write_back_info
#define IN_TO_S_P to_s4_sqcb1_wb_info
#define TO_S6_P to_s6_cq_info

#define K_CUR_SGE_ID CAPRI_KEY_FIELD(IN_P, cur_sge_id)
#define K_CUR_SGE_OFFSET CAPRI_KEY_RANGE(IN_P, cur_sge_offset_sbit0_ebit7, cur_sge_offset_sbit24_ebit31)
#define K_MSG_PSN CAPRI_KEY_RANGE(IN_P, msg_psn_sbit0_ebit15, msg_psn_sbit16_ebit23)
#define K_REXMIT_PSN CAPRI_KEY_RANGE(IN_P, rexmit_psn_sbit0_ebit2, rexmit_psn_sbit19_ebit23)
#define K_MSN CAPRI_KEY_RANGE(IN_P, msn_sbit0_ebit2, msn_sbit19_ebit23)

#define K_MY_TOKEN_ID CAPRI_KEY_RANGE(IN_TO_S_P, my_token_id_sbit0_ebit1, my_token_id_sbit2_ebit7)
#define K_REMAINING_PAYLOAD_BYTES CAPRI_KEY_RANGE(IN_TO_S_P, remaining_payload_bytes_sbit0_ebit7, remaining_payload_bytes_sbit8_ebit13)
#define K_POST_CQ CAPRI_KEY_FIELD(IN_P, post_cq)

%%
    .param req_rx_recirc_mpu_only_process
    .param req_rx_stats_process
    .param req_rx_cqcb_process

.align
req_rx_sqcb1_write_back_process:
    mfspr          r1, spr_mpuid
    seq            c1, r1[4:2], STAGE_4
    bcf            [!c1], bubble_to_next_stage

    // check if qp is already in error disable state. if so, drop the phv instead of recirc
    sle             c2, d.state, QP_STATE_ERR
    bcf             [c2], drop_response
    CAPRI_SET_TABLE_2_VALID(0)  //BD Slot

    seq            c1, K_MY_TOKEN_ID, d.nxt_to_go_token_id // Branch Delay Slot
    bcf            [!c1], recirc_for_turn

    seq            c1, d.bktrack_in_progress, 1 // BD-Slot
    bcf            [c1], drop_phv
    nop            // BD-Slot

    bbeq           CAPRI_KEY_FIELD(IN_TO_S_P, error_drop_phv), 1, drop_phv

    scwlt24        c2, K_REXMIT_PSN, d.rexmit_psn // Branch Delay Slot
    bcf            [c2], drop_phv

    seq            c1, CAPRI_KEY_FIELD(IN_P, incr_nxt_to_go_token_id), 1 // BD-Slot
    tbladd.c1      d.nxt_to_go_token_id, 1

    bbeq           CAPRI_KEY_FIELD(IN_TO_S_P, sge_opt), 1, update_qstate
    tblwr          d.rexmit_psn, K_REXMIT_PSN // BD-Slot

    tblwr          d.rrqwqe_cur_sge_id, K_CUR_SGE_ID
    tblwr          d.rrqwqe_cur_sge_offset, K_CUR_SGE_OFFSET

update_qstate:
    phvwr          CAPRI_PHV_FIELD(TO_S6_P, state), d.state

    CAPRI_NEXT_TABLE3_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_stats_process, r0)

    seq            c1, CAPRI_KEY_FIELD(IN_P, last_pkt), 1
    bcf            [!c1], post_cq
    SQCB2_ADDR_GET(r5) //BD-slot
    tblmincri      RRQ_C_INDEX, d.log_rrq_size, 1 

    /*
     * Update rrq_cindex in sqcb2. This will be used by fence-wqe to check for any outstanding
     * read/atomic requests. Ideally this should be updated after posting completion entry!
     * Since completion entry will always be posted after this point, doing it here doesn't hurt.
     */
    add            r6, FIELD_OFFSET(sqcb2_t, rrq_cindex), r5
    memwr.b        r6, RRQ_C_INDEX
post_cq:
    sne            c1, K_POST_CQ, 1
    scwle24.!c1    c1, K_MSN, d.msn // BD-Slot
    bcf            [c1], check_bktrack
    // Re-load err_retry, rnr_retry counter and set rnr_timeout to 0 if
    // outstanding request is cleared. Otherwise, keep decrementing the
    // err_retry_count upon retrans timer expiry or NAK (seq_err) or
    // implicit NAK, or rnr_retry_count upon rnr timer expiry
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_LSN_OR_REXMIT_PSN) // Branch Delay Slot
    DMA_HBM_PHV2MEM_PHV_END_SETUP(r6, rnr_timeout)

    //SQCB2_ADDR_GET(r1)
    //DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_RNR_TIMEOUT)
    //add            r2, r1, SQCB2_RNR_TIMEOUT_OFFSET
    //DMA_HBM_PHV2MEM_SETUP(r6, rnr_timeout, rnr_timeout, r2)

    // Hardcode table id 2 for CQCB process
    CAPRI_RESET_TABLE_2_ARG()

    phvwrpair      CAPRI_PHV_FIELD(RRQWQE_TO_CQ_P, cq_id), d.cq_id, \
                   CAPRI_PHV_FIELD(RRQWQE_TO_CQ_P, cqe_type), CQE_TYPE_SEND_MSN
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_cqcb_process, r0)
    tblwr          d.msn, K_MSN

    bbeq           K_GLOBAL_FLAG(_error_disable_qp), 1, error_disable_exit
    nop            // Branch Delay Slot

check_bktrack:
    bbne           CAPRI_KEY_FIELD(IN_P, post_bktrack), 1, check_sq_drain

post_bktrack_ring:
     // get DMA cmd entry based on dma_cmd_index
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_BKTRACK_DB) // Branch Delay Slot

    // dma_cmd - bktrack_ring db data
    PREPARE_DOORBELL_INC_PINDEX(K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, SQ_BKTRACK_RING_ID, r1, r2)
    phvwr          p.db_data2, r2.dx
    DMA_HBM_PHV2MEM_SETUP(r6, db_data2, db_data2, r1)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r6)
    // c1 set to TRUE if no completion AND no payload bytes to skip
    seq            c2, K_REMAINING_PAYLOAD_BYTES, r0
    setcf          c3, [c1 & c2]
    DMA_SET_END_OF_CMDS_C(DMA_CMD_PHV2MEM_T, r6, c3)

    // set cmd_eop if skip_to_eop cmd exists and cq is not posted
    setcf          c3, [c1 & !c2]
    DMA_CMD_STATIC_BASE_GET(r6, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_SKIP_TO_EOP)
    DMA_SET_END_OF_CMDS_C(DMA_CMD_PHV2MEM_T, r6, c3)

    tblwr          d.bktrack_in_progress, 1

check_sq_drain:
     // check for any unacknowledged requests if in drain state
     slt           c1, d.state, QP_STATE_SQD
     bcf           [c1], exit
     tblwr.c1      d.sq_drained, 0

     sub           r1, d.max_ssn, 1 // Branch Delay Slot
     mincr         r1, 24, r0
     seq           c2, d.msn, r1
     seq           c1, d.sq_drained, 1
     // Drain is complete upon receiving sq_drain feedback from TxDMA
     // AND all pending responses are acked
     bcf           [!c2 | !c1], exit

     // If QP is in QP_STATE_SQD_ON_ERR and all acks have been received then
     // move QP to QP_STATE_ERR and trigger TXDMA to send flush feedback to RQ
     seq           c1, d.state, QP_STATE_SQD_ON_ERR // Branch Delay Slot
     bcf           [c1], error_disable_exit
     nop           // Branch Delay Slot

     // Do not raise SQ drain async event if async_notify is not requested
     bbne          d.sqd_async_notify_enable, 1, exit
     // if QP_STATE_SQD, post async event and notify driver about drain completion
     phvwr         p.async_eqwqe.qid, K_GLOBAL_QID // Branch Delay Slot
     phvwrpair.e   p.async_eqwqe.code, EQE_CODE_QP_SQ_DRAIN, p.async_eqwqe.type, EQE_TYPE_QP
     phvwr         CAPRI_PHV_FIELD(TO_S6_P, async_event), 1

bubble_to_next_stage:
     seq           c1, r1[4:2], STAGE_3
     bcf           [!c1], exit
     SQCB1_ADDR_GET(r1)
     CAPRI_GET_TABLE_2_K(req_rx_phv_t, r7)
     CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)

exit:
     nop.e
     nop

drop_response:
    //skip to payload end
    DMA_CMD_STATIC_BASE_GET_E(r7, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_START)
    DMA_SKIP_CMD_SETUP(r7, 1 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/)

recirc_for_turn:
    seq            c1, d.state, QP_STATE_ERR
    bcf            [c1], drop_phv
    nop

    //This flag is different from global flags error_disable_qp.
    //This is used by stages after writeback(s5, s6, s7) to report intent to error_disable_qp using recirc
    bbeq            CAPRI_KEY_FIELD(IN_TO_S_P, error_disable_qp), 1, recirc_error_disable_qp
    nop

    phvwr          p.common.rdma_recirc_recirc_reason, CAPRI_RECIRC_REASON_INORDER_WORK_DONE

    // fire an mpu only program to set table 0 valid bit to 1 prior to recirc
    phvwr          p.common.p4_intr_recirc, 1
    CAPRI_NEXT_TABLE0_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_recirc_mpu_only_process, r0)

drop_phv:
    tbladd.e      d.nxt_to_go_token_id, 1
    phvwr         p.common.p4_intr_global_drop, 1

recirc_error_disable_qp:
error_disable_exit:
    // RXDMA encountered error on processing response msg. This is on
    // processing an in order response msg so set SQCBs state to QP_STATE_ERR.
    // Setting QP_STATE_ERR allows all phvs in pipeline for this QP to be dropped
    // and no additional CQEs to get posted after the error CQE. Once the state is
    // reflected in TXDMA SQCB, it will stop processing new wqes and will also post
    // flush feedback to error disable RQ.
    tblwr          d.state, QP_STATE_ERR

    phvwr          CAPRI_PHV_FIELD(TO_S6_P, state), QP_STATE_ERR
    phvwrpair      p.service, d.service, p.{flush_rq...state}, \
                   (1<<SQCB0_FLUSH_RQ_BIT_OFFSET | QP_STATE_ERR)
    DMA_CMD_STATIC_BASE_GET(r7, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_START)
    SQCB0_ADDR_GET(r1)
    add            r1, FIELD_OFFSET(sqcb0_t, service), r1
    DMA_HBM_PHV2MEM_SETUP(r7, service, state, r1)

    // doorbell to inc CNP ring's p_index so that TXDMA is triggered to send Flush
    // feedback for RQ. This is fenced on state update in sqcb0 such that when
    // doorbell evals cnp ring and schedules req_tx stage0 sqcb0's state is guaranteed
    // to be updated. Since inc_pindex is used, ring should have a size of 2^16,
    // hence one of the internal rings is used
    DMA_CMD_STATIC_BASE_GET(r7, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_RQ_FLUSH_DB)
    PREPARE_DOORBELL_INC_PINDEX(K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, CNP_RING_ID, r1, r2)
    phvwr          p.db_data1, r2.dx
    DMA_HBM_PHV2MEM_SETUP(r7, db_data1, db_data1, r1)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r7)

    
    // Skip to payload end if non-zero length
    seq            c1, K_REMAINING_PAYLOAD_BYTES, 0
    DMA_CMD_STATIC_BASE_GET_E(r7, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_SKIP_TO_EOP) // Branch Delay Slot
    DMA_SKIP_CMD_SETUP_C(r7, 0 /*CMD_EOP*/, 1 /*SKIP_TO_EOP*/, !c1)

