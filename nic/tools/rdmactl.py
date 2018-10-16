#!/usr/bin/python2.7
import sys
import os
import argparse
from scapy.all import *
from subprocess import call
import binascii
import time

PATH_PREFIX='/sys/kernel/debug/ionic/'
PATH_SUFFIX='/rdma/aq/1'
DUMP_TYPE_QP=0
DUMP_TYPE_CQ=1
DUMP_TYPE_EQ=2
DUMP_TYPE_PT=3
DUMP_TYPE_KT=4
DUMP_TYPE_AQ=5

DBG_WR_CTRL='dbg_wr_ctrl'
DBG_WR_DATA='dbg_wr_data'
DUMP_DATA='/tmp/hexdump_data'


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

        LEShortField("proxy_pindex", 0),
        BitField("log_wqe_size", 0, 5),
        BitField("log_num_size", 0, 5),
        BitField("rsvd2", 0 , 6),

        XLongField("phy_base_addr", 0),
        X3BytesField("aq_id", 0),
        ByteField("busy", 0),
        X3BytesField("cq_id", 0),
        ByteField("rsvd3", 0),
        XLongField("cqcb_addr", 0),
        BitField("pad", 0, 192),
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
        BitField("pad", 0, 15),

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

class RdmaSQCB0state(Packet):
    name = "RdmaSQCB0state"
    fields_desc = [
        #SQCB0
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
        LEShortField("sqd_cindex", 0),
        LEShortField("rsvd1", 0),

        XIntField("pt_base_addr/sq_hbm_base_addr", 0),
        XIntField("sqcb0_header_template_addr", 0),
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
        BitField("congestion_mgmt_enable",0, 1),
        BitField("local_ack_timeout", 0, 5),
        BitField("ring_empty_sched_eval_done", 0, 1),

        ShortField("spec_sq_cindex", 0),

        XLongField("curr_wqe_ptr", 0),
        IntField("current_sge_offset", 0),
        ByteField("current_sge_id", 0),
        ByteField("num_sges", 0),

        BitField("sqcb0_sq_drained", 0, 1),
        BitField("ac_local_wr", 0, 1),
        BitField("ac_mw_bind", 0, 1),
        BitField("rsvd_state_flags", 0, 5),

        BitField("priv_oper_enable", 0, 1),
        BitField("in_progress", 0, 1),
        BitField("sqcb0_bktrack_in_progress", 0, 1),
        BitField("rsvd_flag", 0, 1),
        BitField("color", 0, 1),
        BitField("fence", 0, 1),
        BitField("li_fence", 0, 1),
        BitField("busy", 0, 1),

        BitField("cb1_busy", 0, 1),
        BitField("need_credits", 0, 1),
        BitField("frpmr_in_progress", 0, 1),
        BitField("rsvd_cb1_flags", 0, 5),
    ]
        
class RdmaSQCB1state(Packet):
    name = "RdmaSQCB1state"
    fields_desc = [
        # SQCB1 
        ByteField("pc_offset", 0),
        X3BytesField("cq_id", 0),
        ShortField("sqcb1_p_index4", 0),
        ShortField("c_index4", 0),

        XIntField("sqcb1_rrq_base_addr", 0),
        ByteField("sqcb1_log_rrq_size", 0),
        BitField("service", 0, 4),
        BitField("congestion_mgmt_enable", 0, 1),
        BitField("sqcb1_log_pmtu", 0xa, 5),
        BitField("err_retry_count", 0, 3),
        BitField("rnr_retry_count", 0, 3),
        BitField("work_not_done_recirc_cnt", 0, 8),

        X3BytesField("sqcb1_tx_psn", 0),
        X3BytesField("sqcb1_ssn", 0),
        X3BytesField("rsvd2", 0),

        XIntField("sqcb1_header_template_addr", 0),
        ByteField("sqcb1_header_template_size", 0),

        ByteField("nxt_to_go_token_id", 0),
        ByteField("token_id", 0),

        X3BytesField("e_rsp_psn", 0),
        X3BytesField("rexmit_psn", 0),
        X3BytesField("msn", 0),

        BitField("credits", 0, 5),
        BitField("rsvd3", 0, 3),

        X3BytesField("max_tx_psn", 0),
        X3BytesField("max_ssn", 0),

        ByteField("rrqwqe_num_sges", 0),
        ByteField("rrqwqe_cur_sge_id", 0),
        IntField("rrqwqe_cur_sge_offset", 0),
        BitField("rrq_in_progress", 0, 1),
        BitField("sqcb1_state", 0, 3),
        BitField("sqcb1_priv_oper_enable", 0, 1),
        BitField("sqcb1_drained", 0, 1),
        BitField("sqd_async_notify_enable", 0, 1),
        BitField("rsvd4", 0, 1),
        ByteField("sqcb1_bktrack_in_progress", 0),
        IntField("sqcb1_pd", 0),
        BitField("rrq_spec_cindex", 0, 16),
        BitField("pad1", 0, 16),
    ]

class RdmaSQCB2state(Packet):
    name = "RdmaSQCB2state"
    fields_desc = [
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

        BitField("sqcb2_in_progress", 0, 1),
        BitField("sqcb2_need_credits", 0, 1),
        BitField("timer_on", 0, 1),
        BitField("local_ack_timeout", 0, 5),

        X3BytesField("tx_psn", 0),
        X3BytesField("ssn", 0),
        X3BytesField("lsn", 0),
        X3BytesField("wqe_start_psn", 0),

        IntField("imm_data_or_inv_key", 0),

        ShortField("sq_cindex", 0),
        ShortField("p_index4", 0),
        ShortField("sqcb2_c_index4", 0),
        BitField("fence", 0, 1),
        BitField("li_fence", 0, 1),
        BitField("fence_done", 0, 1),
        BitField("curr_op_type", 0, 5),

        X3BytesField("exp_rsp_psn", 0),

        ShortField("timestamp", 0),
        BitField("disable_credits", 0, 1),
        BitField("timestamp_echo", 0, 15),
        ShortField("mss", 0),
    ]


class RdmaSQCB3state(Packet):
    name = "RdmaSQCB3state"
    fields_desc = [
        BitField("sqcb3", 0, 512),
    ]


class RdmaReqTxStats(Packet):
    name = "RdmaReqTxStats"
    fields_desc = [
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
        ShortField("tx_num_inline_req", 0),
        ShortField("tx_num_timeout_local_ack", 0),
        ShortField("tx_num_timeout_rnr", 0),
        BitField("sqcb4", 0, 128),
    ]

class RdmaReqRxStats(Packet):
    name = "RdmaReqRxStats"
    fields_desc = [
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
        BitField("sqcb5", 0, 288),
    ]

class RdmaRQCB0state(Packet):
    name = "RdmaRQCB0state"
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

        XIntField("pt_base_addr/rq_hbm_base_addr", 0),
        
        BitField("log_rq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("congestion_mgmt_enable", 0, 1),
        BitField("state", 0, 3),
        BitField("log_rsq_size", 0, 5),
        BitField("serv_type", 0, 3),
        BitField("log_pmtu", 0xa, 5),
        
        XIntField("rsq_base_addr", 0),
    
        X3BytesField("spec_read_rsp_psn", 0),
        BitField("spec_color", 0, 1),
        BitField("drain_in_progress", 0, 1),
        BitField("rsvd", 0, 6),

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
        BitField("rqcb0_rsvd1", 0, 7),
        ShortField("bt_rsq_cindex", 0),
        ByteField("pad", 0),
    ]

class RdmaRQCB1state(Packet):
    name = "RdmaRQCB1state"
    fields_desc = [
        # RQCB1
        ByteField("pc_offset", 0),
        XIntField("pt_base_addr/rq_hbm_base_addr", 0),
        
        BitField("log_rq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("congestion_mgmt_enable", 0, 1),
        BitField("cb1_state", 0, 3),
        BitField("log_rsq_size", 0, 5),
        BitField("serv_type", 0, 3),
        BitField("log_pmtu", 0xa, 5),
        
        XIntField("rsq_base_addr", 0),
    
        IntField("pd", 0),

        XIntField("header_template_addr", 0),

        ByteField("token_id", 0),
        ByteField("nxt_to_go_token_id", 0),
        ByteField("work_not_done_recirc_cnt", 0),                                            
        BitField("srq_enabled", 0, 1),
        BitField("cache", 0, 1),
        BitField("immdt_as_dbell", 0, 1),
        BitField("rq_in_hbm", 0, 1),
        BitField("nak_prune", 0, 1),
        BitField("priv_oper_enable", 0, 1),
        BitField("rqcb1_rsvd0", 0, 2),

        BitField("busy", 0, 1),
        BitField("rqcb1_rsvd1", 0, 7),
        BitField("in_progress", 0, 1),
        BitField("rqcb1_rsvd2", 0, 7),
        LEShortField("spec_cindex", 0),

        X3BytesField("e_psn", 0),
        BitField("rqcb1_rsvd3", 0, 5),
        BitField("ac_remote_wr", 0, 1),
        BitField("ac_remote_rd", 0, 1),
        BitField("ac_remote_atomic", 0, 1),

        X3BytesField("msn", 0),
        ByteField("header_template_size", 0),

        X3BytesField("cq_id", 0),
        ByteField("rqcb1_bt_in_progress", 0),

        ByteField("rsq_pindex", 0),

        XLongField("curr_wqe_ptr", 0),
        IntField("current_sge_offset", 0),
        ByteField("current_sge_id", 0),
        ByteField("num_sges", 0),

        LEShortField("proxy_cindex", 0),
        LEShortField("proxy_pindex", 0),

        X3BytesField("srq_id", 0),
        BitField("rqcb1_pad", 0, 8),
    ]

class RdmaRQCB2state(Packet):
    name = "RdmaRQCB2state"
    fields_desc = [
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

        BitField("rqcb2_pad", 0, 240),
    ]

class RdmaRQCB3state(Packet):
    name = "RdmaRQCB3state"
    fields_desc = [
        # RQCB3
        LongField("wrid", 0),
        XLongField("va", 0),
        IntField("len", 0),
        IntField("r_key", 0),

        IntField("roce_opt_ts_value", 0),
        IntField("roce_opt_ts_echo", 0),

        ShortField("roce_opt_mss", 0),
        BitField("rqcb3_rsvd1", 0, 16),

        BitField("rqcb3_pad", 0, 224),
    ]

class RdmaRespTxStats(Packet):
    name = "RdmaRespTxStats"
    fields_desc = [
        #RQCB4 - RESP_TX stats
        LongField("tx_num_bytes", 0),
        IntField("tx_num_pkts", 0),
        IntField("tx_num_read_resp_pkts", 0),
        IntField("tx_num_acks", 0),
        ShortField("tx_num_read_resp_msgs", 0),
        ShortField("tx_num_atomic_resp_msgs", 0),
        ShortField("tx_num_pkts_in_cur_msg", 0),
        ShortField("tx_max_pkts_in_any_msg", 0),
        BitField("rqcb4", 0, 288),
    ]

class RdmaRespRxStats(Packet):
    name = "RdmaRespRxStats"
    fields_desc = [
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
        BitField("pad", 0, 192),
    ]

class RdmaKeyTableEntry(Packet):
    name = "RdmaKeyTableEntry"
    fields_desc = [
        ByteField("user_key", 0),
        BitField("state", 0, 4),
        BitField("type", 0, 4),
        ByteField("acc_ctrl", 0),
        ByteField("log_page_size", 0),
        IntField("len", 0),
        LongField("base_va", 0),
        IntField("pt_base", 0),
        IntField("pd", 0),
        BitField("rsvd1", 0, 19),
        BitField("override_lif_vld", 0, 1),
        BitField("override_lif", 0, 12),
        ByteField("flags", 0),
        X3BytesField("qp", 0),
        IntField("mr_l_key", 0),
        IntField("mr_cookie", 0),
        BitField("rsvd2", 0, 192)
    ]

class RdmaPageTableEntry(Packet):
    name = "RdmaPageTableEntry"
    fields_desc = [
        LELongField("pa_0", 0),
        LELongField("pa_1", 0),
        LELongField("pa_2", 0),
        LELongField("pa_3", 0),
        LELongField("pa_4", 0),
        LELongField("pa_5", 0),
        LELongField("pa_6", 0),
        LELongField("pa_7", 0),
    ]

def exec_dump_cmd(tbl_type, tbl_index, start_offset, num_bytes):
    cmd_str = "echo -n 'tbl " + str(tbl_type) + " idx " + str(tbl_index) + " post' > " + DBG_WR_CTRL
    #print cmd_str
    os.system(cmd_str)
    time.sleep(1)
    cmd2_str = "xxd -r " + DBG_WR_DATA + " > " + DUMP_DATA
    #print cmd2_str
    os.system(cmd2_str)
    
    with open(DUMP_DATA, "rb") as data_file:
        bin_str = data_file.read()

    #print data
    hex_str = binascii.hexlify(bin_str)
    #print len(bin_str), len(hex_str)
    #print hex_str

    #hex is twice long as bytes
    s = start_offset*2
    e = s + num_bytes*2

    #print s,e

    hex_str = hex_str[s:e]
    #print hex_str

    return binascii.unhexlify(hex_str)


parser = argparse.ArgumentParser()
parser.add_argument('--lif', help='prints info for given lif', type=int)
parser.add_argument('--pcie_id', help='prints info for given ionic pcie id')
grp = parser.add_mutually_exclusive_group()
grp.add_argument('--sqcb0', help='prints sqcb0 state given qid', type=int, metavar='qid')
grp.add_argument('--sqcb1', help='prints sqcb1 state given qid', type=int, metavar='qid')
grp.add_argument('--sqcb2', help='prints sqcb2 state given qid', type=int, metavar='qid')
grp.add_argument('--sqcb3', help='prints sqcb3 state given qid', type=int, metavar='qid')
grp.add_argument('--rqcb0', help='prints rqcb0 state given qid', type=int, metavar='qid')
grp.add_argument('--rqcb1', help='prints rqcb1 state given qid', type=int, metavar='qid')
grp.add_argument('--rqcb2', help='prints rqcb2 state given qid', type=int, metavar='qid')
grp.add_argument('--rqcb3', help='prints rqcb3 state given qid', type=int, metavar='qid')
grp.add_argument('--cqcb',  help='prints cqcb state given qid', type=int, metavar='qid')
grp.add_argument('--eqcb',  help='prints eqcb state given qid', type=int, metavar='qid')
grp.add_argument('--aqcb',  help='prints aqcb state given qid', type=int, metavar='qid')
grp.add_argument('--req_tx_stats', help='prints req_tx stats given qid', type=int, metavar='qid')
grp.add_argument('--req_rx_stats', help='prints req_rx stats given qid', type=int, metavar='qid')
grp.add_argument('--resp_tx_stats', help='prints resp_tx stats given qid', type=int, metavar='qid')
grp.add_argument('--resp_rx_stats', help='prints resp_rx stats given qid', type=int, metavar='qid')
grp.add_argument('--kt_entry', help='prints key table entry given key id', type=int, metavar='key_id')
grp.add_argument('--pt_entry', help='print page table entry given pt offset', type=int, metavar='pt_offset')

args = parser.parse_args()

#print args

if args.lif is None or args.pcie_id is None:
    print 'lif and/or pcie_id is not specified'

path = PATH_PREFIX+args.pcie_id+'/lif'+str(args.lif)+PATH_SUFFIX
#print path
os.chdir(path)

if args.cqcb is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_CQ, args.cqcb, 0, 64)
    cqcb = RdmaCQstate(bin_str)
    cqcb.show()
elif args.eqcb is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_EQ, args.cqcb, 0, 64)
    eqcb = RdmaEQstate(bin_str)
    eqcb.show()
elif args.aqcb is not None:
    #for now qid is ignored for aqcb dump.
    #qstate is dumped for the aq on which this rdmactl command is issued.
    bin_str = exec_dump_cmd(DUMP_TYPE_AQ, 1, 0, 64)
    aqcb = RdmaAQstate(bin_str)
    aqcb.show()
elif args.sqcb0 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.sqcb0, 0, 64)
    sqcb0 = RdmaSQCB0state(bin_str)
    sqcb0.show()
elif args.sqcb1 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.sqcb1, 64, 64)
    sqcb1 = RdmaSQCB1state(bin_str)
    sqcb1.show()
elif args.sqcb2 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.sqcb2, 128, 64)
    sqcb2 = RdmaSQCB2state(bin_str)
    sqcb2.show()
elif args.sqcb3 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.sqcb3, 192, 64)
    sqcb3 = RdmaSQCB3state(bin_str)
    sqcb3.show()
elif args.rqcb0 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.rqcb0, 1024, 64)
    rqcb0 = RdmaRQCB0state(bin_str)
    rqcb0.show()
elif args.rqcb1 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.rqcb1, 1024+64, 64)
    rqcb1 = RdmaRQCB1state(bin_str)
    rqcb1.show()
elif args.rqcb2 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.rqcb2, 1024+128, 64)
    rqcb2 = RdmaRQCB2state(bin_str)
    rqcb2.show()
elif args.rqcb3 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.rqcb3, 1024+192, 64)
    rqcb3 = RdmaRQCB3state(bin_str)
    rqcb3.show()
elif args.req_tx_stats is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.req_tx_stats, 256, 64)
    req_tx_stats = RdmaReqTxStats(bin_str)
    req_tx_stats.show()
elif args.req_rx_stats is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.req_rx_stats, 320, 64)
    req_rx_stats = RdmaReqRxStats(bin_str)
    req_rx_stats.show()
elif args.resp_tx_stats is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.resp_tx_stats, 1024+256, 64)
    resp_tx_stats = RdmaRespTxStats(bin_str)
    resp_tx_stats.show()
elif args.resp_rx_stats is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.resp_rx_stats, 1024+320, 64)
    resp_rx_stats = RdmaRespRxStats(bin_str)
    resp_rx_stats.show()
elif args.kt_entry is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_KT, args.kt_entry, 0, 64)
    kt_entry = RdmaKeyTableEntry(bin_str)
    kt_entry.show()
elif args.pt_entry is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_PT, args.pt_entry, 0, 64)
    pt_entry = RdmaPageTableEntry(bin_str)
    pt_entry.show()
