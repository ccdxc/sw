#include "req_rx.h"
#include "sqcb.h"

struct req_rx_phv_t p;
struct req_rx_s4_t2_k k;
struct sqcb1_t d;

#define IN_P t2_s2s_sqcb1_to_sq_drain_feedback_info

#define K_SSN CAPRI_KEY_RANGE(IN_P, ssn_sbit0_ebit7, ssn_sbit8_ebit23)
#define TO_S6_P to_s6_cq_info

%%
    .param req_rx_cqcb_process

.align
req_rx_sq_drain_feedback_process:
    mfspr          r1, spr_mpuid
    seq            c1, r1[4:2], STAGE_4
    bcf            [!c1], bubble_to_next_stage

    // If state is anything other than QP_STATE_SQD or QP_STATE_SQD_ON_ERR
    // ignore this feedback phv
    slt            c1, d.state, QP_STATE_SQD // Branch Delay Slot
    bcf            [c1], exit
    CAPRI_SET_TABLE_2_VALID(0) // Branch Delay Slot

    // If sq_drain feedback was already received (sq_drained == 1), then
    // ignore this duplicate feedback phv. If all outstanding responses
    // are received and if state is QP_STATE_SQD, raise affiliated async
    // event for sq drain. If state is QP_STATE_SQD_ON_ERR just exit
    // as completions were already posted for successful ones when
    // corresponding responses were received and error completion was also
    // posted before moving state to QP_STATE_SQD_ON_ERR
    bbeq           d.sq_drained, 1, exit

    sub            r1, K_SSN, 1 // Branch Delay Slot
    mincr          r1, 24, r0
    seq            c2, d.msn, r1
    bcf            [!c2], exit
    tblwr          d.sq_drained, 1 // Branch Delay Slot

    seq           c1, d.state, QP_STATE_SQD_ON_ERR
    seq           c2, d.sqd_async_notify_enable, 1
    bcf           [c1 | !c2], exit

    // if QP_STATE_SQD, post async event and notify driver about drain completion
    phvwr       p.async_eqwqe.qid, K_GLOBAL_QID // Branch Delay Slot
    phvwrpair   p.async_eqwqe.code, EQE_CODE_QP_SQ_DRAIN, p.async_eqwqe.type, EQE_TYPE_QP
    phvwr       CAPRI_PHV_FIELD(TO_S6_P, async_event), 1
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_rx_cqcb_process, r0)

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    bcf           [!c1], exit
    SQCB1_ADDR_GET(r1)
    CAPRI_GET_TABLE_2_K(req_rx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, r1)

exit:
    phvwr.e       p.common.p4_intr_global_drop, 1
    nop
