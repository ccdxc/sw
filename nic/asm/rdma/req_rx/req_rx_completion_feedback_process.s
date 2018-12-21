#include "req_rx.h"
#include "sqcb.h"
#include "defines.h"

struct req_rx_phv_t p;
struct req_rx_s4_t2_k k;
struct sqcb1_t d;

#define RRQWQE_TO_CQ_P t2_s2s_rrqwqe_to_cq_info

#define IN_P t2_s2s_sqcb1_to_compl_feedback_info

#define K_STATUS CAPRI_KEY_FIELD(IN_P, status)
#define TO_S6_P to_s6_cq_info
#define TO_S7_P to_s7_stats_info

%%
    .param req_rx_cqcb_process

.align
req_rx_completion_feedback_process:
    mfspr          r1, spr_mpuid
    seq            c1, r1[4:2], STAGE_4
    bcf            [!c1], bubble_to_next_stage
    nop            // Branch Delay Slot

    // if SQ in RXDMA has already encountered error due to error response msg
    // or due to flush from RQ, drop this feedback msg. Error feedback msg from
    // TXDMA is for request later than the one for which RXMDA has errored
    // already and so drop this error feedback. If its flush feedback from RQ,
    // then it could be that SQ and RQ are encountering error simultaneously or
    // RQ is sending flush feedback multiple times.
    seq            c1, d.state, QP_STATE_ERR
    bcf            [c1], exit
    CAPRI_SET_TABLE_2_VALID(0) // Branch Delay Slot

    CAPRI_RESET_TABLE_2_ARG()
    phvwrpair      CAPRI_PHV_FIELD(RRQWQE_TO_CQ_P, cq_id), d.cq_id, \
                   CAPRI_PHV_FIELD(RRQWQE_TO_CQ_P, cqe_type), CQE_TYPE_SEND_NPG

    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_cqcb_process, r0)

    seq            c1, K_STATUS, CQ_STATUS_WQE_FLUSHED_ERR
    bcf            [c1], flush_completion

    seq             c1, d.service, RDMA_SERV_TYPE_UD // Branch Delay Slot
    bcf             [c1], ud_err_completion

rc_err_completion:
    // if TXDMA encounters error then there can be outstanding requests
    // for which acks are expected. So move SQ to QP_STATE_SQD_ON_ERR and drain
    // all outstanding requests. On receiving all the acks, move QP to
    // QP_STATE_ERR, post doorbell to TXDMA to send flush feedback to RQ
    sub            r1, d.max_ssn, 1
    mincr          r1, 24, r0
    seq            c1, d.msn, r1
    cmov           r1, c1, QP_STATE_ERR, QP_STATE_SQD_ON_ERR
    tblwr          d.state, r1
    phvwr          CAPRI_PHV_FIELD(TO_S6_P, state), r1
    add            r1, r1, 1, SQCB0_FLUSH_RQ_BIT_OFFSET
    phvwrpair      p.service, d.service, p.{flush_rq...state}, r1
    DMA_CMD_STATIC_BASE_GET(r7, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_START)
    SQCB0_ADDR_GET(r1)
    add            r1, FIELD_OFFSET(sqcb0_t, state), r1
    DMA_HBM_PHV2MEM_SETUP(r7, service, state, r1)

    // If state is SQD_ON_ERR do not ring CNP ring id until all pending responses
    // are acked and QP is moved to ERR state
    bcf            [!c1], exit

    // doorbell to inc CNP ring's p_index so that TXDMA is triggered to send Flush
    // feedback for RQ. This is fenced on state update in sqcb0 such that when
    // doorbell evals CNP ring and schedules req_tx stage0 sqcb0's state is guaranteed
    // to be updated. Since inc_pindex is used, ring should have a size of 2^16,
    // hence one of the internal rings is used
    DMA_CMD_STATIC_BASE_GET(r7, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_RQ_FLUSH_DB) // Branch Delay Slot
    PREPARE_DOORBELL_INC_PINDEX(K_GLOBAL_LIF, K_GLOBAL_QTYPE, K_GLOBAL_QID, CNP_RING_ID, r1, r2)
    phvwr          p.db_data1, r2.dx // Branch Delay Slot
    DMA_HBM_PHV2MEM_SETUP(r7, db_data1, db_data1, r1)
    DMA_SET_WR_FENCE(DMA_CMD_PHV2MEM_T, r7)

    nop.e
    nop

flush_completion:
    // on receiving flush feedback msg from RQ, post flush cqe to mark
    // end of SQ completion and DMA to update state in TXDMA's SQCB

    // TODO Do we need msn in the flush completion? Can driver figure out
    // by itself?
    //add            r2, d.msn, 1
    //mincr          r2, 24, r0
    //phvwr          p.cqe.send.wrid, r2

    DMA_CMD_STATIC_BASE_GET(r7, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_START)
    SQCB0_ADDR_GET(r1)
    add            r1, FIELD_OFFSET(sqcb0_t, state), r1
    DMA_HBM_PHV2MEM_SETUP(r7, service, state, r1)
    phvwr          CAPRI_PHV_FIELD(TO_S6_P, state), QP_STATE_ERR
    phvwr          CAPRI_PHV_RANGE(TO_S7_P, lif_error_id_vld, lif_error_id), \
                    ((1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_RX_CQE_FLUSH_ERR_OFFSET))

    tblwr.e        d.state, QP_STATE_ERR
    phvwrpair      p.service, d.service, p.{flush_rq...state}, QP_STATE_ERR

ud_err_completion:
    // On error move UD Qstate to SQ_ERR and post error completion. Driver will
    // flush all the outstanding wqes. No need to send flush feedback to RQ, so
    // no logic to trigger TXDMA. DMA to change TXDMA Qstate's state to
    //SQ_ERR
    DMA_CMD_STATIC_BASE_GET(r7, REQ_RX_DMA_CMD_START_FLIT_ID, REQ_RX_DMA_CMD_START)
    SQCB0_ADDR_GET(r1)
    add            r1, FIELD_OFFSET(sqcb0_t, state), r1
    DMA_HBM_PHV2MEM_SETUP(r7, service, state, r1)
    phvwrpair.e    p.service, d.service, p.state, QP_STATE_SQ_ERR
    tblwr          d.state, QP_STATE_SQ_ERR

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    bcf           [!c1], exit
    SQCB1_ADDR_GET(r1)
    CAPRI_GET_TABLE_2_K(req_rx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)

exit:
    nop.e
    nop
