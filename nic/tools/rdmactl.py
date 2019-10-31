#!/usr/bin/env python2.7
import sys
import os
import argparse
from scapy.all import *
from subprocess import call
import subprocess
import binascii
import time
import telnetlib
import json
import re

pcie_id=None
lif_id=0

CMD_FMT='tbl {tbl} idx {idx} post'
CTRL_FMT=None
DATA_FMT=None
HOST_OS= ''
REMOTE_CMD_FMT='{cmd} -q -o StrictHostKeyChecking=no -o BatchMode=yes {options}'

DUMP_TYPE_QP=0
DUMP_TYPE_CQ=1
DUMP_TYPE_EQ=2
DUMP_TYPE_PT=3
DUMP_TYPE_KT=4
DUMP_TYPE_AQ=5
DUMP_TYPE_LIF=6
AQ_DEBUG_ENABLE=7
AQ_DEBUG_DISABLE=8
DUMP_TYPE_DCQCN_PROFILE=9
DUMP_TYPE_DCQCN_CB=10

DBG_WR_CTRL='dbg_wr_ctrl'
DBG_WR_DATA='dbg_wr_data'
DUMP_DATA='/tmp/hexdump_data'

DMESG_FILE='/tmp/dmesg.txt'

HOST=''
USER=''
CMD_PREFIX=''

def getPwd():
    pwd = os.path.dirname(sys.argv[0])
    pwd = os.path.abspath(pwd)
    return pwd

def sendTelnetCmd(self, cmd, exp, **kwargs):

    if "timeout" in kwargs:
        timeout = kwargs["timeout"]
    else:
        timeout = 30

    self.write(cmd)
    op = self.expect(exp, timeout=timeout)
    return op

def clearLine(console, port):
    tn = telnetlib.Telnet(console)
    logging.info('Clearing line for {0}'.format(console))

    # expect either of two prompts
    res = tn.expect(["Username: ", "Password: "], timeout=5)
    if (res[0] == 0):
        sendTelnetCmd(tn, 'admin\n', ['Password: '])

    sendTelnetCmd(tn, 'N0isystem$\n', ['#'])
    cmd = 'clear line ' + str(port) + '\n'
    sendTelnetCmd(tn, cmd, ['confirm'])
    sendTelnetCmd(tn, 'y\n', ['#'])

    tn.write("exit\n")
    tn.close()
    logging.info('Done')

def telnetToConsole(console, port):
    logging.info('telnet to {0} {1}'.format(console, port))
    tn = telnetlib.Telnet(console, port)
    time.sleep(2)
    tn.write("\n")
    op = tn.expect(['capri login: ', '#'], timeout=5)
    logging.info(op[2])
    if op[0] == 0:
        tn.write("root\n")
        op = tn.expect(['Password: '], timeout=5)
        logging.info(op[2])
        tn.write("pen123\n")
        op = tn.expect(['#'], timeout=5)
        logging.info(op[2])

    tn.write("ls -lrt /\n")
    op = tn.expect(['#'], timeout=5)
    logging.info(op[2])
    return tn

def setHostOS():

    global HOST_OS

    if HOST != '':
        options = USER + '@' + HOST + ' -t uname'
        cmd_str = REMOTE_CMD_FMT.format(
                  cmd = 'ssh', options = options)

        op = os.popen3(cmd_str)
        out = op[1].read()

        if ("Linux" in out):
            HOST_OS = 'Linux'
        elif ("FreeBSD" in out):
            HOST_OS = 'FreeBSD'
        else:
            print "ERROR: Failed to login to " + HOST + " as " + USER + ". Please setup passwordless login and try again." 
            exit()
    else:
        if sys.platform.startswith('freebsd'):
            HOST_OS = 'FreeBSD'
        else:
            HOST_OS = 'Linux'

class RdmaAQCB0state(Packet):
    name = "RdmaAQCB0state"
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
        LEShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),

        IntField("map_count_completed", 0),
        ByteField("first_pass", 0),
        ByteField("token_id", 0),

        BitField("rsvd1", 0, 24),
        BitField("debug", 0, 8),
        
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
    ]


class RdmaAQCB1state(Packet):
    name = "RdmaAQCB1state"
    fields_desc = [
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
        LEShortField("pid", 0),

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
        LEShortField("pid", 0),

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
        ByteField("sqcb0_rsvd1", 0),

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
        BitField("congestion_mgmt_enable",0, 1),
        BitField("local_ack_timeout", 0, 5),
        BitField("ring_empty_sched_eval_done", 0, 1),

        ShortField("spec_sq_cindex", 0),

        XLongField("curr_wqe_ptr", 0),
        X3BytesField("spec_msg_psn", 0),
        X3BytesField("msg_psn", 0),

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
        BitField("sqcb0_rsvd2", 0, 5),
    ]
        
class RdmaSQCB1state(Packet):
    name = "RdmaSQCB1state"
    fields_desc = [
        # SQCB1 
        ByteField("pc_offset", 0),
        X3BytesField("cq_id", 0),
        ByteField("sqcb1_p_index4", 0),
        ByteField("c_index4", 0),
        ShortField("sqcb1_rsvd1", 16),

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
        BitField("sqcb1_rsvd5", 0, 3),

        X3BytesField("max_tx_psn", 0),
        X3BytesField("max_ssn", 0),

        ByteField("rrqwqe_num_sges", 0),
        ByteField("rrqwqe_cur_sge_id", 0),
        IntField("rrqwqe_cur_sge_offset", 0),
        BitField("sqcb1_state", 0, 3),
        BitField("sqcb1_priv_oper_enable", 0, 1),
        BitField("sqcb1_drained", 0, 1),
        BitField("sqd_async_notify_enable", 0, 1),
        BitField("sqcb1_rsvd6", 0, 2),
        ByteField("sqcb1_bktrack_in_progress", 0),
        IntField("sqcb1_pd", 0),
        BitField("rrq_spec_cindex", 0, 16),
        BitField("sqcb1_rsvd7", 0, 16),
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
        BitField("sqcb1_rsvd1", 0, 2),
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
        ShortField("tx_num_npg_local_inv_req", 0),
        ShortField("tx_num_inline_req", 0),
        ShortField("tx_num_timeout_local_ack", 0),
        ShortField("tx_num_timeout_rnr", 0),
        ShortField("tx_num_sq_drains", 0),
        ShortField("np_cnp_sent", 0),
        ShortField("rp_num_byte_threshold_db", 0),
        BitField("qp_err_disabled", 0, 1),
        BitField("qp_err_dis_flush_rq", 0, 1),
        BitField("qp_err_dis_ud_pmtu", 0, 1),
        BitField("qp_err_dis_ud_fast_reg", 0, 1),
        BitField("qp_err_dis_ud_priv", 0, 1),
        BitField("qp_err_dis_no_dma_cmds", 0, 1),
        BitField("qp_err_dis_lkey_inv_state", 0, 1),
        BitField("qp_err_dis_lkey_inv_pd", 0, 1),
        BitField("qp_err_dis_lkey_rsvd_lkey", 0, 1),
        BitField("qp_err_dis_lkey_access_violation", 0, 1),
        BitField("qp_err_dis_bind_mw_rkey_inv_pd", 0, 1),
        BitField("qp_err_dis_bind_mw_len_exceeded", 0, 1),
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
        ShortField("rx_max_pkts_in_any_msg", 0),

        BitField("qp_err_disabled", 0, 1),
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
        LEShortField("pid", 0),

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
        BitField("congestion_mgmt_enable", 0, 1),
        BitField("cb0_state", 0, 3),
        BitField("log_rsq_size", 0, 5),

        XByteField("phy_base_addr_8", 0),
        XIntField("pt_base_addr/rq_hbm_base_addr/phy_base_addr_32", 0),

        XIntField("rsq_base_addr", 0),

        X3BytesField("spec_read_rsp_psn", 0),
        BitField("spec_color", 0, 1),
        BitField("drain_in_progress", 0, 1),
        BitField("skip_pt", 0, 1),
        BitField("prefetch_en", 0, 1),
        BitField("rsvd", 0, 4),

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
        BitField("serv_type", 0, 3),
        BitField("log_pmtu", 0xa, 5),
    ]

class RdmaRQCB1state(Packet):
    name = "RdmaRQCB1state"
    fields_desc = [
        # RQCB1
        ByteField("pc_offset", 0),

        XIntField("rsq_base_addr/q_key", 0),

        XByteField("phy_base_addr_8", 0),
        XIntField("pt_or_rq_hbm_or_prefetch_base_addr", 0),

        BitField("log_rq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("congestion_mgmt_enable", 0, 1),
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
        BitField("rqcb1_rsvd1", 0, 7),
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

        LEShortField("proxy_cindex", 0),
        LEShortField("prefetch_pindex", 0),
        LEShortField("prefetch_proxy_cindex", 0),
        XIntField("prefetch_base_addr", 0),
        BitField("log_num_prefetch_wqes", 0, 5),
        BitField("rqcb2_rsvd3", 0, 3),

        ShortField("prefetch_buff_index", 0),
        BitField("checkout_done", 0, 1),
        BitField("prefetch_init_done", 0, 1),
        BitField("rqcb2_pad", 0, 134),
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

        IntField("num_pkts_in_curr_msg", 0),
        IntField("dma_len", 0),

        BitField("rqcb3_pad", 0, 160),
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
        BitField("qp_err_disabled", 0, 1),
        BitField("qp_err_dis_rsvd_rkey_err", 0, 1),
        BitField("qp_err_dis_rkey_state_err", 0, 1),
        BitField("qp_err_dis_rkey_pd_mismatch", 0, 1),
        BitField("qp_err_dis_rkey_acc_ctrl_err", 0, 1),
        BitField("qp_err_dis_rkey_va_err", 0, 1),
        BitField("qp_err_dis_type2a_mw_qp_mismatch", 0, 1),
        BitField("qp_err_dis_resp_rx", 0, 1),

        BitField("rqcb4", 0, 32),
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
        BitField("qp_err_dis_wr_only_zero_len_err", 0, 1),
        BitField("qp_err_dis_unaligned_atomic_va_err", 0, 1),
        BitField("qp_err_dis_access_err", 0, 1),
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

class RdmaDcqcnProfile(Packet):
    name = "RdmaDcqcnProfile"
    fields_desc = [
        BitField("pad", 0, 64),
        ByteField("np_incp_802p_prio", 0),
        ByteField("np_cnp_dscp", 0),
        BitField("np_rsvd", 0, 32),
        ShortField("rp_dce_tcp_g", 0),
        IntField("rp_dce_tcp_rtt", 0),
        IntField("rp_rate_reduce_monitor_period", 0),
        IntField("rp_rate_to_set_on_first_cnp", 0),
        IntField("rp_min_rate", 0),
        ShortField("rp_initial_alpha_value", 0),
        ByteField("rp_gd", 0),
        ByteField("rp_min_dec_fac", 0),
        BitField("rp_clamp_flags", 0, 8),
        ByteField("rp_threshold", 0),
        ShortField("rp_time_reset", 0),
        IntField("rp_qp_rate", 0),
        IntField("rp_byte_reset", 0),
        IntField("rp_ai_rate", 0),
        IntField("rp_hai_rate", 0),
        BitField("rp_rsvd", 0, 64),
    ]

class RdmaDcqcnCB(Packet):
    name = "RdmaDcqcnCB"
    fields_desc = [
        BitField("last_cnp_timestamp", 0, 48),
        IntField("byte_counter_thr",0),
        ByteField("rsvd1",0),
        BitField("sq_msg_psn", 0, 24),
        IntField("rate_enforced", 0),
        IntField("target_rate",0),
        BitField("alpha_value", 0, 16),
        IntField("cur_byte_counter",0),
        BitField("byte_counter_exp_cnt",0, 16),
        BitField("timer_exp_cnt", 0, 16),
        BitField("num_alpha_exp_cnt", 0, 16), 
        ByteField("num_cnp_rcvd", 0),
        ByteField("num_cnp_processed", 0),
        BitField("max_rate_reached", 0, 1),
        BitField("log_sq_size", 0, 5),
        BitField("resp_rl_failure", 0, 1),
        BitField("rsvd0", 0, 1),
        BitField("last_sched_timestamp", 0, 48),
        BitField("delta_tokens_last_sched", 0, 16),
        BitField("cur_avail_tokens", 0, 48),
        BitField("token_bucket_size", 0, 48),
        BitField("sq_cindex", 0, 16),
        ByteField("num_sched_drop", 0),
        BitField("cur_timestamp", 0, 32),
    ]

class RdmaKeyTableEntry(Packet):
    name = "RdmaKeyTableEntry"
    fields_desc = [
        XByteField("user_key", 0),
        BitField("state", 0, 4),
        BitField("type", 0, 4),
        XByteField("acc_ctrl", 0),
        ByteField("log_page_size", 0),
        LongField("len", 0),
        XLongField("base_va", 0),
        XLongField("pt_base", 0),
        IntField("pd", 0),
        BitField("host_addr", 0, 1),
        BitField("override_lif_vld", 0, 1),
        BitField("override_lif", 0, 12),
        BitField("is_phy_addr", 0, 1),
        BitField("rsvd1", 0, 17),
        XByteField("flags", 0),
        X3BytesField("qp", 0),
        XIntField("mr_l_key", 0),
        XIntField("mr_cookie", 0),
        IntField("num_pt_entries_rsvd", 0),
        BitField("rsvd2", 0, 96)
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

class LifStats(Packet):
    name = "LifStats"
    fields_desc = [
        BitField("rsvd_eth_reg", 0, 3072),
        LELongField("tx_rdma_ucast_bytes", 0),
        LELongField("tx_rdma_ucast_packets", 0),
        LELongField("tx_rdma_mcast_bytes", 0),
        LELongField("tx_rdma_mcast_packets", 0),
        LELongField("tx_rdma_cnp_packets", 0),
        BitField("rsvd_rdma_tx", 0, 192),

        LELongField("rx_rdma_ucast_bytes", 0),
        LELongField("rx_rdma_ucast_packets", 0),
        LELongField("rx_rdma_mcast_bytes", 0),
        LELongField("rx_rdma_mcast_packets", 0),
        LELongField("rx_rdma_cnp_packets", 0),
        LELongField("rx_rdma_ecn_packets", 0),
        BitField("rsvd_rdma_rx", 0, 128),

        BitField("rsvd_eth_dbg", 0, 2048),

        LELongField("rdma_req_rx_pkt_seq_err", 0),
        LELongField("rdma_req_rx_rnr_retry_err", 0),
        LELongField("rdma_req_rx_remote_access_err", 0),
        LELongField("rdma_req_rx_remote_inv_req_err", 0),
        LELongField("rdma_req_rx_remote_oper_err", 0),
        LELongField("rdma_req_rx_implied_nak_seq_err", 0),
        LELongField("rdma_req_rx_cqe_err", 0),
        LELongField("rdma_req_rx_cqe_flush_err", 0),

        LELongField("rdma_req_rx_dup_responses", 0),
        LELongField("rdma_req_rx_invalid_packets", 0),
        LELongField("rdma_req_tx_local_access_err", 0),
        LELongField("rdma_req_tx_local_oper_err", 0),
        LELongField("rdma_req_tx_memory_mgmt_err", 0),
        LELongField("rdma_req_tx_retry_exceeded_err", 0),
        BitField("rsvd_rdma_dbg_req", 0, 128),

        LELongField("rdma_resp_rx_dup_requests", 0),
        LELongField("rdma_resp_rx_out_of_buffer", 0),
        LELongField("rdma_resp_rx_out_of_seq_pkts", 0),
        LELongField("rdma_resp_rx_cqe_err", 0),
        LELongField("rdma_resp_rx_cqe_flush_er", 0),
        LELongField("rdma_resp_rx_local_len_err", 0),
        LELongField("rdma_resp_rx_inv_request_err", 0),
        LELongField("rdma_resp_rx_local_qp_oper_err", 0),

        LELongField("rdma_resp_rx_out_of_atomic_resource", 0),
        LELongField("rdma_resp_tx_pkt_seq_err", 0),
        LELongField("rdma_resp_tx_remote_inv_req_err", 0),
        LELongField("rdma_resp_tx_remote_access_err", 0),
        LELongField("rdma_resp_tx_remote_oper_err", 0),
        LELongField("rdma_resp_tx_rnr_retry_err", 0),
        BitField("rsvd_rdma_dbg_resp", 0, 128),
    ]

class RdmaAQWqeBase(Packet):
    name = "RdmaAQWqeBase"
    fields_desc = [
        ByteEnumField("op", 0, {0:'NOP', 1:'Create CQ', 2:'Create QP', 3:'Reg MR', 6:'Dereg MR', 9:'Modify QP', 11:'Destroy QP', 12:'Stats Dump', 15:'Modify DCQCN'}),
        ByteField("type_state", 0),
        XLEShortField("dbid_flags", 0),
        IntField("id_ver", 0),
    ]

class RdmaAQWqeStats(Packet):
    name = "RdmaAqWqeStats"
    fields_desc = [
        XLELongField("dma_addr", 0),
        LEIntField("length", 0),
        XBitField("rsvd", 0, 352),
    ]
class RdmaAQWqeAH(Packet):
    name = "RdmaAqWqeAH"
    fields_desc = [
        XLELongField("dma_addr", 0),
        LEIntField("length", 0),
        LEIntField("pd_id", 0),
        XBitField("rsvd", 0, 320),
    ]
class RdmaAQWqeMR(Packet):
    name = "RdmaAqWqeMR"
    fields_desc = [
        XLELongField("va", 0),
        XLELongField("length", 0),
        LEIntField("pd_id", 0),
        XBitField("rsvd", 0, 144),
        ByteField("dir_size_log2", 0),
        ByteField("page_size_log2", 0),
        LEIntField("tbl_index", 0),
        LEIntField("map_count", 0),
        XLELongField("dma_addr", 0),
    ]
class RdmaAQWqeCQ(Packet):
    name = "RdmaAQWqeCQ"
    fields_desc = [
        LEIntField("eq_id", 0),            
        ByteField("depth_log2", 0),       
        ByteField("stride_log2", 0),      
        ByteField("dir_size_log2_rsvd", 0),
        ByteField("page_size_log2", 0),   
        XBitField("rsvd", 0, 256),
        LEIntField("tbl_index", 0),
        LEIntField("map_count", 0),
        XLELongField("dma_addr", 0),
    ]

class RdmaAQWqeCQP(Packet):
    name = "RdmaAQWqeCQP"
    fields_desc = [
        LEIntField("pd_id", 0),               
        XIntField("priv_flags", 0),          
        LEIntField("sq_cq_id", 0),            
        ByteField("sq_depth_log2", 0),       
        ByteField("sq_stride_log2", 0),      
        ByteField("sq_dir_size_log2_rsvd", 0),
        ByteField("sq_page_size_log2", 0),   
        LEIntField("sq_tbl_index_xrcd_id", 0),
        LEIntField("sq_map_count", 0),        
        XLELongField("sq_dma_addr", 0),         
        LEIntField("rq_cq_id", 0),            
        ByteField("rq_depth_log2", 0),       
        ByteField("rq_stride_log2", 0),      
        ByteField("rq_dir_size_log2_rsvd", 0),
        ByteField("rq_page_size_log2", 0),   
        LEIntField("rq_tbl_index_srq_id", 0), 
        LEIntField("rq_map_count", 0),        
        XLELongField("rq_dma_addr", 0),         
    ]

class RdmaAQWqeMQP(Packet):
    name = "RdmaAQWqeMQP"
    fields_desc = [
        XIntField("attr_mask", 0),      
        XIntField("access_flags", 0),   
        LEIntField("rq_psn", 0),         
        LEIntField("sq_psn", 0),         
        LEIntField("qkey_dest_qpn", 0),  
        LEIntField("rate_limit_kbps", 0),
        ByteField("pmtu", 0),           
        ByteField("retry", 0),          
        ByteField("rnr_timer", 0),      
        ByteField("retry_timeout", 0),  
        ByteField("rsq_depth", 0),      
        ByteField("rrq_depth", 0),      
        LEShortField("pkey_id", 0),        
        LEIntField("ah_id_len", 0),      
        XBitField("rsvd", 0, 32),        
        LEIntField("rrq_index", 0),      
        LEIntField("rsq_index", 0),      
        XLELongField("dma_addr", 0),       
    ]

class RdmaAQWqeQuery(Packet):
    name = "RdmaAQWqeQuery"
    fields_desc = [
        XBitField("rsvd", 0, 384),        
        XLELongField("dma_addr", 0),       
    ]

class RdmaAQWqeModDcqcn(Packet):
    name = "RdmaAQWqeModDcqcn"
    fields_desc = [
        ByteField("np_incp_802p_prio", 0),
        ByteField("np_cnp_dscp", 0),
        BitField("np_rsvd", 0, 48),
        ShortField("rp_initial_alpha_value", 0),
        ShortField("rp_dce_tcp_g", 0),
        IntField("rp_dce_tcp_rtt", 0),
        IntField("rp_rate_reduce_monitor_period", 0),
        IntField("rp_rate_to_set_on_first_cnp", 0),
        IntField("rp_min_rate", 0),
        ByteField("rp_gd", 0),
        ByteField("rp_min_dec_fac", 0),
        BitField("rp_clamp_flags", 0, 8),
        ByteField("rp_threshold", 0),
        IntField("rp_time_reset", 0),
        IntField("rp_byte_reset", 0),
        IntField("rp_ai_rate", 0),
        IntField("rp_hai_rate", 0),
        BitField("rp_rsvd", 0, 64),
    ]

bind_layers(RdmaAQWqeBase, RdmaAQWqeCQ, op=1)
bind_layers(RdmaAQWqeBase, RdmaAQWqeCQP, op=2)
bind_layers(RdmaAQWqeBase, RdmaAQWqeMR, op=3)
bind_layers(RdmaAQWqeBase, RdmaAQWqeMQP, op=9)
bind_layers(RdmaAQWqeBase, RdmaAQWqeQuery, op=10)
bind_layers(RdmaAQWqeBase, RdmaAQWqeStats, op=12)
bind_layers(RdmaAQWqeBase, RdmaAQWqeAH, op=13)
bind_layers(RdmaAQWqeBase, RdmaAQWqeModDcqcn, op=15)

class RdmaAqCqe(Packet):
    name = "RdmaAqCqe"
    fields_desc = [
        ShortField("cmd_idx", 0),
        ByteField("cmd_op", 0),
        BitField("rsvd", 0, 136),
        LEShortField("old_sq_cindex", 0),
        LEShortField("old_rq_cq_cindex", 0),
        IntField("status_length", 0),
        XIntField("qid_type_flags", 0),
    ]

class RdmaRsqReadstate(Packet):
    name = "RdmaRsqReadstate"
    fields_desc = [
        BitField("read_or_atomic", 0, 1),
        BitField("rsvd1", 0, 7),
        X3BytesField("psn", 0),
        XLongField("rsvd2", 0),
        IntField("r_key", 0),
        XLongField("va", 0),
        IntField("len", 0),
        XIntField("offset", 0),
        BitField("pad", 0, 256),
    ]

class RdmaRsqAtomicstate(Packet):
    name = "RdmaRsqAtomicstate"
    fields_desc = [
        BitField("read_or_atomic", 0, 1),
        BitField("rsvd1", 0, 7),
        X3BytesField("psn", 0),
        XLongField("rsvd2", 0),
        IntField("r_key", 0),
        XLongField("va", 0),
        XLongField("orig_data", 0),
        BitField("pad", 0, 256),
    ]

class RdmaRrqReadstate(Packet):
    name = "RdmaRrqReadstate"
    fields_desc = [
        BitField("read_rsp_or_atomic", 0, 1),
        BitField("rsvd", 0, 7),
        ByteField("num_sges", 0),
        X3BytesField("psn", 0),
        X3BytesField("msn", 0),
        IntField("len", 0),
        XLongField("wqe_sge_list_addr", 0),
        BitField("pad", 0, 96),
    ]

class RdmaRrqAtomicstate(Packet):
    name = "RdmaRrqAtomicstate"
    fields_desc = [
        BitField("read_rsp_or_atomic", 0, 1),
        BitField("rsvd", 0, 7),
        ByteField("num_sges", 0),
        X3BytesField("psn", 0),
        X3BytesField("msn", 0),
        XLongField("va", 0),
        IntField("len", 0),
        IntField("l_key", 0),
        ByteField("op_type", 0),
        BitField("pad", 0, 56),
    ]

def parse_dmesg():

    if args.dmesg:
        os.system("dmesg > {}".format(DMESG_FILE))
        file_str = DMESG_FILE
    else:
        file_str = args.dmesg_file

    with open (file_str) as fp:
        line = fp.readline()
        while line:
            line = line.rstrip()
            print line
            if "post admin" in line:
                tmp_str = ""
                for i in range(4):
                    line = fp.readline().rstrip('\n')
                    print line
                    if HOST_OS == 'FreeBSD':
                        m = re.match(r"(.*wqe\W+\[.*?\])([ A-Fa-f0-9]+).*", line)
                    else:
                        m = re.match(r"(.*wqe.*:)([ A-Fa-f0-9]+).*", line)
                    if m is not None:
                        tmp_str = tmp_str + m.group(2)
                    else:
                        line = fp.readline().rstrip('\n')
                        break
                tmp_str = tmp_str.replace(" ","")
                if len(tmp_str) == 128:
                    aq_wqe = RdmaAQWqeBase(binascii.unhexlify(tmp_str))
                    aq_wqe.show()
                line = fp.readline()
            elif "poll cq" in line:
                line = fp.readline().rstrip('\n')
                if "poll eq" in line or "eqe" in line:
                    print line
                    line = fp.readline().rstrip('\n')
                if "poll eq" in line or "eqe" in line:
                    print line
                    line = fp.readline().rstrip('\n')
                tmp_str = ""
                i = 0
                for i in range(2):
                    print line
                    if HOST_OS == 'FreeBSD':
                        m = re.match(r"(.*cqe\W+\[.*?\])([ A-Fa-f0-9]+).*", line)
                    else:
                        m = re.match(r"(.*cqe.*:)([ A-Fa-f0-9]+).*", line)
                    if m is not None:
                        tmp_str = tmp_str + m.group(2)
                    else:
                        line = fp.readline().rstrip('\n')
                        break
                    line = fp.readline()
                tmp_str = tmp_str.replace(" ","")
                if len(tmp_str) == 64:
                    aq_cqe = RdmaAqCqe(binascii.unhexlify(tmp_str))
                    aq_cqe.show()
            else:
                line = fp.readline()

def show_slice(parse, bin_str, start_offset, num_bytes):
    parse(bin_str[start_offset: start_offset + num_bytes]).show()

def exec_dump_cmd(tbl_type, tbl_index, start_offset, num_bytes, aqid = 1):
    if HOST != '':
        options = USER + '@' + HOST + ' " '
        prefix = REMOTE_CMD_FMT.format(
                 cmd = 'ssh', options = options)
        suffix = ' " '
    else:
        prefix = ''
        suffix = ''

    cmd_str = prefix + CTRL_FMT.format(
            lif = lif_id, pci = pcie_id,
            tbl = tbl_type, idx = tbl_index, aqid = aqid) + suffix

    #print cmd_str
    if os.system(cmd_str):
        return '\0'*num_bytes

    #time.sleep(1)

    cmd2_str = prefix + DATA_FMT.format(
            lif = lif_id, pci = pcie_id, aqid = aqid,
            out = DUMP_DATA) + suffix

    #print cmd2_str
    if os.system(cmd2_str):
        return '\0'*num_bytes

    # if running from an external server, copy DUMP_DATA
    if HOST != '':
        options = USER + '@' + HOST + ':' + DUMP_DATA + ' ' + DUMP_DATA
        cmd_str = REMOTE_CMD_FMT.format(
                  cmd = 'scp', options = options)
        ret = os.system(cmd_str)
        if (ret != 0):
            print("ERROR: Failed to copy DUMP_DATA from {0}".format(HOST))
            exit()

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

def getLif(tn, devname):
    cmd = 'halctl show lif | grep ' + devname + ' | cut -d \' \' -f 1\n'
    tn.write(cmd)
    op = tn.expect(['#'], timeout=5)
    op_list = op[2].split('\n')
    return op_list[1].rstrip()

def getQBaseAddr(tn, lif, type):

    # start capview
    cmd = 'capview\n'
    tn.write(cmd)
    op = tn.expect(['>'], timeout=5)

    # read lif_qstate
    cmd = 'read pr_psp_dhs_lif_qstate_map_entry[' + lif + ']\n'
    tn.write(cmd)
    op = tn.expect(['>'], timeout=5)
    op_list = op[2].split('\n')

    while(True):
        if "Fields:" in op_list[0]:
            break
        else:
            op_list.pop(0)

    m = re.search('qstate_base:.*(0x.*)', op_list[-3])
    base_addr = int(m.group(1), 16) << 12

    for i in range(0, 4):
        start = 13 + (2*i)

        # find the size
        m = re.search('size.*(0x.*)', op_list[start])
        size = 32 * (1 << int(m.group(1), 16))

        # find the length
        m = re.search('length.*(0x.*)', op_list[start+1])
        length = 1 << int(m.group(1), 16)

        # type 0: sq_base_addr
        if (type == 0):
            if (i > 0):
                base_addr += (size * length)

        elif (type == 1):
            base_addr += (size * length)

    # simulate Ctrl-D to quit capview
    time.sleep(1)
    tn.write('\x04\r\n')
    op = tn.expect(['#'], timeout=5)
    return hex(base_addr)

def getLifFromLog(tn, devname):

    cmd = 'grep -r rdma_lif /var/log/nicmgr.log | grep rq_base | cut -d \' \' -f 5\n'
    tn.write(cmd)
    op = tn.expect(['#'], timeout=5)
    op_list = op[2].split('\n')

    if (len(op_list) == 2):
        print("ERROR: Failed to get LIF index from the NIC")
        exit()

    if devname == 'eth0':
        m = re.search('lif-(.*):', op_list[1])
    elif devname == 'eth1':
        m = re.search('lif-(.*):', op_list[2])

    return m.group(1)

def getQBaseAddrFromLog(tn, lif, type):

    if type == 0:
        base_type = 'sq_base'
    else:
        base_type = 'rq_base'

    cmd = 'grep -r rdma_lif /var/log/nicmgr.log | grep lif-' + lif + ' | grep ' + base_type + ' | cut -d \' \' -f 7\n'
    # get SQ/RQ base address
    tn.write(cmd)
    op = tn.expect(['#'], timeout=5)
    m = re.search('(0x.*)', op[2])

    return m.group(1).rstrip()

def getCbData(tn, addr, offset):

    # dump 64 bytes
    addr = str(hex(addr + offset))
    #print(addr)
    cmd = '/platform/bin/eth_dbgtool memrd ' + addr + ' 64\n'
    tn.write(cmd)
    op = tn.expect(['#'], timeout=5)
    op_list = op[2].split('\n')
    data = ''
    for i in range(1, 5):
        line = op_list[i].split(' ')
        for j in range(2, 18):
            data += line[j]
    #print(data)

    # format the data
    cb_str = data.decode("hex")
    return cb_str

def printQpHeader(qid):

    print("QP num: {0}".format(qid))
    print("==============")
    return

parser = argparse.ArgumentParser()
parser.add_argument('--dmesg', help='parse dmesg and parse rdma adminq wqes/cqes', action='store_true', default=False)
parser.add_argument('--dmesg_file', help='parse dmesg from file and parse rdma adminq wqes/cqes')
parser.add_argument('--DEVNAME', help='prints info for given rdma device')
parser.add_argument('--offline', help='prints rdma per queue state through the console when the host is not accessible', action='store_true', default=False)
parser.add_argument('--user', help='username to login to the host, root is the default', type=str, default='root')
parser.add_argument('--host', help='name of the host where Naples is present', type=str)
parser.add_argument('--num_qps', help='number of QPs', type=int)
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
grp.add_argument('--aqcb0', help='prints aqcb0 state given qid', type=int, metavar='qid')
grp.add_argument('--aqcb1', help='prints aqcb1 state given qid', type=int, metavar='qid')
grp.add_argument('--req_tx_stats', help='prints req_tx stats given qid', type=int, metavar='qid')
grp.add_argument('--req_rx_stats', help='prints req_rx stats given qid', type=int, metavar='qid')
grp.add_argument('--resp_tx_stats', help='prints resp_tx stats given qid', type=int, metavar='qid')
grp.add_argument('--resp_rx_stats', help='prints resp_rx stats given qid', type=int, metavar='qid')
grp.add_argument('--dcqcn_profile', help='prints dcqcn config for profile number', type=int, metavar='profile')
grp.add_argument('--dcqcn_cb', help='prints dcqcn state for RC queue pair', type=int, metavar='qid')
grp.add_argument('--kt_entry', help='prints key table entry given key id', type=int, metavar='key_id')
grp.add_argument('--pt_entry', help='print page table entry given pt offset', type=int, metavar='pt_offset')
grp.add_argument('--lif_stats', help='prints rdma LIF statistics', action='store_true', default=False)
grp.add_argument('--q_stats', help='prints rdma per queue statistics', type=int, metavar='qid')
grp.add_argument('--q_state', help='prints rdma per queue state', type=int, metavar='qid')
grp.add_argument('--rsq', help='prints rdma rsq entries', type=int, metavar='qid')
grp.add_argument('--rrq', help='prints rdma rrq entries', type=int, metavar='qid')
grp.add_argument('--aq_debug_enable', help='Enable AQ captrace', type=int, metavar='qid')
grp.add_argument('--aq_debug_disable', help='Disable AQ captrace', type=int, metavar='qid')

args = parser.parse_args()

USER = args.user
if args.host is not None:
    HOST = args.host

setHostOS()

if HOST_OS == 'FreeBSD':
    CTRL_FMT='sysctl dev.ionic.{lif}.rdma_dbg.aq.{aqid}.dbg_wr_ctrl=\'' + CMD_FMT + '\' > /dev/null'
    DATA_FMT='sysctl -bn dev.ionic.{lif}.rdma_dbg.aq.{aqid}.dbg_wr_data > {out}'
else:
    CTRL_FMT='echo -n \'' + CMD_FMT + '\' > /sys/kernel/debug/ionic/{pci}/lif{lif}/rdma/aq/{aqid}/dbg_wr_ctrl'
    DATA_FMT='xxd -r /sys/kernel/debug/ionic/{pci}/lif{lif}/rdma/aq/{aqid}/dbg_wr_data > {out}'

# Error checks

if args.offline is True:
    if args.host is None or \
       args.DEVNAME is None:

        print("ERROR: Offline mode mandatory options: --host, --offline, --DEVNAME")
        exit()

else:
    if args.host is not None:
        if args.DEVNAME is None:

            print("ERROR: External mode mandatory options: --host, --DEVNAME")
            exit()

# offline debug
if args.host is not None and \
   args.offline is True and \
   args.DEVNAME is not None:

    # throw an error for unsupported options
    if args.cqcb is not None or \
       args.eqcb is not None or \
       args.aqcb0 is not None or \
       args.aqcb1 is not None or \
       args.dcqcn_profile is not None or \
       args.dcqcn_cb is not None or \
       args.kt_entry is not None or \
       args.pt_entry is not None or \
       args.lif_stats is True:

        print("ERROR: This option is not yet supported in offline mode")
        exit()

    if args.DEVNAME != 'eth0' and \
       args.DEVNAME != 'eth1':

        print("ERROR: offline mode supports following DEVNAMES - eth0 and eth1")
        exit()

    pwd = getPwd()
    cfg = pwd + '/../conf/dev.json'
    with open(cfg, 'r') as f:
        dev_dict = json.load(f)

    found = False
    for dev in dev_dict:
        if (dev["host"] == args.host):
            logging.info(dev["host"])
            found = True
            break

    if found is False:
        print("ERROR: device info not found in conf/dev.json. Please update and try again")
        exit()

    console = dev['console']
    port = dev['port']

    # clear line
    clearLine(console, port)
    time.sleep(1)

    # telnet to console
    port_num = int(int(port) + 2000)
    tn = telnetToConsole(console, port_num)

    lif = getLif(tn, args.DEVNAME)

    # get SQ base address
    sq_base_addr = getQBaseAddr(tn, lif, 0)

    # get RQ base address
    rq_base_addr = getQBaseAddr(tn, lif, 1)

    tn.close()
    # clear line
    clearLine(console, port)
    time.sleep(1)
    tn = telnetToConsole(console, port_num)
    #print(sq_base_addr)
    #print(rq_base_addr)

    if args.num_qps is None:
        n_qp = 1
    else:
        n_qp = int(args.num_qps)

    for i in range(0, n_qp):

        # calculate the offset for specified CB
        if args.rqcb0 is not None:
            qid = int(args.rqcb0) + i
            printQpHeader(qid)
            addr = int(rq_base_addr, 16)
            offset = 0 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRQCB0state(cb_str).show()

        elif args.rqcb1 is not None:
            qid = int(args.rqcb1) + i
            printQpHeader(qid)
            addr = int(rq_base_addr, 16)
            offset = 64 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRQCB1state(cb_str).show()

        elif args.rqcb2 is not None:
            qid = int(args.rqcb2) + i
            printQpHeader(qid)
            addr = int(rq_base_addr, 16)
            offset = 128 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRQCB2state(cb_str).show()

        elif args.rqcb3 is not None:
            qid = int(args.rqcb3) + i
            printQpHeader(qid)
            addr = int(rq_base_addr, 16)
            offset = 192 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRQCB3state(cb_str).show()

        elif args.resp_tx_stats is not None:
            qid = int(args.resp_tx_stats) + i
            printQpHeader(qid)
            addr = int(rq_base_addr, 16)
            offset = 256 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRespTxStats(cb_str).show()

        elif args.resp_rx_stats is not None:
            qid = int(args.resp_rx_stats) + i
            printQpHeader(qid)
            addr = int(rq_base_addr, 16)
            offset = 320 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRespRxStats(cb_str).show()

        elif args.sqcb0 is not None:
            qid = int(args.sqcb0) + i
            printQpHeader(qid)
            addr = int(sq_base_addr, 16)
            offset = 0 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaSQCB0state(cb_str).show()

        elif args.sqcb1 is not None:
            qid = int(args.sqcb1) + i
            printQpHeader(qid)
            addr = int(sq_base_addr, 16)
            offset = 64 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaSQCB1state(cb_str).show()

        elif args.sqcb2 is not None:
            qid = int(args.sqcb2) + i
            printQpHeader(qid)
            addr = int(sq_base_addr, 16)
            offset = 128 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaSQCB2state(cb_str).show()

        elif args.sqcb3 is not None:
            qid = int(args.sqcb3) + i
            printQpHeader(qid)
            addr = int(sq_base_addr, 16)
            offset = 192 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaSQCB3state(cb_str).show()

        elif args.req_tx_stats is not None:
            qid = int(args.req_tx_stats) + i
            printQpHeader(qid)
            addr = int(sq_base_addr, 16)
            offset = 256 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaReqTxStats(cb_str).show()

        elif args.req_rx_stats is not None:
            qid = int(args.req_rx_stats) + i
            printQpHeader(qid)
            addr = int(sq_base_addr, 16)
            offset = 320 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaReqRxStats(cb_str).show()

        elif args.q_state is not None:
            qid = int(args.q_state) + i
            printQpHeader(qid)
            # RQCB0
            addr = int(rq_base_addr, 16)
            offset = 0 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRQCB0state(cb_str).show()
            # RQCB1
            offset = 64 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRQCB1state(cb_str).show()
            # RQCB2
            offset = 128 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRQCB2state(cb_str).show()
            # RQCB3
            offset = 192 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRQCB3state(cb_str).show()
            # SQCB0
            addr = int(sq_base_addr, 16)
            offset = 0 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaSQCB0state(cb_str).show()
            # SQCB1
            offset = 64 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaSQCB1state(cb_str).show()
            # SQCB2
            offset = 128 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaSQCB2state(cb_str).show()
            # SQCB3
            offset = 192 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaSQCB3state(cb_str).show()

        elif args.q_stats is not None:
            qid = int(args.q_stats) + i
            printQpHeader(qid)
            # RespTxStats
            addr = int(rq_base_addr, 16)
            offset = 256 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRespTxStats(cb_str).show()
            # RespRxStats
            offset = 320 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaRespRxStats(cb_str).show()
            # ReqTxStats
            addr = int(sq_base_addr, 16)
            offset = 256 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaReqTxStats(cb_str).show()
            # ReqRxStats
            offset = 320 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)
            RdmaReqRxStats(cb_str).show()

        elif args.rsq is not None: 
            qid = int(args.rsq) + i
            printQpHeader(qid)
            addr = int(rq_base_addr, 16)
            offset = 64 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)

            buff = str(RdmaRQCB1state(cb_str).show(dump=True))

            # get the rsq_base_addr from rqcb1
            m = re.search('rsq_base_addr/q_key= (.*)', buff)
            rsq_base_addr = str(hex(int(m.group(1), 16) << 3))

            # dump all 32 rsq entries
            # ideally we should read log_rsq_size to figure out the depth of RSQ
            # but destroy QP overwrites this with 0 since it is in the same
            # byte as cb1_state. so if it is 0, we assume default RSQ size of 32
            m = re.search('log_rsq_size= (.*)', buff)
            rsq_size = 1 << (int(m.group(1), 16))
            if (rsq_size == 1):
                rsq_size = 32

            for k in range(0, rsq_size):
                print("RSQ entry: {0}".format(k))
                print("==============")
                addr = hex(int(rsq_base_addr, 16) + int(32*k))
                cmd = '/platform/bin/eth_dbgtool memrd ' + addr + ' 32\n'
                tn.write(cmd)
                op = tn.expect(['#'], timeout=5)
                op_list = op[2].split('\n')
                data = ''
                for i in range(1, 3):
                    line = op_list[i].split(' ')
                    for j in range(2, 18):
                        data += line[j]
                cb_str = data.decode("hex")

                # check if read or atomic and decode accordingly
                if (k == 0):
                    buff = RdmaRsqReadstate(cb_str).show(dump=True)
                    m = re.search('read_or_atomic= (.*)', buff)
                    rd_or_atomic = m.group(1)

                if (rd_or_atomic == '0'):
                    RdmaRsqReadstate(cb_str).show()
                else:
                    RdmaRsqAtomicstate(cb_str).show()

        elif args.rrq is not None: 
            qid = int(args.rrq) + i
            printQpHeader(qid)
            addr = int(sq_base_addr, 16)
            offset = 64 + int(512*qid)
            cb_str = getCbData(tn, addr, offset)

            buff = str(RdmaSQCB1state(cb_str).show(dump=True))

            # get rrq_base_addr from sqcb1
            m = re.search('sqcb1_rrq_base_addr= (.*)', buff)
            rrq_base_addr = str(hex(int(m.group(1), 16) << 3))

            # get log_rrq_size from sqcb1
            m = re.search('sqcb1_log_rrq_size= (.*)', buff)
            rrq_size = 1 << int(m.group(1), 16)

            # dump all rrq entries
            for k in range(0, rrq_size):
                print("RRQ entry: {0}".format(k))
                print("==============")
                addr = hex(int(rrq_base_addr, 16) + int(32*k))
                cmd = '/platform/bin/eth_dbgtool memrd ' + addr + ' 32\n'
                tn.write(cmd)
                op = tn.expect(['#'], timeout=5)
                op_list = op[2].split('\n')
                data = ''
                for i in range(1, 3):
                    line = op_list[i].split(' ')
                    for j in range(2, 18):
                        data += line[j]
                cb_str = data.decode("hex")

                # check if read or atomic and decode accordingly
                if (k == 0):
                    buff = RdmaRrqReadstate(cb_str).show(dump=True)
                    m = re.search('read_rsp_or_atomic= (.*)', buff)
                    rd_or_atomic = m.group(1)

                if (rd_or_atomic == '0'):
                    RdmaRrqReadstate(cb_str).show()
                else:
                    RdmaRrqAtomicstate(cb_str).show()

    # clear line
    clearLine(console, port)

    exit()

# dump rdmactl from an external server
if args.host is not None and \
   args.DEVNAME is not None:

    options = USER + '@' + HOST + ' -t '
    CMD_PREFIX = REMOTE_CMD_FMT.format(
                 cmd = 'ssh', options = options)

#print args
if args.dmesg is False and args.dmesg_file is None :
    if args.DEVNAME is None:
        print 'device not specified'
    else:
        if HOST_OS == 'FreeBSD':
            cmd = CMD_PREFIX + "ifconfig " + args.DEVNAME
            if os.system(cmd + "> /dev/null 2>&1") is not 0:
                print "invalid device " + args.DEVNAME + " specified. please check and try again"
                exit()
            tmp, lif_id = args.DEVNAME.split("ionic")
            #print "derived " + lif_id + " as device id for device" + args.DEVNAME
        else:
            cmd = CMD_PREFIX + "ethtool -i " + args.DEVNAME + " | grep bus-info"
            if os.system(cmd + "> /dev/null 2>&1") is not 0:
                print "invalid device " + args.DEVNAME + " specified. please check and try again"
                exit()
            bus_info = subprocess.check_output(cmd, shell=True)
            if bus_info is None:
                print "invalid device " + args.DEVNAME + " specified. please check and try again"
            tmp, pcie_id = bus_info.split(": ")
            pcie_id, tmp = pcie_id.split(pcie_id[-1])
            pcie_id = pcie_id.rstrip()
            #print "derived " + pcie_id + " as pcie id for device " + args.DEVNAME

if args.cqcb is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_CQ, args.cqcb, 0, 64)
    cqcb = RdmaCQstate(bin_str)
    cqcb.show()
elif args.eqcb is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_EQ, args.eqcb, 0, 64)
    eqcb = RdmaEQstate(bin_str)
    eqcb.show()
elif args.aqcb0 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_AQ, 1, 0, 64, aqid=args.aqcb0)
    aqcb0 = RdmaAQCB0state(bin_str)
    aqcb0.show()
elif args.aqcb1 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_AQ, 1, 64, 64, aqid=args.aqcb1)
    aqcb1 = RdmaAQCB1state(bin_str)
    aqcb1.show()
elif args.aq_debug_enable is not None:
    bin_str = exec_dump_cmd(AQ_DEBUG_ENABLE, 1, 0, 64)
elif args.aq_debug_disable is not None:
    bin_str = exec_dump_cmd(AQ_DEBUG_DISABLE, 1, 0, 64)
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
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.rqcb0, 512, 64)
    rqcb0 = RdmaRQCB0state(bin_str)
    rqcb0.show()
elif args.rqcb1 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.rqcb1, 512+64, 64)
    rqcb1 = RdmaRQCB1state(bin_str)
    rqcb1.show()
elif args.rqcb2 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.rqcb2, 512+128, 64)
    rqcb2 = RdmaRQCB2state(bin_str)
    rqcb2.show()
elif args.rqcb3 is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.rqcb3, 512+192, 64)
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
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.resp_tx_stats, 512+256, 64)
    resp_tx_stats = RdmaRespTxStats(bin_str)
    resp_tx_stats.show()
elif args.resp_rx_stats is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.resp_rx_stats, 512+320, 64)
    resp_rx_stats = RdmaRespRxStats(bin_str)
    resp_rx_stats.show()
elif args.dcqcn_profile is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_DCQCN_PROFILE, args.dcqcn_profile, 0, 64)
    dcqcn_profile = RdmaDcqcnProfile(bin_str)
    dcqcn_profile.show()
elif args.dcqcn_cb is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_DCQCN_CB, args.dcqcn_cb, 0, 64)
    dcqcn_cb = RdmaDcqcnCB(bin_str)
    dcqcn_cb.show()
elif args.kt_entry is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_KT, args.kt_entry, 0, 64)
    kt_entry = RdmaKeyTableEntry(bin_str)
    kt_entry.show()
elif args.pt_entry is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_PT, args.pt_entry, 0, 64)
    pt_entry = RdmaPageTableEntry(bin_str)
    pt_entry.show()
elif args.lif_stats is True:
    bin_str = exec_dump_cmd(DUMP_TYPE_LIF, 0, 0, 1024)
    lif_stats = LifStats(bin_str)
    lif_stats.show()
elif args.q_stats is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.q_state, 0, 1024)
    show_slice(RdmaReqTxStats, bin_str, 256, 64)
    show_slice(RdmaReqRxStats, bin_str, 320, 64)
    show_slice(RdmaRespTxStats, bin_str, 512+256, 64)
    show_slice(RdmaRespRxStats, bin_str, 512+320, 64)
elif args.q_state is not None:
    bin_str = exec_dump_cmd(DUMP_TYPE_QP, args.q_state, 0, 1024)
    show_slice(RdmaSQCB0state, bin_str, 0, 64)
    show_slice(RdmaSQCB1state, bin_str, 64, 64)
    show_slice(RdmaSQCB2state, bin_str, 128, 64)
    show_slice(RdmaSQCB3state, bin_str, 192, 64)
    show_slice(RdmaRQCB0state, bin_str, 512, 64)
    show_slice(RdmaRQCB1state, bin_str, 512+64, 64)
    show_slice(RdmaRQCB2state, bin_str, 512+128, 64)
    show_slice(RdmaRQCB3state, bin_str, 512+192, 64)
    bin_str = exec_dump_cmd(DUMP_TYPE_DCQCN_CB, args.q_state, 0, 64)
    dcqcn_cb = RdmaDcqcnCB(bin_str)
    dcqcn_cb.show()
elif args.dmesg is True:
    parse_dmesg()
elif args.dmesg_file is not None:
    parse_dmesg()
elif args.rsq is not None:
    print("ERROR: This option is not yet supported. Try with --offline")
    exit()
elif args.rrq is not None:
    print("ERROR: This option is not yet supported. Try with --offline")
    exit()
