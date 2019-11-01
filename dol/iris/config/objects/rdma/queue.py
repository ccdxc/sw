#! /usr/bin/python3
import pdb
import math
import time

import infra.common.defs        as defs
import infra.common.objects     as objects
import iris.config.resmgr            as resmgr
import iris.config.objects.rdma.ring  as ring
import iris.config.hal.api           as halapi
import iris.config.hal.defs          as haldefs

from iris.config.store               import Store
from infra.common.logging       import logger
from iris.config.objects.queue       import QueueObject

import model_sim.src.model_wrap as model_wrap
from infra.common.glopts import GlobalOptions

from scapy.all import *


class RdmaAQstate(Packet):
    name = "RdmaAQstate"
    fields_desc = [

        # AQCB0
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        ShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),

        IntField("map_count_completed", 0),
        ByteField("first_pass", 0),
        ByteField("token_id", 0),

        BitField("rsvd1", 0, 24),
        ByteField("debug", 0),
        
        BitField("log_wqe_size", 0, 5),
        BitField("log_num_size", 0, 5),
        BitField("ring_empty_sched_eval_done", 0, 1), 
        BitField("rsvd2", 0 , 5),

        XLongField("phy_base_addr", 0),

        ByteField("next_token_id", 0),
        X3BytesField("aq_id", 0),
        X3BytesField("cq_id", 0),
        ByteField("error", 0),

        XLongField("cqcb_addr", 0),
        BitField("pad", 0, 128),

        # AQCB1
        ShortField("num_nops", 0),
        ShortField("num_create_cq", 0),
        ShortField("num_create_qp", 0),
        ShortField("num_reg_mr", 0),
        ShortField("num_stats_hdrs", 0),
        ShortField("num_stats_vals", 0),
        ShortField("num_dereg_mr", 0),
        ShortField("num_resize_cq", 0),
        ShortField("num_destroy_cq", 0),
        ShortField("num_modify_qp", 0),
        ShortField("num_query_qp", 0),
        ShortField("num_destroy_qp", 0),
        ShortField("num_stats_dump", 0),
        ShortField("num_create_ah", 0),
        ShortField("num_query_ah", 0),
        ShortField("num_destroy_ah", 0),
        LongField("num_any", 0),
        BitField("aqcb1_pad", 0, 192),
    ]

class RdmaRQstate(Packet):
    name = "RdmaRQstate"
    fields_desc = [

        # RQCB0
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        ShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),
        LEShortField("p_index1", 0),
        LEShortField("c_index1", 0),
        LEShortField("p_index2", 0),
        LEShortField("c_index2", 0),
        LEShortField("p_index3", 0),
        LEShortField("c_index3", 0),
        LEShortField("p_index4", 0),
        LEShortField("c_index4", 0),
        LEShortField("p_index5", 0),
        LEShortField("c_index5", 0),

        BitField("log_rq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("rsvd2", 0, 1),
        BitField("cb0_state", 0, 3),
        BitField("log_rsq_size", 0, 5),

        XByteField("phy_base_addr_8", 0),
        XIntField("pt_base_addr/rq_hbm_base_addr/phy_base_addr_32", 0),

        XIntField("rsq_base_addr/rqcb0_q_key", 0),

        X3BytesField("spec_read_rsp_psn", 0),
        BitField("spec_color", 0, 1),
        BitField("drain_in_progress", 0, 1),
        BitField("skip_pt", 0, 1),
        BitField("prefetch_en", 0, 1),
        BitField("congestion_mgmt_type", 0, 2),
        BitField("rsvd", 0, 2),

        XIntField("header_template_addr", 0),

        X3BytesField("dst_qp", 0),
        BitField("curr_color", 0, 1),
        BitField("read_rsp_in_progress", 0, 1),
        BitField("rq_in_hbm", 0, 1),
        BitField("bt_lock", 0, 1),
        BitField("rqcb0_bt_in_progress", 0, 1),
        BitField("drain_done", 0, 1),
        BitField("rqcb0_rsvd0", 0, 2),

        X3BytesField("curr_read_rsp_psn", 0),
        ByteField("header_template_size", 0),

        BitField("ring_empty_sched_eval_done", 0, 1),
        BitField("dcqcn_cfg_id", 0, 4),
        BitField("rqcb0_rsvd1", 0, 3),
        ShortField("bt_rsq_cindex", 0),

        BitField("serv_type", 0, 3),
        BitField("log_pmtu", 0xa, 5),

        # RQCB1
        ByteField("pc_offset", 0),

        XIntField("rsq_base_addr/q_key", 0),

        XByteField("phy_base_addr_8", 0),
        XIntField("pt_base_addr/rq_hbm_base_addr/phy_base_addr_32", 0),

        BitField("log_rq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("rsvd4", 0, 1),
        BitField("cb1_state", 0, 3),
        BitField("log_rsq_size", 0, 5),

        BitField("srq_enabled", 0, 1),
        BitField("cache", 0, 1),
        BitField("immdt_as_dbell", 0, 1),
        BitField("rq_in_hbm", 0, 1),
        BitField("nak_prune", 0, 1),
        BitField("priv_oper_enable", 0, 1),
        BitField("skip_pt", 0, 1),
        BitField("prefetch_en", 0, 1),

        X3BytesField("cq_id", 0),

        IntField("pd", 0),

        XIntField("header_template_addr", 0),

        ByteField("token_id", 0),
        ByteField("nxt_to_go_token_id", 0),
        ByteField("work_not_done_recirc_cnt", 0),

        BitField("busy", 0, 1),
        BitField("congestion_mgmt_type",0, 2),
        BitField("rqcb1_rsvd1", 0, 5),
        BitField("in_progress", 0, 1),
        BitField("rqcb1_rsvd2", 0, 7),
        LEShortField("spec_cindex", 0),

        X3BytesField("e_psn", 0),
        BitField("next_op_type", 0, 2),
        BitField("next_pkt_type", 0, 1),
        BitField("spec_en", 0, 1),
        BitField("access_flags", 0, 3),
        BitField("rqcb1_rsvd3", 0, 1),

        X3BytesField("msn", 0),
        ByteField("header_template_size", 0),

        ByteField("rqcb1_bt_in_progress", 0),

        ByteField("rsq_pindex", 0),

        XLongField("curr_wqe_ptr", 0),
        IntField("current_sge_offset", 0),
        ByteField("current_sge_id", 0),
        ByteField("num_sges", 0),

        LEShortField("proxy_cindex", 0),
        LEShortField("proxy_pindex", 0),

        X3BytesField("srq_id", 0),

        BitField("serv_type", 0, 3),
        BitField("log_pmtu", 0xa, 5),

        # RQCB2
        ByteField("rqcb2_rsvd0", 0),
        X3BytesField("ack_nak_psn", 0),

        X3BytesField("aeth_msn", 0),
        ByteField("syndrome", 0),
        BitField("credits", 0, 5),
        BitField("rqcb2_rsvd1", 0, 3),

        BitField("bt_read_or_atomic", 0, 1),
        BitField("bt_rsvd0", 0, 7),
        X3BytesField("bt_psn", 0),
        LongField("bt_va", 0),
        IntField("bt_r_key", 0),
        IntField("bt_len", 0),

        BitField("rnr_timeout", 0, 5),
        BitField("rqcb2_rsvd2", 0, 3),

        IntField("pd", 0),

        LEShortField("rqcb2_proxy_cindex", 0),
        LEShortField("prefetch_pindex", 0),
        LEShortField("prefetch_proxy_cindex", 0),
        XIntField("prefetch_base_addr", 0),
        BitField("log_num_prefetch_wqes", 0, 5),
        BitField("rqcb2_rsvd3", 0, 3),
        ShortField("prefetch_buf_index", 0),
        BitField("checkout_done", 0, 1),
        BitField("prefetch_init_done", 0, 1),

        BitField("rqcb2_pad", 0, 134),

        # RQCB3
        LongField("wrid", 0),
        XLongField("va", 0),
        IntField("len", 0),
        IntField("r_key", 0),

        IntField("roce_opt_ts_value", 0),
        IntField("roce_opt_ts_echo", 0),

        ShortField("roce_opt_mss", 0),
        BitField("rqcb3_rsvd1", 0, 16),

        IntField("num_pkts_in_curr_msg", 0),
        IntField("dma_len", 0),

        BitField("rqcb3_pad", 0, 160),

        #RQCB4 - RESP_TX stats
        LongField("tx_num_bytes", 0),
        IntField("tx_num_pkts", 0),
        IntField("tx_num_read_resp_pkts", 0),
        IntField("tx_num_acks", 0),
        ShortField("tx_num_read_resp_msgs", 0),
        ShortField("tx_num_atomic_resp_msgs", 0),
        ShortField("tx_num_pkts_in_cur_msg", 0),
        ShortField("tx_max_pkts_in_any_msg", 0),
        ShortField("tx_num_prefetch", 0),
        IntField("tx_num_rnrs", 0),
        IntField("tx_num_seq_errs", 0),
        X3BytesField("tx_last_psn", 0),
        ByteField("tx_last_syndrome", 0),
        X3BytesField("tx_last_msn", 0),
        ShortField("rp_num_additive_increase", 0),
        ShortField("rp_num_fast_recovery", 0),
        ShortField("rp_num_hyper_increase", 0),
        ShortField("rp_num_byte_threshold_db", 0),
        ShortField("rp_num_alpha_timer_expiry", 0),
        ShortField("rp_num_timer_T_expiry", 0),
        ShortField("rp_num_max_rate_reached", 0),
        BitField("tx_qp_err_disabled", 0, 1),
        BitField("tx_qp_err_dis_rsvd_rkey_err", 0, 1),
        BitField("tx_qp_err_dis_rkey_state_err", 0, 1),
        BitField("tx_qp_err_dis_rkey_pd_mismatch", 0, 1),
        BitField("tx_qp_err_dis_rkey_acc_ctrl_err", 0, 1),
        BitField("tx_qp_err_dis_rkey_va_err", 0, 1),
        BitField("tx_qp_err_dis_type2a_mw_qp_mismatch", 0, 1),
        BitField("tx_qp_err_dis_resp_rx", 0, 1),

        BitField("rqcb4", 0, 32),

        #RQCB5 - RESP_RX stats
        LongField("num_bytes", 0),
        IntField("num_pkts", 0),
        ShortField("num_send_msgs", 0),
        ShortField("num_write_msgs", 0),
        ShortField("num_read_req_msgs", 0),
        ShortField("num_atomic_fna_msgs", 0),
        ShortField("num_atomic_cswap_msgs", 0),
        ShortField("num_send_msgs_inv_rkey", 0),
        ShortField("num_send_msgs_imm_data", 0),
        ShortField("num_write_msgs_imm_data", 0),
        ShortField("num_ack_requested", 0),
        ShortField("num_ring_dbell", 0),
        ShortField("num_pkts_in_cur_msg", 0),
        ShortField("max_pkts_in_any_msg", 0),
        ShortField("num_recirc_drop_pkts", 0),
        ShortField("num_mem_window_inv", 0),
        ShortField("num_dup_wr_send_pkts", 0),
        ShortField("num_dup_rd_atomic_bt_pkts", 0),
        ShortField("num_dup_rd_atomic_drop_pkts", 0),
        ShortField("np_ecn_marked_packets", 0),
        ShortField("rp_cnp_processed", 0),
        BitField("qp_err_disabled", 0, 1),
        BitField("qp_err_dis_svc_type_err", 0, 1),
        BitField("qp_err_dis_pyld_len_err", 0, 1),
        BitField("qp_err_dis_last_pkt_len_err", 0, 1),
        BitField("qp_err_dis_pmtu_err", 0, 1),
        BitField("qp_err_dis_opcode_err", 0, 1),
        BitField("qp_err_dis_access_err", 0, 1),
        BitField("qp_err_dis_wr_only_zero_len_err", 0, 1),
        BitField("qp_err_dis_unaligned_atomic_va_err", 0, 1),
        BitField("qp_err_dis_dma_len_err", 0, 1),
        BitField("qp_err_dis_insuff_sge_err", 0, 1),
        BitField("qp_err_dis_max_sge_err", 0, 1),
        BitField("qp_err_dis_rsvd_key_err", 0, 1),
        BitField("qp_err_dis_key_state_err", 0, 1),
        BitField("qp_err_dis_key_pd_mismatch", 0, 1),
        BitField("qp_err_dis_key_acc_ctrl_err", 0, 1),
        BitField("qp_err_dis_user_key_err", 0, 1),
        BitField("qp_err_dis_key_va_err", 0, 1),
        BitField("qp_err_dis_inv_rkey_rsvd_key_err", 0, 1),
        BitField("qp_err_dis_ineligible_mr_err", 0, 1),
        BitField("qp_err_dis_inv_rkey_state_err", 0, 1),
        BitField("qp_err_dis_type1_mw_inv_err", 0, 1),
        BitField("qp_err_dis_type2a_mw_qp_mismatch", 0, 1),
        BitField("qp_err_dis_mr_mw_pd_mismatch", 0, 1),
        BitField("qp_err_dis_mr_state_invalid", 0, 1),
        BitField("qp_err_dis_mr_cookie_mismatch", 0, 1),
        BitField("qp_err_dis_feedback", 0, 1),
        BitField("qp_err_dis_table_error", 0, 1),
        BitField("qp_err_dis_phv_intrinsic_error", 0, 1),
        BitField("qp_err_dis_table_resp_error", 0, 1),
        BitField("qp_err_dis_rsvd", 0, 2),
        X3BytesField("recirc_bth_psn", 0),
        ByteField("last_bth_opcode", 0),
        BitField("recirc_reason", 0, 4),
        BitField("max_recirc_cnt_err", 0, 1),
        BitField("pad", 0, 43),
    ]

class RdmaSQstate(Packet):
    name = "RdmaSQstate"
    fields_desc = [
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        LEShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),
        LEShortField("p_index1", 0),
        LEShortField("c_index1", 0),
        LEShortField("p_index2", 0),
        LEShortField("c_index2", 0),
        LEShortField("p_index3", 0),
        LEShortField("c_index3", 0),
        ShortField("sqd_cindex", 0),
        XByteField("sqcb0_rsvd1", 0),

        XByteField("phy_base_addr_8", 0),
        XIntField("pt_base_addr/sq_hbm_base_addr/phy_base_addr_32", 0),
        XIntField("sqcb0_header_template_addr/q_key", 0),
        IntField("pd", 0),

        BitField("poll_in_progress", 0, 1),
        BitField("log_pmtu", 0xa, 5),
        BitField("log_sq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("poll_for_work", 0, 1),
        BitField("signalled_completion", 0, 1),
        BitField("dcqcn_rl_failure", 0, 1),

        BitField("serv_type", 0, 4),
        BitField("flush_rq", 0, 1),
        BitField("state", 0, 3),
        BitField("sq_in_hbm", 0, 1),
        BitField("rsvd5",0, 1),
        BitField("local_ack_timeout", 0, 5),
        BitField("ring_empty_sched_eval_done", 0, 1),

        ShortField("spec_sq_cindex", 0),

        XLongField("curr_wqe_ptr", 0),
        ByteField("current_sge_id", 0),
        ByteField("num_sges", 0),
        IntField("current_sge_offset", 0),

        BitField("sqcb0_sq_drained", 0, 1),
        BitField("rsvd_state_flags", 0, 7),

        BitField("priv_oper_enable", 0, 1),
        BitField("in_progress", 0, 1),
        BitField("sqcb0_bktrack_in_progress", 0, 1),
        BitField("frpmr_in_progress", 0, 1),
        BitField("color", 0, 1),
        BitField("fence", 0, 1),
        BitField("li_fence", 0, 1),
        BitField("busy", 0, 1),

        BitField("spec_enable", 0, 1),
        BitField("skip_pt", 0, 1),
        BitField("bktrack_marker_in_progress", 0, 1),
        BitField("congestion_mgmt_type",0, 2),
        BitField("sqcb0_rsvd2", 0, 3),        

        # SQCB1 
        ByteField("pc_offset", 0),
        X3BytesField("cq_id", 0),
        ByteField("sqcb1_p_index4", 0),
        ByteField("c_index4", 0),
        ShortField("sqcb1_rsvd1", 16),

        XIntField("sqcb1_rrq_base_addr", 0),
        ByteField("sqcb1_log_rrq_size", 0),
        BitField("service", 0, 4),
        BitField("rsvd8", 0, 1),
        BitField("sqcb1_log_pmtu", 0xa, 5),
        BitField("err_retry_count", 0, 3),
        BitField("rnr_retry_count", 0, 3),
        BitField("work_not_done_recirc_cnt", 0, 8),

        X3BytesField("sqcb1_tx_psn", 0),
        X3BytesField("sqcb1_ssn", 0),
        ShortField("sqcb1_rsvd2", 0),
        BitField("log_sqwqe_size", 0, 5),
        BitField("pkt_spec_enable", 0, 1),
        BitField("sqcb1_rsvd3", 0, 2),

        XIntField("sqcb1_header_template_addr", 0),
        ByteField("sqcb1_header_template_size", 0),

        ByteField("nxt_to_go_token_id", 0),
        ByteField("token_id", 0),

        X3BytesField("sqcb1_rsvd4", 0),
        X3BytesField("rexmit_psn", 0),
        X3BytesField("msn", 0),

        BitField("credits", 0, 5),
        BitField("congestion_mgmt_type", 0, 2),
        BitField("sqcb1_rsvd5", 0, 1),

        X3BytesField("max_tx_psn", 0),
        X3BytesField("max_ssn", 0),

        ByteField("rrqwqe_num_sges", 0),
        ByteField("rrqwqe_cur_sge_id", 0),
        IntField("rrqwqe_cur_sge_offset", 0),
        BitField("sqcb1_state", 0, 3),
        BitField("sqcb1_priv_oper_enable", 0, 1),
        BitField("sqcb1_sq_drained", 0, 1),
        BitField("sqd_async_notify_enable", 0, 1),
        BitField("sqcb1_rsvd6", 0, 2),
        ByteField("sqcb1_bktrack_in_progress", 0),
        IntField("sqcb1_pd", 0),
        BitField("rrq_spec_cindex", 0, 16),
        BitField("sqcb1_rsvd7", 0, 16),

        #SQCB2
        X3BytesField("dst_qp", 0),
        ByteField("sqcb2_header_template_size", 0),
        XIntField("sqcb2_header_template_addr", 0),

        XIntField("sqcb2_rrq_base_addr", 0),
        BitField("sqcb2_log_rrq_size", 0, 5),
        BitField("log_sq_size", 0, 5),
        BitField("roce_opt_ts_enable", 0, 1),
        BitField("roce_opt_mss_enable", 0, 1),
        BitField("service", 0, 4),

        X3BytesField("lsn_tx", 0),
        X3BytesField("lsn_rx", 0),
        X3BytesField("sqcb2_rexmit_psn", 0),
     
        BitField("last_ack_or_req_ts", 0, 48),
        BitField("err_retry_ctr", 0, 4),
        BitField("rnr_retry_ctr", 0, 4),
        ByteField("rnr_timeout", 0),
        BitField("sqcb2_rsvd1", 0, 2),
        BitField("timer_on", 0, 1),
        BitField("local_ack_timeout", 0, 5),

        X3BytesField("tx_psn", 0),
        X3BytesField("ssn", 0),
        X3BytesField("lsn", 0),
        X3BytesField("wqe_start_psn", 0),

        IntField("imm_data_or_inv_key", 0),

        ShortField("sq_cindex", 0),
        ByteField("p_index4", 0),
        ByteField("sqcb2_c_index4", 0),
        BitField("sqcb2_busy", 0, 1),
        BitField("sqcb2_need_credits", 0, 1),
        BitField("sqcb2_rsvd2", 0, 14),
        BitField("fence", 0, 1),
        BitField("li_fence", 0, 1),
        BitField("fence_done", 0, 1),
        BitField("curr_op_type", 0, 5),

        X3BytesField("exp_rsp_psn", 0),

        ShortField("timestamp", 0),
        BitField("disable_credits", 0, 1),
        BitField("sqcb2_rsvd3", 0, 7),
        X3BytesField("sq_msg_psn", 0),

        #SQCB3
        BitField("sqcb3", 0, 512),

        #SQCB4 - REQ_TX stats
        LongField("tx_num_bytes", 0),
        IntField("tx_num_pkts", 0),
        ShortField("tx_num_send_msgs", 0),
        ShortField("tx_num_write_msgs", 0),
        ShortField("tx_num_read_req_msgs", 0),
        ShortField("tx_num_atomic_fna_msgs", 0),
        ShortField("tx_num_atomic_cswap_msgs", 0),
        ShortField("tx_num_send_msgs_inv_rkey", 0),
        ShortField("tx_num_send_msgs_imm_data", 0),
        ShortField("tx_num_write_msgs_imm_data", 0),
        ShortField("tx_num_pkts_in_cur_msg", 0),
        ShortField("tx_max_pkts_in_any_msg", 0),
        IntField("tx_num_npg_requests", 0),
        ShortField("tx_num_npg_bindmw_t1_req", 0),
        ShortField("tx_num_npg_bindmw_t2_req", 0),
        ShortField("tx_num_npg_frpmr_req", 0),
        ShortField("tx_num_npg_li_req", 0),
        ShortField("tx_num_inline_req", 0),
        ShortField("tx_num_timeout_local_ack", 0),
        ShortField("tx_num_timeout_rnr", 0),
        ShortField("tx_num_sq_drains", 0),
        ShortField("np_cnp_sent", 0),
        ShortField("rp_num_byte_threshold_db", 0),
        BitField("tx_qp_err_disabled", 0, 1),
        BitField("qp_err_dis_flush_rq", 0, 1),
        BitField("qp_err_dis_ud_pmtu", 0, 1),
        BitField("qp_err_dis_ud_fast_reg", 0, 1),
        BitField("qp_err_dis_ud_priv", 0, 1),
        BitField("qp_err_dis_no_dma_cmds", 0, 1),
        BitField("qp_err_dis_lkey_inv_state", 0, 1),
        BitField("qp_err_dis_lkey_inv_pd", 0, 1),
        BitField("qp_err_dis_lkey_rsvd_lkey", 0, 1),
        BitField("qp_err_dis_lkey_access_violation", 0, 1),
        BitField("qp_err_dis_bind_mw_len_exceeded", 0, 1),
        BitField("qp_err_dis_bind_mw_rkey_inv_pd", 0, 1),
        BitField("qp_err_dis_bind_mw_rkey_inv_zbva", 0, 1),
        BitField("qp_err_dis_bind_mw_rkey_inv_len", 0, 1),
        BitField("qp_err_dis_bind_mw_rkey_inv_mw_state", 0, 1),
        BitField("qp_err_dis_bind_mw_rkey_type_disallowed", 0, 1),
        BitField("qp_err_dis_bind_mw_lkey_state_valid", 0, 1),
        BitField("qp_err_dis_bind_mw_lkey_no_bind", 0, 1),
        BitField("qp_err_dis_bind_mw_lkey_zero_based", 0, 1),
        BitField("qp_err_dis_bind_mw_lkey_invalid_acc_ctrl", 0, 1),
        BitField("qp_err_dis_bind_mw_lkey_invalid_va", 0, 1),
        BitField("qp_err_dis_bktrack_inv_num_sges", 0, 1),
        BitField("qp_err_dis_bktrack_inv_rexmit_psn", 0, 1),
        BitField("qp_err_dis_frpmr_fast_reg_not_enabled", 0, 1),
        BitField("qp_err_dis_frpmr_invalid_pd", 0, 1),
        BitField("qp_err_dis_frpmr_invalid_state", 0, 1),
        BitField("qp_err_dis_frpmr_invalid_len", 0, 1),
        BitField("qp_err_dis_frpmr_ukey_not_enabled", 0, 1),
        BitField("qp_err_dis_inv_lkey_qp_mismatch", 0, 1),
        BitField("qp_err_dis_inv_lkey_pd_mismatch", 0, 1),
        BitField("qp_err_dis_inv_lkey_invalid_state", 0, 1),
        BitField("qp_err_dis_inv_lkey_inv_not_allowed", 0, 1),
        BitField("qp_err_dis_table_error", 0, 1),
        BitField("qp_err_dis_phv_intrinsic_error", 0, 1),
        BitField("qp_err_dis_table_resp_error", 0, 1),
        BitField("qp_err_dis_inv_optype", 0, 1),
        BitField("qp_err_dis_err_retry_exceed", 0, 1),
        BitField("qp_err_dis_rnr_retry_exceed", 0, 1),
        BitField("qp_err_dis_rsvd", 0, 26),

        #SQCB5
        LongField("rx_num_bytes", 0),
        IntField("rx_num_pkts", 0),
        IntField("rx_num_read_resp_pkts", 0),
        ShortField("rx_num_read_resp_msgs", 0),
        ShortField("rx_num_feedback", 0),
        ShortField("rx_num_ack", 0),
        ShortField("rx_num_atomic_ack", 0),
        ShortField("rx_num_pkts_in_cur_msg", 0),
        ShortField("rx_num_pkts_in_any_msg", 0),
        BitField("rx_qp_err_disabled", 0, 1),
        BitField("qp_err_dis_rrqlkey_pd_mismatch", 0, 1),
        BitField("qp_err_dis_rrqlkey_inv_state", 0, 1),
        BitField("qp_err_dis_rrqlkey_rsvd_lkey", 0, 1),
        BitField("qp_err_dis_rrqlkey_acc_no_wr_perm", 0, 1),
        BitField("qp_err_dis_rrqlkey_acc_len_lower", 0, 1),
        BitField("qp_err_dis_rrqlkey_acc_len_higher", 0, 1),
        BitField("qp_err_dis_rrqsge_insuff_sges", 0, 1),
        BitField("qp_err_dis_rrqsge_insuff_sge_len", 0, 1),
        BitField("qp_err_dis_rrqsge_insuff_dma_cmds", 0, 1),
        BitField("qp_err_dis_rrqwqe_remote_inv_req_err_rcvd", 0, 1),
        BitField("qp_err_dis_rrqwqe_remote_acc_err_rcvd", 0, 1),
        BitField("qp_err_dis_rrqwqe_remote_oper_err_rcvd", 0, 1),
        BitField("qp_err_dis_table_error", 0, 1),
        BitField("qp_err_dis_phv_intrinsic_error", 0, 1),
        BitField("qp_err_dis_table_resp_error", 0, 1),
        BitField("qp_err_dis_rsvd", 0, 16),
        X3BytesField("sqcb5_recirc_bth_psn", 0),
        ByteField("sqcb5_recirc_bth_opcode", 0),
        BitField("sqcb5_recirc_reason", 0, 4),
        BitField("sqcb5_max_recirc_cnt_err", 0, 1),
        BitField("sqcb5", 0, 219),
    ]

class RdmaCQstate(Packet):
    name = "RdmaCQstate"
    fields_desc = [
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        ShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),

        LEShortField("p_index1", 0),
        LEShortField("c_index1", 0),

        LEShortField("p_index2", 0),
        LEShortField("c_index2", 0),

        LEShortField("proxy_pindex", 0),
        LEShortField("proxy_s_pindex", 0),

        XIntField("pt_base_addr", 0),
        BitField("log_cq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("ring_empty_sched_eval_done", 0, 1),
        X3BytesField("cq_id", 0),
        X3BytesField("eq_id", 0),
        BitField("arm", 0, 1),
        BitField("sarm", 0, 1),

        BitField("wakeup_dpath", 0, 1),
        BitField("color", 0, 1),
        BitField("wakeup_lif", 0, 12),
        BitField("wakeup_qtype", 0, 3),
        BitField("wakeup_qid", 0, 24),
        BitField("wakeup_ring_id", 0, 3),
        BitField("cq_full_hint", 0, 1),

        BitField("cq_full", 0, 1),

        ShortField("pt_pg_index", 0),
        ShortField("pt_next_pg_index", 0),
        BitField("host_addr", 0, 1),
        BitField("is_phy_addr", 0, 1),        
        BitField("pad", 0, 14),

        XLongField("pt_pa", 0),
        XLongField("pt_next_pa", 0),
    ]

class RdmaEQstate(Packet):
    name = "RdmaEQstate"
    fields_desc = [
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        ShortField("pid", 0),

        ShortField("p_index0", 0),
        ShortField("c_index0", 0),

        XLongField("eqe_base_addr", 0),
        IntField("rsvd0", 0),
        X3BytesField("eq_id", 0),
        BitField("log_num_wqes", 0, 5),
        BitField("log_wqe_size", 2, 5),
        BitField("int_enabled", 0, 1),
        BitField("color", 0, 1),
        BitField("rsvd", 0, 28),
        XLongField("int_assert_addr", 0),
        BitField("rsvd1", 0, 192),
    ]

qt_params = {
    #fix the label/program for following entry reflecting txdma params
    'RDMA_AQ': {'state': RdmaAQstate, 'hrings': 1, 'trings': 1, 'has_label':1, 'label': 'rdma_aq_rx_stage0', 'prog': 'rxdma_stage0.bin'},
    'RDMA_SQ': {'state': RdmaSQstate, 'hrings': 1, 'trings': 2, 'has_label':1, 'label': 'rdma_req_rx_stage0', 'prog': 'rxdma_stage0.bin'},
    'RDMA_RQ': {'state': RdmaRQstate, 'hrings': 1, 'trings': 2, 'has_label':1, 'label': 'rdma_resp_rx_stage0', 'prog': 'rxdma_stage0.bin'},
    'RDMA_CQ': {'state': RdmaCQstate, 'hrings': 3, 'trings': 3, 'has_label':0, 'label': 'rdma_cq_rx_stage0', 'prog': 'rxdma_stage0.bin'},
    'RDMA_EQ': {'state': RdmaEQstate, 'hrings': 1, 'trings': 1, 'has_label':0, 'label': '', 'prog': ''},
}

class RdmaQstateObject(object):
    def __init__(self, queue_type, addr, size):
        self.queue_type = queue_type
        self.addr = addr
        self.size = size
        self.proxy_cindex = 0
        if queue_type == 'RDMA_CQ':
            self.proxy_cindex_en = True
        else:
            self.proxy_cindex_en = False
        self.Read()

    def Write(self, debug = True):
        if (GlobalOptions.dryrun): return
        if debug is True:
            logger.info("Writing Qstate @0x%x Type: %s size: %d" % (self.addr, self.queue_type, self.size))
        model_wrap.write_mem_pcie(self.addr, bytes(self.data), len(self.data))

    def WriteWithDelay(self):
        if (GlobalOptions.dryrun): return
        logger.info("Writing Qstate @0x%x Type: %s size: %d with delay" % (self.addr, self.queue_type, self.size))
        model_wrap.write_mem_pcie(self.addr, bytes(self.data), len(self.data))
        # On RTL, write is asyncronous and taking long time to complete
        # Wait until the write succeed otherwise tests may fail in RTL (timing)
        # Read(synchrouns) in loop and wait until the read data is same as what was inteded to write
        # Be aware this logic would break if there is continous change to QStata data in hardware 
        # like spruious schdule count in qstate, etc.
        # Check only 256 bytes for Q state - data in use (no need to check entire Qstata/1024 bytes)
        wdata = self.data[0:256]
        if GlobalOptions.rtl:
            count = 1
            while True:
                self.data = qt_params[self.queue_type]['state'](model_wrap.read_mem(self.addr, self.size))
                rdata = self.data[0:256]
                if rdata == wdata:
                    break
                # Read data is not same as we wrote, so sleep for 1 sec and try again
                time.sleep(1)
                count = count + 1
            logger.info("Qstate Write @0x%x Type: %s size: %d completed after %d secs" % (self.addr, self.queue_type, self.size, count))

    def Read(self, debug = True):
        if (GlobalOptions.dryrun):
            data = bytes(self.size)
            self.data = qt_params[self.queue_type]['state'](data)
            return
        self.data = qt_params[self.queue_type]['state'](model_wrap.read_mem(self.addr, self.size))
        if debug is True:
            logger.info("Read Qstate @0x%x Type: %s size: %d" % (self.addr, self.queue_type, self.size))

    def incr_pindex(self, ring, ring_size):
        assert(ring < 7)
        prev_value = self.get_pindex(ring)
        new_value = ((self.get_pindex(ring) + 1) & (ring_size - 1))
        logger.info("  incr_pindex: pre-val: %d new-val: %d ring_size %d" % (prev_value, new_value, ring_size))
        self.set_pindex(ring, new_value)

    def incr_cindex(self, ring, ring_size):
        assert(ring < 7)
        prev_value = self.get_cindex(ring)
        new_value = ((self.get_cindex(ring) + 1) & (ring_size - 1))
        logger.info("  incr_cindex(%d): pre-val: %d new-val: %d ring_size %d" % (ring, prev_value, new_value, ring_size))
        self.set_cindex(ring, new_value)

    def set_pindex(self, ring, value):
        assert(ring < 7)
        setattr(self.data, 'p_index%d' % ring, value)
        #Avoid writing Qstate/PI to ASIC, and let DB pick up updated PI (0x9)

    def set_cindex(self, ring, value):
        assert(ring < 7)
        setattr(self.data, 'c_index%d' % ring, value)
        if self.proxy_cindex_en == 1 and ring == 0:
           self.proxy_cindex = value
        #CQ will update cindex to HW using doorbell. Do not write to HW
        if self.queue_type != 'RDMA_CQ':
            self.WriteWithDelay()

    def set_ring_base(self, value):
        self.data.ring_base = value
        self.Write()

    def set_ring_size(self, value):
        self.data.ring_size = value
        self.Write()

    def get_pindex(self, ring):
        assert(ring < 7)
        return getattr(self.data, 'p_index%d' % ring)

    def get_cindex(self, ring):
        assert(ring < 7)
        if self.proxy_cindex_en == 1 and ring == 0:
           return self.proxy_cindex
        else:
           return getattr(self.data, 'c_index%d' % ring)

    def get_proxy_cindex(self):
        return getattr(self.data, 'proxy_cindex')

    def reset_proxy_s_pindex(self):
        assert(self.queue_type == 'RDMA_CQ')
        setattr(self.data, 'proxy_s_pindex', getattr(self.data, 'proxy_pindex'))
        self.WriteWithDelay()

    def reset_cindex(self, ring):
        assert(ring < 7)
        self.set_cindex(ring, self.get_pindex(ring))
        if hasattr(self.data, 'cq_full_hint'):
           setattr(self.data, 'cq_full_hint', 0)
        if hasattr(self.data, 'cq_full'):
           setattr(self.data, 'cq_full', 0)
        self.WriteWithDelay()

   # This is depricated. Use RING based ARM instead
   #def ArmCq(self):
   #    assert(self.queue_type == 'RDMA_CQ')
   #    setattr(self.data, 'arm', 1)
   #    self.WriteWithDelay()

    def reset_Arm(self):
        setattr(self.data, 'arm', 0)
        self.WriteWithDelay()

    def reset_sArm(self):
        setattr(self.data, 'sarm', 0)
        self.WriteWithDelay()

    def get_Arm(self):
        return getattr(self.data, 'arm')

    def get_sArm(self):
        return getattr(self.data, 'sarm')

    def set_full_hint(self, ring, num_wqes):
        self.set_pindex(ring, self.get_cindex(ring))
        if hasattr(self.data, 'cq_full_hint'):
           setattr(self.data, 'cq_full_hint', 1)
        #self.incr_cindex(ring, num_wqes)
        logger.info("Setting ring full hint(cindex: %d, pindex: %d) for %s @0x%x" % (self.get_cindex(ring), self.get_pindex(ring), self.queue_type, self.addr))
        self.WriteWithDelay()
        return
        
    def set_full(self, ring, num_wqes):
        if hasattr(self.data, 'cq_full'):
           setattr(self.data, 'cq_full', 1)
        logger.info("Setting ring full for %s @0x%x" % (self.queue_type, self.addr))
        self.WriteWithDelay()

    def set_priv(self):
        setattr(self.data, 'priv_oper_enable', 1)
        if hasattr(self.data, 'sqcb1_priv_oper_enable'):
           setattr(self.data, 'sqcb1_priv_oper_enable', 1)
        self.WriteWithDelay()

    def reset_priv(self):
        setattr(self.data, 'priv_oper_enable', 0)
        if hasattr(self.data, 'sqcb1_priv_oper_enable'):
           setattr(self.data, 'sqcb1_priv_oper_enable', 0)
        self.WriteWithDelay()

    def Show(self, lgh = logger):
        lgh.ShowScapyObject(self.data) 



class RdmaQueueObject(QueueObject):
    def __init__(self):
        super().__init__()
        self._qstate    = None

    def Init(self, queue_type, spec):
        self.queue_type = queue_type
        self.id = queue_type.GetQid()
        # Start qid numbering from 1 for adminq, eq
        if queue_type.GID() == "RDMA_AQ":
            self.id = self.id + 1
        self.qp_e_psn   = 0      #Needed for rx multi QP scale tests to pick next psn
        self.GID(str(self.id))

        self.rings      = objects.ObjectDatabase()
        self.obj_helper_ring = ring.RdmaRingObjectHelper()
        self.obj_helper_ring.Generate(self, spec)
        self.rings.SetAll(self.obj_helper_ring.rings)

        self.Show()

    @property
    def qstate(self):
        if self._qstate is None:
            self._qstate = RdmaQstateObject(queue_type=self.queue_type.GID(), addr=self.GetQstateAddr(), size=self.queue_type.size)
        return self._qstate

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.lif_handle = 0  # HW LIF ID is not known in DOL. Assume it is filled in by hal::LifCreate.
        req_spec.type_num = self.queue_type.type
        req_spec.qid = self.id
        qt_params_entry = qt_params[self.queue_type.GID()]
        qstate = qt_params_entry["state"]()
        qstate.host = qt_params_entry["hrings"]
        qstate.total = qt_params_entry["trings"]
        logger.ShowScapyObject(qstate)
        req_spec.queue_state = bytes(qstate)
        if qt_params_entry["has_label"]:
            req_spec.label.handle = "p4plus"
            req_spec.label.prog_name = qt_params_entry["prog"]
            req_spec.label.label = qt_params_entry["label"]
        #print("End of id: %d " % self.id)

    def GetQstateAddr(self):
        return self.queue_type.GetQstateAddr() + (self.id * self.queue_type.size)

    def ConfigureRings(self):
        self.obj_helper_ring.Configure()
        if not ( GlobalOptions.dryrun or GlobalOptions.cfgonly):
            # Ignore time stamp memory comparison b/w RTL and SW model as that woudl differ
            # Timestamp in SQCB2 at offset of 21 bytes for 6B long
            if self.queue_type.GID() == 'RDMA_SQ':
                model_wrap.eos_ignore_addr(self.GetQstateAddr() + 64 + 64 + 21, 6)

    def Show(self):
        logger.info('Queue: %s' % self.GID())
        logger.info('- type   : %s' % self.queue_type.GID())
        logger.info('- id     : %s' % self.id)

    def SetRingParams(self, ring_id, hw_ring_id, host, nic_resident, mem_handle, address, size, desc_size):
        r = self.rings.Get(ring_id)
        if r is None:
            assert(0)
        r.SetRingParams(hw_ring_id, host, nic_resident, mem_handle, address, size, desc_size)
        return

    def SetRingQpSpecEn(self, ring_id, spec_en):
        r = self.rings.Get(ring_id)
        if r is None:
            assert(0)
        r.SetRingQpSpecEn(spec_en)
        return

class RdmaQueueObjectHelper:
    def __init__(self):
        self.queues = []

    def Generate(self, queue_type, spec):
        for espec in spec.queues:
            for qspec in range(espec.queue.count):
                queue = RdmaQueueObject()
                queue.Init(queue_type, espec.queue)
                self.queues.append(queue)

    def Configure(self):
        for queue in self.queues:
            queue.ConfigureRings()
