#include "capri.h"
#include "req_tx.h"
#include "sqcb.h"
#include "defines.h"
#include "nic/p4/common/defines.h"

struct req_tx_phv_t p;
struct req_tx_s4_t0_k k;
struct key_entry_aligned_t d;


#define IN_P t0_s2s_sqwqe_to_lkey_inv_info
#define IN_TO_S_P to_s4_dcqcn_bind_mw_info

#define TO_S7_STATS_INFO_P to_s7_stats_info

#define K_SGE_INDEX CAPRI_KEY_FIELD(IN_P, sge_index)
#define WQE_TO_LKEY_T0 t0_s2s_sqwqe_to_lkey_inv_info
#define WQE_TO_LKEY_T1 t1_s2s_sqwqe_to_lkey_inv_info
#define SQCB_WRITE_BACK_P t2_s2s_sqcb_write_back_info
#define K_HEADER_TEMPLATE_ADDR CAPRI_KEY_FIELD(IN_TO_S_P, header_template_addr_or_pd)

%%

    .param  req_tx_dcqcn_enforce_process

.align
req_tx_sqlkey_invalidate_process:

    // Pin lkey_invalidate to stage 4
    mfspr         r1, spr_mpuid
    seq           c1, r1[4:2], STAGE_4
    bcf           [!c1], bubble_to_next_stage

    // Set table valid to 0.
    add           r2, K_SGE_INDEX, r0 // BD-slot
    CAPRI_SET_TABLE_I_VALID(r2, 0)

    // Skip invalidate if li_fence is set (first pass)
    seq           c2, CAPRI_KEY_FIELD(IN_P, set_li_fence), 1 
    bcf           [c2], exit
    nop

    // it is an error to invalidate an MR not eligible for invalidation
    bbeq         d.mr_flags.inv_en, 0, err_inv_not_allowed

    // it is an error to invalidate an MR in INVALID state
    seq          c1, d.state, KEY_STATE_INVALID // BD-slot 
    bcf          [c1], err_inv_state

    seq          c1, d.type, MR_TYPE_MW_TYPE_2 // BD-slot
    bcf          [!c1], invalidate

    // Type 2 MW
    seq          c2, d.state, KEY_STATE_VALID // BD-Slot
    bcf          [c2], inv_valid_mw

    // Type 2 MW in Free state can only be invalidated via QP associated to same PD as MW.
    seq          c3, CAPRI_KEY_FIELD(IN_TO_S_P, header_template_addr_or_pd), d.pd    // BD-slot
    bcf          [!c3], err_inv_pd
    nop     // BD-Slot

    nop.e   
    nop

inv_valid_mw:
    // Type 2 MW in valid state can only be invalidated via QP on which MW was bound
    seq          c4, K_GLOBAL_QID, d.qp
    bcf          [!c4], err_inv_qp
    nop //BD-slot
    //fall-through

invalidate:
    // Update state to FREE
    tblwr.e      d.state, KEY_STATE_FREE
    phvwrpair    CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, npg), 1, \
                 CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, npg_local_inv), 1 //Exit Slot

err_inv_not_allowed:
    b              error_completion
    phvwrpair      CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_dis_inv_lkey_inv_not_allowed), 1 //BD Slot

err_inv_state:
    b              error_completion
    phvwrpair      CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_dis_inv_lkey_invalid_state), 1 //BD Slot

err_inv_pd:
    b              error_completion
    phvwrpair      CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_dis_inv_lkey_pd_mismatch), 1 //BD Slot

err_inv_qp:
    b              error_completion
    phvwrpair      CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_disabled), 1, \
                   CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_dis_inv_lkey_qp_mismatch), 1 //BD Slot

error_completion:
    // Set completion status to Memory-Management-Operation-Error
    phvwrpair      p.rdma_feedback.feedback_type, RDMA_COMPLETION_FEEDBACK, \
                   p.{rdma_feedback.completion.status, rdma_feedback.completion.error}, (CQ_STATUS_MEM_MGMT_OPER_ERR << 1 | 1)
    phvwr          p.{rdma_feedback.completion.lif_cqe_error_id_vld, rdma_feedback.completion.lif_error_id_vld, rdma_feedback.completion.lif_error_id}, \
                       ((1 << 5) | (1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_TX_MEMORY_MGMT_ERR_OFFSET))

    // Set error-disable-qp. TODO: Using just as a place-holder. Full-blown error_disable_qp code will follow.
    phvwr.e        CAPRI_PHV_FIELD(phv_global_common, _error_disable_qp),  1
    nop

bubble_to_next_stage:
    seq           c1, r1[4:2], STAGE_3
    // Skip loading lkey-table if li_fence is set(first pass)
    seq           c2, CAPRI_KEY_FIELD(IN_P, set_li_fence), 1
    // Pass sge_index to S4
    phvwr.c1      CAPRI_PHV_FIELD(WQE_TO_LKEY_T0, sge_index), 0
    bcf           [!c1 | c2], load_dcqcn
    phvwr.c1      CAPRI_PHV_FIELD(WQE_TO_LKEY_T1, sge_index), 1 //BD-slot

    /*
     * Load sqlkey in tables 0 and 1 for invalidation. 
     * Its done in both tables coz next phv should be able to see invalidation in bypass-cache.
     */

    //invoke the same routine, but with valid lkey entry as d[] vector
    CAPRI_GET_TABLE_0_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, k.common_te0_phv_table_addr)

    CAPRI_GET_TABLE_1_K(req_tx_phv_t, r7)
    CAPRI_NEXT_TABLE_I_READ_SET_SIZE_TBL_ADDR(r7, CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, k.common_te0_phv_table_addr)

load_dcqcn:
    // Skip DCQCN stage if congestion-mgmt is not enabled.
    seq     c3, CAPRI_KEY_FIELD(IN_TO_S_P, congestion_mgmt_type), 0
    bcf     [c3], dcqcn_mpu_only
    phvwr   CAPRI_PHV_FIELD(SQCB_WRITE_BACK_P, non_packet_wqe), 1 // BD-slot
    add     r1, AH_ENTRY_T_SIZE_BYTES, K_HEADER_TEMPLATE_ADDR, HDR_TEMP_ADDR_SHIFT // Branch Delay Slot
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_EN, CAPRI_TABLE_SIZE_512_BITS, req_tx_dcqcn_enforce_process, r1)
    nop.e
    nop

dcqcn_mpu_only:
    CAPRI_NEXT_TABLE2_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r0)
    nop.e
    nop

exit:
    nop.e
    nop

