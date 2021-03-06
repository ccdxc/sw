#include "req_tx.h"
#include "sqcb.h"
#include "defines.h"

struct req_tx_phv_t p;
struct req_tx_s1_t0_k k;

#define SQCB_TO_WQE_P t0_s2s_sqcb_to_wqe_info

#define IN_P t0_s2s_sqcb_to_pt_info

#define K_PAGE_OFFSET CAPRI_KEY_RANGE(IN_P, page_offset_sbit0_ebit7, page_offset_sbit8_ebit15)
#define K_REMAINING_PAYLOAD_BYTES CAPRI_KEY_RANGE(IN_P, remaining_payload_bytes_sbit0_ebit4, remaining_payload_bytes_sbit13_ebit15)
#define K_READ_REQ_ADJUST CAPRI_KEY_RANGE(IN_P, read_req_adjust_sbit0_ebit5, read_req_adjust_sbit30_ebit31)

#define TO_S5_SQCB_WB_ADD_HDR_P to_s5_sqcb_wb_add_hdr_info
#define TO_S7_STATS_INFO_P to_s7_stats_info
%%
    .param    req_tx_sqwqe_process
    .param    req_tx_dcqcn_enforce_process

.align
req_tx_sqpt_process:

    bcf            [c2 | c3 | c7], table_error
    // big-endian
    sub           r1, (HBM_NUM_PT_ENTRIES_PER_CACHE_LINE-1), CAPRI_KEY_FIELD(IN_P, page_seg_offset) // BD Slot
 
    // page_addr_p = (u64 *)(d_p + sizeof(u64) * sqcb_to_pt_info_p->page_seg_offset)
    sll            r1, r1, CAPRI_LOG_SIZEOF_U64_BITS
    // big-endian
    tblrdp.dx      r1, r1, 0, CAPRI_SIZEOF_U64_BITS
    or             r1, r1, 1, 63
    or             r1, r1, K_GLOBAL_LIF, 52

    //wqe_p = (void *)(*page_addr_p + sqcb_to_pt_info_p->page_offset)
    add            r1, r1, K_PAGE_OFFSET

    // populate t0 stage to stage data req_tx_sqcb_to_wqe_info_t for next stage
    CAPRI_RESET_TABLE_0_ARG()
    phvwrpair CAPRI_PHV_FIELD(SQCB_TO_WQE_P, log_pmtu), CAPRI_KEY_FIELD(IN_P, log_pmtu), \
              CAPRI_PHV_RANGE(SQCB_TO_WQE_P, poll_in_progress, color), \
              CAPRI_KEY_RANGE(IN_P, poll_in_progress, color) 
    phvwrpair CAPRI_PHV_FIELD(SQCB_TO_WQE_P, current_sge_offset), K_READ_REQ_ADJUST, \
              CAPRI_PHV_FIELD(SQCB_TO_WQE_P, remaining_payload_bytes), K_REMAINING_PAYLOAD_BYTES
    phvwr     CAPRI_PHV_FIELD(SQCB_TO_WQE_P, spec_enable), CAPRI_KEY_FIELD(IN_P, spec_enable)

    //CAPRI_SET_FIELD2(SQCB_TO_WQE_P, pd, CAPRI_KEY_FIELD(IN_P, pd))
    
    phvwr CAPRI_PHV_FIELD(TO_S5_SQCB_WB_ADD_HDR_P, wqe_addr), r1

    // populate t0 PC and table address
    CAPRI_NEXT_TABLE0_READ_PC(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, req_tx_sqwqe_process, r1)

    nop.e
    nop

table_error:
    phvwrpair      p.{rdma_feedback.completion.status, rdma_feedback.completion.error}, (CQ_STATUS_LOCAL_QP_OPER_ERR << 1 | 1), \
                   p.{rdma_feedback.completion.lif_cqe_error_id_vld, rdma_feedback.completion.lif_error_id_vld, rdma_feedback.completion.lif_error_id}, \
                       ((1 << 5) | (1 << 4) | LIF_STATS_RDMA_REQ_STAT(LIF_STATS_REQ_TX_LOCAL_OPER_ERR_OFFSET))

    // set err_dis_qp
    phvwr          CAPRI_PHV_FIELD(phv_global_common, _error_disable_qp), 1

    phvwr          CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_disabled), 1
    // update stats
    phvwr.c2       CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_dis_table_error), 1
    phvwr.c3       CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_dis_phv_intrinsic_error), 1
    phvwr.c7       CAPRI_PHV_FIELD(TO_S7_STATS_INFO_P, qp_err_dis_table_resp_error), 1
    phvwr          p.common.p4_intr_global_drop, 1

    CAPRI_SET_TABLE_0_VALID(0)
    // load dcqcn as mpu only
    CAPRI_NEXT_TABLE2_READ_PC_E(CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_0_BITS, req_tx_dcqcn_enforce_process, r0)
