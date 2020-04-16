/*
* Copyright (c) 2020, Pensando Systems Inc.
*/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "nic/sdk/lib/pal/pal.hpp"
#include "platform/src/lib/nicmgr/include/rdma_dev.hpp"

#include "impl.hpp"


#define RESET_HEADER() do { hdr_done = false; } while (0)
#define PRINT_HEADER(owner)                                             \
    do {                                                                \
        if (!hdr_done) {                                                \
            printf("\t" #owner "\n");                                   \
            hdr_done = true;                                            \
        }                                                               \
    } while (0);

#define PRINT_FLD16(owner, field, fmt)                                  \
    do {                                                                \
        if (owner->field) {                                             \
            PRINT_HEADER(owner);                                        \
            printf("\t\t%16s " fmt "\n", #field, owner->field);         \
        }                                                               \
    } while (0)

#define PRINT_FLD24(owner, field, fmt)                                  \
    do {                                                                \
        if (owner->field) {                                             \
            PRINT_HEADER(owner);                                        \
            printf("\t%24s " fmt "\n", #field, owner->field);           \
        }                                                               \
    } while (0)

#define PRINT_FLD32(owner, field, fmt)                                  \
    do {                                                                \
        if (owner->field) {                                             \
            PRINT_HEADER(owner);                                        \
            printf("%32s " fmt "\n", #field, owner->field);             \
        }                                                               \
    } while (0)

static void
rdma_qstate_one(queue_info_t *qinfo, uint8_t qtype, uint32_t qid)
{
    union {
        aqcb_t aq;
        sqcb_t sq;
        rqcb_t rq;
        cqcb0_t cq;
        eqcb0_t eq;
    } qstate;
    uint64_t addr = qinfo[qtype].base + qid * qinfo[qtype].size;

    aqcb0_t *aqcb0;
    aqcb1_t *aqcb1;
    sqcb0_t *sqcb0;
    sqcb1_t *sqcb1;
    sqcb2_t *sqcb2;
    // There is nothing in sqcb3 at present
    sqcb4_t *sqcb4;
    sqcb5_t *sqcb5;
    rqcb0_t *rqcb0;
    rqcb1_t *rqcb1;
    rqcb2_t *rqcb2;
    rqcb3_t *rqcb3;
    rqcb4_t *rqcb4;
    rqcb5_t *rqcb5;
    cqcb0_t *cqcb0;
    eqcb0_t *eqcb0;
    bool hdr_done;

    /*
     * SQ, RQ, and CQ must have total_rings set to be printed.
     * AQ and EQ must have log_num_wqes set to be printed.
     * Queue ID 0 may only be printed in the EQ case.
     */
    if (!qid && qtype != 6 /* EQ */)
        return;

    switch (qtype) {
    case 2: // AQ
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate.aq, sizeof(qstate.aq));

        aqcb0 = (aqcb0_t *)memrev(&qstate.aq.aqcb0, sizeof *aqcb0);
        if (!aqcb0->log_num_wqes)
            break;

        printf("\naq %u addr %lx size %lx\n",
               qid, addr, sizeof(qstate.aq));

        RESET_HEADER();
        PRINT_FLD24(aqcb0, map_count_completed, "%0x");
        PRINT_FLD16(aqcb0, first_pass, "%0x");
        PRINT_FLD16(aqcb0, token_id, "%0x");
        PRINT_FLD16(aqcb0, rsvd1[0], "%0x");
        PRINT_FLD16(aqcb0, rsvd1[1], "%0x");
        PRINT_FLD16(aqcb0, uplink_num, "%0x");
        PRINT_FLD16(aqcb0, debug, "%0x");
        PRINT_FLD16(aqcb0, log_wqe_size, "%0x");
        PRINT_FLD16(aqcb0, log_num_wqes, "%0x");
        PRINT_FLD32(aqcb0, ring_empty_sched_eval_done, "%0x");
        PRINT_FLD16(aqcb0, rsvd2, "%0x");
        PRINT_FLD16(aqcb0, phy_base_addr, "%0lx");
        PRINT_FLD16(aqcb0, next_token_id, "%0x");
        PRINT_FLD16(aqcb0, aq_id, "%0x");
        PRINT_FLD16(aqcb0, cq_id, "%0x");
        PRINT_FLD16(aqcb0, error, "%0x");
        PRINT_FLD16(aqcb0, cqcb_addr, "%0lx");

        aqcb1 = (aqcb1_t *)memrev(&qstate.aq.aqcb1, sizeof *aqcb1);
        RESET_HEADER();
        PRINT_FLD16(aqcb1, num_nop, "%0x");
        PRINT_FLD16(aqcb1, num_create_cq, "%0x");
        PRINT_FLD16(aqcb1, num_create_qp, "%0x");
        PRINT_FLD16(aqcb1, num_reg_mr, "%0x");
        PRINT_FLD16(aqcb1, num_stats_hdrs, "%0x");
        PRINT_FLD16(aqcb1, num_stats_vals, "%0x");
        PRINT_FLD16(aqcb1, num_dereg_mr, "%0x");
        PRINT_FLD16(aqcb1, num_resize_cq, "%0x");
        PRINT_FLD16(aqcb1, num_destroy_cq, "%0x");
        PRINT_FLD16(aqcb1, num_modify_qp, "%0x");
        PRINT_FLD16(aqcb1, num_query_qp, "%0x");
        PRINT_FLD16(aqcb1, num_destroy_qp, "%0x");
        PRINT_FLD16(aqcb1, num_stats_dump, "%0x");
        PRINT_FLD16(aqcb1, num_create_ah, "%0x");
        PRINT_FLD16(aqcb1, num_query_ah, "%0x");
        PRINT_FLD16(aqcb1, num_destroy_ah, "%0x");
        PRINT_FLD16(aqcb1, num_any, "%0lx");
        break;

    case 3: // SQ
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate.sq, sizeof(qstate.sq));

        sqcb0 = (sqcb0_t *)memrev(&qstate.sq.sqcb0, sizeof *sqcb0);
        if (!sqcb0->total_rings)
            break;

        printf("\nsq %u addr %lx size %lx\n",
               qid, addr, sizeof(qstate.sq));

        RESET_HEADER();
        PRINT_FLD16(sqcb0, pc, "%0x");
        PRINT_FLD16(sqcb0, rsvd, "%0x");
        PRINT_FLD16(sqcb0, cosA, "%0x");
        PRINT_FLD16(sqcb0, cosB, "%0x");
        PRINT_FLD16(sqcb0, cos_selector, "%0x");
        PRINT_FLD16(sqcb0, eval_last, "%0x");
        PRINT_FLD16(sqcb0, host_rings, "%0x");
        PRINT_FLD16(sqcb0, total_rings, "%0x");
        PRINT_FLD16(sqcb0, pid, "%0x");
        PRINT_FLD16(sqcb0, p_index0, "%0x");
        PRINT_FLD16(sqcb0, c_index0, "%0x");
        PRINT_FLD16(sqcb0, p_index1, "%0x");
        PRINT_FLD16(sqcb0, c_index1, "%0x");
        PRINT_FLD16(sqcb0, p_index2, "%0x");
        PRINT_FLD16(sqcb0, c_index2, "%0x");
        PRINT_FLD16(sqcb0, p_index3, "%0x");
        PRINT_FLD16(sqcb0, c_index3, "%0x");
        PRINT_FLD16(sqcb0, sqd_cindex, "%0lx");
        PRINT_FLD16(sqcb0, rsvd1, "%0lx");
        PRINT_FLD16(sqcb0, base_addr, "%0lx");
        PRINT_FLD24(sqcb0, header_template_addr, "%0x");
        PRINT_FLD16(sqcb0, pd, "%0x");
        PRINT_FLD16(sqcb0, poll_in_progress, "%0x");
        PRINT_FLD16(sqcb0, log_pmtu, "%0x");
        PRINT_FLD16(sqcb0, log_sq_page_size, "%0x");
        PRINT_FLD16(sqcb0, log_wqe_size, "%0x");
        PRINT_FLD16(sqcb0, log_num_wqes, "%0x");
        PRINT_FLD16(sqcb0, poll_for_work, "%0x");
        PRINT_FLD24(sqcb0, signalled_completion, "%0x");
        PRINT_FLD16(sqcb0, dcqcn_rl_failure, "%0x");
        PRINT_FLD16(sqcb0, service, "%0x");
        PRINT_FLD16(sqcb0, flush_rq, "%0x");
        PRINT_FLD16(sqcb0, state, "%0x");
        PRINT_FLD16(sqcb0, sq_in_hbm, "%0x");
        PRINT_FLD16(sqcb0, rsvd5, "%0x");
        PRINT_FLD24(sqcb0, local_ack_timeout, "%0x");
        PRINT_FLD32(sqcb0, ring_empty_sched_eval_done, "%0x");
        PRINT_FLD16(sqcb0, spec_sq_cindex, "%0x");
        PRINT_FLD16(sqcb0, curr_wqe_ptr, "%0lx");
        PRINT_FLD16(sqcb0, spec_msg_psn, "%0x");
        PRINT_FLD16(sqcb0, msg_psn, "%0x");
        PRINT_FLD16(sqcb0, sq_drained, "%0x");
        PRINT_FLD16(sqcb0, rsvd_state_flags, "%0x");
        PRINT_FLD16(sqcb0, state_flags, "%0x");
        PRINT_FLD16(sqcb0, spec_enable, "%0x");
        PRINT_FLD16(sqcb0, skip_pt, "%0x");
        PRINT_FLD32(sqcb0, bktrack_marker_in_progress, "%0x");
        PRINT_FLD24(sqcb0, congestion_mgmt_type, "%0x");
        PRINT_FLD32(sqcb0, frpmr_in_progress, "%0x");
        PRINT_FLD16(sqcb0, rsvd2, "%0x\n");

        sqcb1 = (sqcb1_t *)memrev(&qstate.sq.sqcb1, sizeof *sqcb1);
        RESET_HEADER();
        PRINT_FLD16(sqcb1, pc, "%0x");
        PRINT_FLD16(sqcb1, cq_id, "%0x");
        PRINT_FLD16(sqcb1, rrq_pindex, "%0x");
        PRINT_FLD16(sqcb1, rrq_cindex, "%0x");
        PRINT_FLD16(sqcb1, rrq_base_addr, "%0x");
        PRINT_FLD16(sqcb1, log_rrq_size, "%0x");
        PRINT_FLD16(sqcb1, service, "%0x");
        PRINT_FLD16(sqcb1, rsvd8, "%0x");
        PRINT_FLD16(sqcb1, log_pmtu, "%0x");
        PRINT_FLD16(sqcb1, err_retry_count, "%0x");
        PRINT_FLD16(sqcb1, rnr_retry_count, "%0x");
        PRINT_FLD32(sqcb1, work_not_done_recirc_cnt, "%0x");
        PRINT_FLD16(sqcb1, tx_psn, "%0x");
        PRINT_FLD16(sqcb1, ssn, "%0x");
        PRINT_FLD16(sqcb1, rsvd2, "%0x");
        PRINT_FLD16(sqcb1, log_sqwqe_size, "%0x");
        PRINT_FLD16(sqcb1, pkt_spec_enable, "%0x");
        PRINT_FLD16(sqcb1, rsvd3, "%0x");
        PRINT_FLD24(sqcb1, header_template_addr, "%0x");
        PRINT_FLD24(sqcb1, header_template_size, "%0x");
        PRINT_FLD16(sqcb1, nxt_to_go_token_id, "%0x");
        PRINT_FLD16(sqcb1, token_id, "%0x");
        PRINT_FLD16(sqcb1, rsvd4, "%0x");
        PRINT_FLD16(sqcb1, rexmit_psn, "%0x");
        PRINT_FLD16(sqcb1, msn, "%0x");
        PRINT_FLD16(sqcb1, credits, "%0x");
        PRINT_FLD24(sqcb1, congestion_mgmt_type, "%0x");
        PRINT_FLD16(sqcb1, rsvd5, "%0x");
        PRINT_FLD16(sqcb1, max_tx_psn, "%0x");
        PRINT_FLD16(sqcb1, max_ssn, "%0x");
        PRINT_FLD16(sqcb1, rrqwqe_num_sges, "%0x");
        PRINT_FLD16(sqcb1, rrqwqe_cur_sge_id, "%0x");
        PRINT_FLD24(sqcb1, rrqwqe_cur_sge_offset, "%0x");
        PRINT_FLD16(sqcb1, state, "%0x");
        PRINT_FLD24(sqcb1, sqcb1_priv_oper_enable, "%0x");
        PRINT_FLD16(sqcb1, sq_drained, "%0x");
        PRINT_FLD24(sqcb1, sqd_async_notify_enable, "%0x");
        PRINT_FLD16(sqcb1, rsvd6, "%0x");
        PRINT_FLD24(sqcb1, bktrack_in_progress, "%0x");
        PRINT_FLD16(sqcb1, pd, "%0x");
        PRINT_FLD16(sqcb1, rrq_spec_cindex, "%0x");
        PRINT_FLD16(sqcb1, rsvd7, "%0x\n");

        sqcb2 = (sqcb2_t *)memrev(&qstate.sq.sqcb2, sizeof *sqcb2);
        RESET_HEADER();
        PRINT_FLD16(sqcb2, dst_qp, "%0x");
        PRINT_FLD24(sqcb2, header_template_size, "%0x");
        PRINT_FLD24(sqcb2, header_template_addr, "%0x");
        PRINT_FLD16(sqcb2, rrq_base_addr, "%0x");
        PRINT_FLD16(sqcb2, log_rrq_size, "%0x");
        PRINT_FLD16(sqcb2, log_sq_size, "%0x");
        PRINT_FLD24(sqcb2, roce_opt_ts_enable, "%0x");
        PRINT_FLD24(sqcb2, roce_opt_mss_enable, "%0x");
        PRINT_FLD16(sqcb2, service, "%0x");
        PRINT_FLD16(sqcb2, lsn_tx, "%0x");
        PRINT_FLD16(sqcb2, lsn_rx, "%0x");
        PRINT_FLD16(sqcb2, rexmit_psn, "%0lx");
        PRINT_FLD24(sqcb2, last_ack_or_req_ts, "%0lx");
        PRINT_FLD16(sqcb2, err_retry_ctr, "%0lx");
        PRINT_FLD16(sqcb2, rnr_retry_ctr, "%0lx");
        PRINT_FLD16(sqcb2, rnr_timeout, "%0lx");
        PRINT_FLD16(sqcb2, rsvd1, "%0lx");
        PRINT_FLD16(sqcb2, timer_on, "%0lx");
        PRINT_FLD24(sqcb2, local_ack_timeout, "%0lx");
        PRINT_FLD16(sqcb2, tx_psn, "%0lx");
        PRINT_FLD16(sqcb2, ssn, "%0lx");
        PRINT_FLD16(sqcb2, lsn, "%0lx");
        PRINT_FLD16(sqcb2, wqe_start_psn, "%0lx");
        PRINT_FLD16(sqcb2, inv_key, "%0x");
        PRINT_FLD16(sqcb2, sq_cindex, "%0x");
        PRINT_FLD16(sqcb2, rrq_pindex, "%0x");
        PRINT_FLD16(sqcb2, rrq_cindex, "%0x");
        PRINT_FLD16(sqcb2, busy, "%0x");
        PRINT_FLD16(sqcb2, need_credits, "%0x");
        PRINT_FLD16(sqcb2, rsvd2, "%0x");
        PRINT_FLD16(sqcb2, fence, "%0x");
        PRINT_FLD16(sqcb2, li_fence, "%0x");
        PRINT_FLD16(sqcb2, fence_done, "%0x");
        PRINT_FLD16(sqcb2, curr_op_type, "%0x");
        PRINT_FLD16(sqcb2, exp_rsp_psn, "%0x");
        PRINT_FLD16(sqcb2, timestamp, "%0x");
        PRINT_FLD16(sqcb2, disable_credits, "%0x");
        PRINT_FLD16(sqcb2, rsvd3, "%0x");
        PRINT_FLD16(sqcb2, sq_msg_psn, "%0x\n");

        sqcb4 = (sqcb4_t *)memrev(&qstate.sq.sqcb4, sizeof *sqcb4);
        RESET_HEADER();
        PRINT_FLD16(sqcb4, num_bytes, "%0lx");
        PRINT_FLD16(sqcb4, num_pkts, "%0x");
        PRINT_FLD16(sqcb4, num_send_msgs, "%0x");
        PRINT_FLD16(sqcb4, num_write_msgs, "%0x");
        PRINT_FLD24(sqcb4, num_read_req_msgs, "%0x");
        PRINT_FLD24(sqcb4, num_atomic_fna_msgs, "%0x");
        PRINT_FLD24(sqcb4, num_atomic_cswap_msgs, "%0x");
        PRINT_FLD24(sqcb4, num_send_msgs_inv_rkey, "%0x");
        PRINT_FLD24(sqcb4, num_send_msgs_imm_data, "%0x");
        PRINT_FLD24(sqcb4, num_write_msgs_imm_data, "%0x");
        PRINT_FLD24(sqcb4, num_pkts_in_cur_msg, "%0x");
        PRINT_FLD24(sqcb4, max_pkts_in_any_msg, "%0x");
        PRINT_FLD16(sqcb4, num_npg_req, "%0x");
        PRINT_FLD24(sqcb4, num_npg_bindmw_t1_req, "%0x");
        PRINT_FLD24(sqcb4, num_npg_bindmw_t2_req, "%0x");
        PRINT_FLD16(sqcb4, num_npg_frpmr_req, "%0x");
        PRINT_FLD24(sqcb4, num_npg_local_inv_req, "%0x");
        PRINT_FLD16(sqcb4, num_inline_req, "%0x");
        PRINT_FLD24(sqcb4, num_timeout_local_ack, "%0x");
        PRINT_FLD16(sqcb4, num_timeout_rnr, "%0x");
        PRINT_FLD16(sqcb4, num_sq_drains, "%0x");
        PRINT_FLD16(sqcb4, np_cnp_sent, "%0x");
        PRINT_FLD24(sqcb4, rp_num_byte_threshold_db, "%0x");
        PRINT_FLD16(sqcb4, qp_err_disabled, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_flush_rq, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_ud_pmtu, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_ud_fast_reg, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_ud_priv, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_no_dma_cmds, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_lkey_inv_state, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_lkey_inv_pd, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_lkey_rsvd_lkey, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_lkey_access_violation, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_len_exceeded, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_rkey_inv_pd, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_rkey_inv_zbva, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_rkey_inv_len, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_rkey_inv_mw_state, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_rkey_type_disallowed, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_lkey_state_valid, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_lkey_no_bind, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_lkey_zero_based, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_lkey_invalid_acc_ctrl, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bind_mw_lkey_invalid_va, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bktrack_inv_num_sges, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_bktrack_inv_rexmit_psn, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_frpmr_fast_reg_not_enabled, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_frpmr_invalid_pd, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_frpmr_invalid_state, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_frpmr_invalid_len, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_frpmr_ukey_not_enabled, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_inv_lkey_qp_mismatch, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_inv_lkey_pd_mismatch, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_inv_lkey_invalid_state, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_inv_lkey_inv_not_allowed, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_table_error, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_phv_intrinsic_error, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_table_resp_error, "%0lx");
        PRINT_FLD24(sqcb4, qp_err_dis_inv_optype, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_err_retry_exceed, "%0lx");
        PRINT_FLD32(sqcb4, qp_err_dis_rnr_retry_exceed, "%0lx");
        PRINT_FLD16(sqcb4, qp_err_dis_rsvd, "%0lx\n");

        sqcb5 = (sqcb5_t *)memrev(&qstate.sq.sqcb5, sizeof *sqcb5);
        RESET_HEADER();
        PRINT_FLD16(sqcb5, num_bytes, "%0lx");
        PRINT_FLD16(sqcb5, num_pkts, "%0x");
        PRINT_FLD24(sqcb5, num_read_resp_pkts, "%0x");
        PRINT_FLD24(sqcb5, num_read_resp_msgs, "%0x");
        PRINT_FLD16(sqcb5, num_feedback, "%0x");
        PRINT_FLD16(sqcb5, num_ack, "%0x");
        PRINT_FLD16(sqcb5, num_atomic_ack, "%0x");
        PRINT_FLD24(sqcb5, num_pkts_in_cur_msg, "%0x");
        PRINT_FLD24(sqcb5, max_pkts_in_any_msg, "%0x");
        PRINT_FLD16(sqcb5, qp_err_disabled, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_pd_mismatch, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_inv_state, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_rsvd_lkey, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_acc_no_wr_perm, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_acc_len_lower, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqlkey_acc_len_higher, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqsge_insuff_sges, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqsge_insuff_sge_len, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqsge_insuff_dma_cmds, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqwqe_remote_inv_req_err_rcvd, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqwqe_remote_acc_err_rcvd, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_rrqwqe_remote_oper_err_rcvd, "%0x");
        PRINT_FLD24(sqcb5, qp_err_dis_table_error, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_phv_intrinsic_error, "%0x");
        PRINT_FLD32(sqcb5, qp_err_dis_table_resp_error, "%0x");
        PRINT_FLD16(sqcb5, qp_err_dis_rsvd, "%0x");
        PRINT_FLD16(sqcb5, recirc_bth_psn, "%0x");
        PRINT_FLD24(sqcb5, recirc_bth_opcode, "%0x");
        PRINT_FLD16(sqcb5, recirc_reason, "%0x");
        PRINT_FLD24(sqcb5, max_recirc_cnt_err, "%0x");
        PRINT_FLD16(sqcb5, rsvd1, "%0x");
        break;

    case 4: // RQ
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate.rq, sizeof(qstate.rq));

        rqcb0 = (rqcb0_t *)memrev(&qstate.rq.rqcb0, sizeof *rqcb0);
        if (!rqcb0->total_rings)
            break;

        printf("\nrq %u addr %lx size %lx\n",
               qid, addr, sizeof(qstate.rq));

        RESET_HEADER();
        PRINT_FLD16(rqcb0, log_pmtu, "%0x");
        PRINT_FLD16(rqcb0, serv_type, "%0x");
        PRINT_FLD16(rqcb0, bt_rsq_cindex, "%0x");
        PRINT_FLD16(rqcb0, rsvd1, "%0x");
        PRINT_FLD16(rqcb0, dcqcn_cfg_id, "%0x");
        PRINT_FLD32(rqcb0, ring_empty_sched_eval_done, "%0x");
        PRINT_FLD24(rqcb0, header_template_size, "%0x");
        PRINT_FLD24(rqcb0, curr_read_rsp_psn, "%0x");
        PRINT_FLD16(rqcb0, rsvd0, "%0x");
        PRINT_FLD16(rqcb0, drain_done, "%0x");
        PRINT_FLD16(rqcb0, bt_in_progress, "%0x");
        PRINT_FLD16(rqcb0, bt_lock, "%0x");
        PRINT_FLD16(rqcb0, rq_in_hbm, "%0x");
        PRINT_FLD24(rqcb0, read_rsp_in_progress, "%0x");
        PRINT_FLD16(rqcb0, curr_color, "%0x");
        PRINT_FLD16(rqcb0, dst_qp, "%0x");
        PRINT_FLD24(rqcb0, header_template_addr, "%0x");
        PRINT_FLD16(rqcb0, rsvd, "%0x");
        PRINT_FLD16(rqcb0, prefetch_en, "%0x");
        PRINT_FLD16(rqcb0, skip_pt, "%0x");
        PRINT_FLD16(rqcb0, drain_in_progress, "%0x");
        PRINT_FLD16(rqcb0, spec_color, "%0x");
        PRINT_FLD24(rqcb0, spec_read_rsp_psn, "%0x");
        PRINT_FLD16(rqcb0, rsq_base_addr, "%0x");
        PRINT_FLD16(rqcb0, phy_base_addr, "%0lx");
        PRINT_FLD16(rqcb0, log_rsq_size, "%0lx");
        PRINT_FLD16(rqcb0, state, "%0lx");
        PRINT_FLD24(rqcb0, congestion_mgmt_enable, "%0lx");
        PRINT_FLD16(rqcb0, log_num_wqes, "%0lx");
        PRINT_FLD16(rqcb0, log_wqe_size, "%0lx");
        PRINT_FLD16(rqcb0, log_rq_page_size, "%0lx");
        PRINT_FLD16(rqcb0, c_index5, "%0x");
        PRINT_FLD16(rqcb0, p_index5, "%0x");
        PRINT_FLD16(rqcb0, c_index4, "%0x");
        PRINT_FLD16(rqcb0, p_index4, "%0x");
        PRINT_FLD16(rqcb0, c_index3, "%0x");
        PRINT_FLD16(rqcb0, p_index3, "%0x");
        PRINT_FLD16(rqcb0, c_index2, "%0x");
        PRINT_FLD16(rqcb0, p_index2, "%0x");
        PRINT_FLD16(rqcb0, c_index1, "%0x");
        PRINT_FLD16(rqcb0, p_index1, "%0x");
        PRINT_FLD16(rqcb0, c_index0, "%0x");
        PRINT_FLD16(rqcb0, p_index0, "%0x");
        PRINT_FLD16(rqcb0, pid, "%0x");
        PRINT_FLD16(rqcb0, host_rings, "%0x");
        PRINT_FLD16(rqcb0, total_rings, "%0x");
        PRINT_FLD16(rqcb0, eval_last, "%0x");
        PRINT_FLD16(rqcb0, cos_selector, "%0x");
        PRINT_FLD16(rqcb0, cosA, "%0x");
        PRINT_FLD16(rqcb0, cosB, "%0x");
        PRINT_FLD16(rqcb0, intrinsic_rsvd, "%0x");
        PRINT_FLD16(rqcb0, pc, "%0x\n");

        rqcb1 = (rqcb1_t *)memrev(&qstate.rq.rqcb1, sizeof *rqcb1);
        RESET_HEADER();
        PRINT_FLD16(rqcb1, log_pmtu, "%0x");
        PRINT_FLD16(rqcb1, serv_type, "%0x");
        PRINT_FLD16(rqcb1, srq_id, "%0x");
        PRINT_FLD16(rqcb1, proxy_pindex, "%0x");
        PRINT_FLD16(rqcb1, proxy_cindex, "%0x");
        PRINT_FLD16(rqcb1, num_sges, "%0x");
        PRINT_FLD16(rqcb1, current_sge_id, "%0x");
        PRINT_FLD24(rqcb1, current_sge_offset, "%0x");
        PRINT_FLD16(rqcb1, curr_wqe_ptr, "%0lx");
        PRINT_FLD16(rqcb1, rsq_pindex, "%0x");
        PRINT_FLD16(rqcb1, bt_in_progress, "%0x");
        PRINT_FLD24(rqcb1, header_template_size, "%0x");
        PRINT_FLD16(rqcb1, msn, "%0x");
        PRINT_FLD16(rqcb1, rsvd3, "%0x");
        PRINT_FLD16(rqcb1, access_flags, "%0x");
        PRINT_FLD16(rqcb1, spec_en, "%0x");
        PRINT_FLD16(rqcb1, next_pkt_type, "%0x");
        PRINT_FLD16(rqcb1, next_op_type, "%0x");
        PRINT_FLD16(rqcb1, e_psn, "%0x");
        PRINT_FLD16(rqcb1, spec_cindex, "%0x");
        PRINT_FLD16(rqcb1, rsvd2, "%0x");
        PRINT_FLD16(rqcb1, in_progress, "%0x");
        PRINT_FLD16(rqcb1, rsvd1, "%0x");
        PRINT_FLD16(rqcb1, busy, "%0x");
        PRINT_FLD24(rqcb1, work_not_done_recirc_cnt, "%0x");
        PRINT_FLD24(rqcb1, nxt_to_go_token_id, "%0x");
        PRINT_FLD16(rqcb1, token_id, "%0x");
        PRINT_FLD24(rqcb1, header_template_addr, "%0x");
        PRINT_FLD16(rqcb1, pd, "%0x");
        PRINT_FLD16(rqcb1, cq_id, "%0x");
        PRINT_FLD16(rqcb1, prefetch_en, "%0x");
        PRINT_FLD16(rqcb1, skip_pt, "%0x");
        PRINT_FLD16(rqcb1, priv_oper_enable, "%0x");
        PRINT_FLD16(rqcb1, nak_prune, "%0x");
        PRINT_FLD16(rqcb1, rq_in_hbm, "%0x");
        PRINT_FLD16(rqcb1, immdt_as_dbell, "%0x");
        PRINT_FLD16(rqcb1, cache, "%0x");
        PRINT_FLD16(rqcb1, srq_enabled, "%0x");
        PRINT_FLD16(rqcb1, log_rsq_size, "%0lx");
        PRINT_FLD16(rqcb1, state, "%0lx");
        PRINT_FLD24(rqcb1, congestion_mgmt_enable, "%0lx");
        PRINT_FLD16(rqcb1, log_num_wqes, "%0lx");
        PRINT_FLD16(rqcb1, log_wqe_size, "%0lx");
        PRINT_FLD16(rqcb1, log_rq_page_size, "%0lx");
        PRINT_FLD16(rqcb1, phy_base_addr, "%0lx");
        PRINT_FLD16(rqcb1, rsq_base_addr, "%0x");
        PRINT_FLD16(rqcb1, pc, "%0x\n");

        rqcb2 = (rqcb2_t *)memrev(&qstate.rq.rqcb2, sizeof *rqcb2);
        RESET_HEADER();
        PRINT_FLD24(rqcb2, prefetch_init_done, "%0x");
        PRINT_FLD16(rqcb2, checkout_done, "%0x");
        PRINT_FLD24(rqcb2, prefetch_buf_index, "%0x");
        PRINT_FLD16(rqcb2, rsvd3, "%0x");
        PRINT_FLD24(rqcb2, log_num_prefetch_wqes, "%0x");
        PRINT_FLD24(rqcb2, prefetch_base_addr, "%0x");
        PRINT_FLD24(rqcb2, prefetch_proxy_cindex, "%0x");
        PRINT_FLD16(rqcb2, prefetch_pindex, "%0x");
        PRINT_FLD16(rqcb2, proxy_cindex, "%0x");
        PRINT_FLD16(rqcb2, pd, "%0x");
        PRINT_FLD16(rqcb2, rsvd2, "%0x");
        PRINT_FLD16(rqcb2, rnr_timeout, "%0x");
        PRINT_FLD16(rqcb2, len, "%0x");
        PRINT_FLD16(rqcb2, r_key, "%0x");
        PRINT_FLD16(rqcb2, va, "%0lx");
        PRINT_FLD16(rqcb2, psn, "%0x");
        PRINT_FLD16(rqcb2, rsvd, "%0x");
        PRINT_FLD16(rqcb2, read_or_atomic, "%0x");
        PRINT_FLD16(rqcb2, rsvd1, "%0x");
        PRINT_FLD16(rqcb2, credits, "%0x");
        PRINT_FLD16(rqcb2, syndrome, "%0x");
        PRINT_FLD16(rqcb2, msn, "%0x");
        PRINT_FLD16(rqcb2, ack_nak_psn, "%0x");
        PRINT_FLD16(rqcb2, rsvd0, "%0x\n");

        rqcb3 = (rqcb3_t *)memrev(&qstate.rq.rqcb3, sizeof *rqcb3);
        RESET_HEADER();
        PRINT_FLD24(rqcb3, resp_rx_timestamp, "%0lx");
        PRINT_FLD24(rqcb3, resp_tx_timestamp, "%0lx");
        PRINT_FLD16(rqcb3, dma_len, "%0x");
        PRINT_FLD24(rqcb3, num_pkts_in_curr_msg, "%0x");
        PRINT_FLD16(rqcb3, rsvd1, "%0x");
        PRINT_FLD16(rqcb3, roce_opt_mss, "%0x");
        PRINT_FLD16(rqcb3, roce_opt_ts_echo, "%0x");
        PRINT_FLD24(rqcb3, roce_opt_ts_value, "%0x");
        PRINT_FLD16(rqcb3, r_key, "%0x");
        PRINT_FLD16(rqcb3, len, "%0x");
        PRINT_FLD16(rqcb3, va, "%0lx");
        PRINT_FLD16(rqcb3, wrid, "%0lx");

        rqcb4 = (rqcb4_t *)memrev(&qstate.rq.rqcb4, sizeof *rqcb4);
        RESET_HEADER();
        PRINT_FLD24(rqcb4, qp_err_dis_resp_rx, "%0x");
        PRINT_FLD32(rqcb4, qp_err_dis_type2a_mw_qp_mismatch, "%0x");
        PRINT_FLD24(rqcb4, qp_err_dis_rkey_va_err, "%0x");
        PRINT_FLD32(rqcb4, qp_err_dis_rkey_acc_ctrl_err, "%0x");
        PRINT_FLD32(rqcb4, qp_err_dis_rkey_pd_mismatch, "%0x");
        PRINT_FLD32(rqcb4, qp_err_dis_rkey_state_err, "%0x");
        PRINT_FLD24(rqcb4, qp_err_dis_rsvd_rkey_err, "%0x");
        PRINT_FLD16(rqcb4, qp_err_disabled, "%0x");
        PRINT_FLD24(rqcb4, rp_num_max_rate_reached, "%0x");
        PRINT_FLD24(rqcb4, rp_num_timer_T_expiry, "%0x");
        PRINT_FLD32(rqcb4, rp_num_alpha_timer_expiry, "%0x");
        PRINT_FLD24(rqcb4, rp_num_byte_threshold_db, "%0x");
        PRINT_FLD24(rqcb4, rp_num_hyper_increase, "%0x");
        PRINT_FLD24(rqcb4, rp_num_fast_recovery, "%0x");
        PRINT_FLD24(rqcb4, rp_num_additive_increase, "%0x");
        PRINT_FLD16(rqcb4, last_msn, "%0x");
        PRINT_FLD16(rqcb4, last_syndrome, "%0x");
        PRINT_FLD16(rqcb4, last_psn, "%0x");
        PRINT_FLD16(rqcb4, num_seq_errs, "%0x");
        PRINT_FLD16(rqcb4, num_rnrs, "%0x");
        PRINT_FLD16(rqcb4, num_prefetch, "%0x");
        PRINT_FLD24(rqcb4, max_pkts_in_any_msg, "%0x");
        PRINT_FLD24(rqcb4, num_pkts_in_cur_msg, "%0x");
        PRINT_FLD24(rqcb4, num_atomic_resp_msgs, "%0x");
        PRINT_FLD24(rqcb4, num_read_resp_msgs, "%0x");
        PRINT_FLD16(rqcb4, num_acks, "%0x");
        PRINT_FLD24(rqcb4, num_read_resp_pkts, "%0x");
        PRINT_FLD16(rqcb4, num_pkts, "%0x");
        PRINT_FLD16(rqcb4, num_bytes, "%0lx\n");

        rqcb5 = (rqcb5_t *)memrev(&qstate.rq.rqcb5, sizeof *rqcb5);
        RESET_HEADER();
        PRINT_FLD24(rqcb5, max_recirc_cnt_err, "%0x");
        PRINT_FLD16(rqcb5, recirc_reason, "%0x");
        PRINT_FLD16(rqcb5, last_bth_opcode, "%0x");
        PRINT_FLD16(rqcb5, recirc_bth_psn, "%0x");
        PRINT_FLD16(rqcb5, qp_err_dis_rsvd, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_table_resp_error, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_phv_intrinsic_error, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_table_error, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_feedback, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_mr_cookie_mismatch, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_mr_state_invalid, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_mr_mw_pd_mismatch, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_type2a_mw_qp_mismatch, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_type1_mw_inv_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_inv_rkey_state_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_ineligible_mr_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_inv_rkey_rsvd_key_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_key_va_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_user_key_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_key_acc_ctrl_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_key_pd_mismatch, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_key_state_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_rsvd_key_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_max_sge_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_insuff_sge_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_dma_len_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_unaligned_atomic_va_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_wr_only_zero_len_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_access_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_opcode_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_pmtu_err, "%0x");
        PRINT_FLD32(rqcb5, qp_err_dis_last_pkt_len_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_pyld_len_err, "%0x");
        PRINT_FLD24(rqcb5, qp_err_dis_svc_type_err, "%0x");
        PRINT_FLD16(rqcb5, qp_err_disabled, "%0x");
        PRINT_FLD16(rqcb5, rp_cnp_processed, "%0x");
        PRINT_FLD24(rqcb5, np_ecn_marked_packets, "%0x");
        PRINT_FLD32(rqcb5, num_dup_rd_atomic_drop_pkts, "%0x");
        PRINT_FLD32(rqcb5, num_dup_rd_atomic_bt_pkts, "%0x");
        PRINT_FLD24(rqcb5, num_dup_wr_send_pkts, "%0x");
        PRINT_FLD24(rqcb5, num_mem_window_inv, "%0x");
        PRINT_FLD24(rqcb5, num_recirc_drop_pkts, "%0x");
        PRINT_FLD24(rqcb5, max_pkts_in_any_msg, "%0x");
        PRINT_FLD24(rqcb5, num_pkts_in_cur_msg, "%0x");
        PRINT_FLD16(rqcb5, num_ring_dbell, "%0x");
        PRINT_FLD16(rqcb5, num_ack_requested, "%0x");
        PRINT_FLD24(rqcb5, num_write_msgs_imm_data, "%0x");
        PRINT_FLD24(rqcb5, num_send_msgs_imm_data, "%0x");
        PRINT_FLD24(rqcb5, num_send_msgs_inv_rkey, "%0x");
        PRINT_FLD24(rqcb5, num_atomic_cswap_msgs, "%0x");
        PRINT_FLD24(rqcb5, num_atomic_fna_msgs, "%0x");
        PRINT_FLD16(rqcb5, num_read_req_msgs, "%0x");
        PRINT_FLD16(rqcb5, num_write_msgs, "%0x");
        PRINT_FLD16(rqcb5, num_send_msgs, "%0x");
        PRINT_FLD16(rqcb5, num_pkts, "%0x");
        PRINT_FLD16(rqcb5, num_bytes, "%0lx");
        break;

    case 5: //CQ
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate.cq, sizeof(qstate.cq));

        cqcb0 = (cqcb0_t *)memrev(&qstate.cq, sizeof *cqcb0);
        if (!cqcb0->total_rings)
            break;

        printf("\ncq %u addr %lx size %lx\n",
               qid, addr, sizeof(qstate.cq));

        RESET_HEADER();
        PRINT_FLD16(cqcb0, pc, "%0x");
        PRINT_FLD16(cqcb0, rsvd, "%0x");
        PRINT_FLD16(cqcb0, cosB, "%0x");
        PRINT_FLD16(cqcb0, cosA, "%0x");
        PRINT_FLD16(cqcb0, cos_selector, "%0x");
        PRINT_FLD16(cqcb0, eval_last, "%0x");
        PRINT_FLD16(cqcb0, total_rings, "%0x");
        PRINT_FLD16(cqcb0, host_rings, "%0x");
        PRINT_FLD16(cqcb0, pid, "%0x");
        PRINT_FLD16(cqcb0, p_index0, "%0x");
        PRINT_FLD16(cqcb0, c_index0, "%0x");
        PRINT_FLD16(cqcb0, p_index1, "%0x");
        PRINT_FLD16(cqcb0, c_index1, "%0x");
        PRINT_FLD16(cqcb0, p_index2, "%0x");
        PRINT_FLD16(cqcb0, c_index2, "%0x");
        PRINT_FLD16(cqcb0, proxy_pindex, "%0x");
        PRINT_FLD16(cqcb0, proxy_s_pindex, "%0x");
        PRINT_FLD16(cqcb0, pt_base_addr, "%0x");
        PRINT_FLD16(cqcb0, log_cq_page_size, "%0x");
        PRINT_FLD16(cqcb0, log_wqe_size, "%0x");
        PRINT_FLD16(cqcb0, log_num_wqes, "%0x");
        PRINT_FLD32(cqcb0, ring_empty_sched_eval_done, "%0x");
        PRINT_FLD16(cqcb0, cq_id, "%0x");
        PRINT_FLD16(cqcb0, eq_id, "%0x");
        PRINT_FLD16(cqcb0, arm, "%0x");
        PRINT_FLD16(cqcb0, sarm, "%0x");
        PRINT_FLD16(cqcb0, wakeup_dpath, "%0x");
        PRINT_FLD16(cqcb0, color, "%0x");
        PRINT_FLD16(cqcb0, wakeup_lif, "%0x");
        PRINT_FLD16(cqcb0, wakeup_qtype, "%0x");
        PRINT_FLD16(cqcb0, wakeup_qid, "%0x");
        PRINT_FLD16(cqcb0, wakeup_ring_id, "%0x");
        PRINT_FLD16(cqcb0, cq_full_hint, "%0x");
        PRINT_FLD16(cqcb0, cq_full, "%0x");
        PRINT_FLD16(cqcb0, pt_pg_index, "%0x");
        PRINT_FLD16(cqcb0, pt_next_pg_index, "%0x");
        PRINT_FLD16(cqcb0, host_addr, "%0x");
        PRINT_FLD16(cqcb0, is_phy_addr, "%0x");
        PRINT_FLD16(cqcb0, pad, "%0x");
        PRINT_FLD16(cqcb0, pt_pa, "%0lx");
        PRINT_FLD16(cqcb0, pt_next_pa, "%0lx");
        break;

    case 6: //EQ
        sdk::lib::pal_mem_read(addr, (uint8_t *)&qstate.eq, sizeof(qstate.eq));

        eqcb0 = (eqcb0_t *)memrev(&qstate.eq, sizeof *eqcb0);
        if (!eqcb0->log_num_wqes)
            break;

        printf("\neq %u addr %lx size %lx\n",
               qid, addr, sizeof(qstate.eq));

        RESET_HEADER();
        PRINT_FLD16(eqcb0, pc, "%0x");
        PRINT_FLD16(eqcb0, rsvd, "%0x");
        PRINT_FLD16(eqcb0, cosb, "%0x");
        PRINT_FLD16(eqcb0, cosa, "%0x");
        PRINT_FLD16(eqcb0, cos_selector, "%0x");
        PRINT_FLD16(eqcb0, eval_last, "%0x");
        PRINT_FLD16(eqcb0, total_rings, "%0x");
        PRINT_FLD16(eqcb0, host_rings, "%0x");
        PRINT_FLD16(eqcb0, pid, "%0x");
        PRINT_FLD16(eqcb0, pindex, "%0x");
        PRINT_FLD16(eqcb0, cindex, "%0x");
        PRINT_FLD16(eqcb0, eqe_base_addr, "%0lx");
        PRINT_FLD16(eqcb0, rsvd0, "%0x");
        PRINT_FLD16(eqcb0, eq_id, "%0x");
        PRINT_FLD16(eqcb0, log_num_wqes, "%0x");
        PRINT_FLD16(eqcb0, log_wqe_size, "%0x");
        PRINT_FLD16(eqcb0, int_enabled, "%0x");
        PRINT_FLD16(eqcb0, color, "%0x");
        PRINT_FLD16(eqcb0, rsvd1, "%0x");
        PRINT_FLD16(eqcb0, int_assert_addr, "%0lx");
        PRINT_FLD16(eqcb0, rsvd2[0], "%0lx");
        PRINT_FLD16(eqcb0, rsvd2[1], "%0lx");
        PRINT_FLD16(eqcb0, rsvd2[2], "%0lx");
        break;

    default:
        break;
    }
}

static const char qnames[][16] = {
    "unknown", "unknown", "AQ", "SQ", "RQ", "CQ", "EQ", "unknown"
};

void
rdma_qstate(uint16_t lif, uint8_t qtype, uint32_t qid)
{
    queue_info_t qinfo[QTYPE_MAX] = {0};

    if (!get_lif_qstate(lif, qinfo)) {
        printf("Failed to get qinfo for lif %u\n", lif);
        return;
    }

    if (qtype >= QTYPE_MAX || qinfo[qtype].size == 0) {
        printf("Invalid type %u for lif %u\n", qtype, lif);
        return;
    }

    if (qid >= qinfo[qtype].length) {
        printf("Invalid qid %u for lif %u qtype %u (%s)\n",
	       qid, lif, qtype, qnames[qtype]);
        return;
    }

    /* Print the state of a single queue */
    printf("RDMA queue state for lif %u qtype %u (%s) qid %u:\n",
	   lif, qtype, qnames[qtype], qid);
    rdma_qstate_one(qinfo, qtype, qid);
}

void
rdma_qstate_all(uint16_t lif, uint8_t qtype)
{
    queue_info_t qinfo[QTYPE_MAX] = {0};
    uint8_t qtype_idx;
    uint8_t start_qtype = 0;
    uint8_t end_qtype = QTYPE_MAX;
    uint32_t qid;

    if (!get_lif_qstate(lif, qinfo)) {
        printf("Failed to get qinfo for lif %u\n", lif);
        return;
    }

    // specific qtype
    if (qtype != QTYPE_MAX) {
        start_qtype = qtype;
        end_qtype = qtype + 1;
    }

    if (start_qtype >= QTYPE_MAX || qinfo[start_qtype].size == 0) {
        printf("Invalid type %u for lif %u\n", start_qtype, lif);
        return;
    }

    /* Print the state of every queue */
    printf("RDMA queue state for lif %u:\n", lif);
    for (qtype_idx = start_qtype; qtype_idx < end_qtype; qtype_idx++) {
        if (qinfo[qtype_idx].size == 0)
            continue;

        for (qid = 0; qid < qinfo[qtype_idx].length; qid++)
            rdma_qstate_one(qinfo, qtype_idx, qid);
    }
}
