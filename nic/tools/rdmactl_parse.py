#!/usr/bin/python

import sys
import argparse
import re

# globals
rqcb1_dict    = {}
rqcb2_dict    = {}
rqcb3_dict    = {}
resp_tx_dict  = {}
resp_rx_dict  = {}
cb_dict       = {}

rqcb1_key_list = [
    "pc_offset",
    "rsq_base_addr/q_key",
    "pt_base_addr/rq_hbm_base_addr",
    "log_rq_page_size",
    "log_wqe_size",
    "log_num_wqes",
    "congestion_mgmt_type",
    "cb1_state",
    "log_rsq_size",
    "serv_type",
    "log_pmtu",
    "srq_enabled",
    "cache",
    "immdt_as_dbell",
    "rq_in_hbm ",
    "nak_prune",
    "priv_oper_enable",
    "cq_id",
    "pd",
    "header_template_addr",
    "token_id",
    "nxt_to_go_token_id",
    "work_not_done_recirc_cnt",
    "busy",
    "rqcb1_rsvd1",
    "in_progress",
    "rqcb1_rsvd2",
    "spec_cindex",
    "e_psn",
    "next_op_type",
    "next_pkt_type",
    "spec_en",
    "rqcb1_rsvd3",
    "msn",
    "header_template_size",
    "rqcb1_bt_in_progress",
    "rsq_pindex",
    "curr_wqe_ptr",
    "current_sge_offset",
    "current_sge_id",
    "num_sges",
    "proxy_cindex",
    "proxy_pindex",
    "srq_id",
    "rqcb1_pad"
]

rqcb2_key_list = [
    "rqcb2_rsvd0",
    "ack_nak_psn",
    "aeth_msn",
    "syndrome",
    "credits",
    "rqcb2_rsvd1",
    "bt_read_or_atomic",
    "bt_rsvd0",
    "bt_psn",
    "bt_va",
    "bt_r_key",
    "bt_len",
    "rnr_timeout",
    "rqcb2_rsvd2",
    "pd",
    "rqcb2_pad"
]

resp_tx_key_list = [
    "tx_num_bytes",
    "tx_num_pkts",
    "tx_num_read_resp_pkts",
    "tx_num_acks",
    "tx_num_read_resp_msgs",
    "tx_num_atomic_resp_msgs",
    "tx_num_pkts_in_cur_msg",
    "tx_max_pkts_in_any_msg",
    "tx_num_rnrs",
    "tx_num_seq_errs",
    "tx_last_psn",
    "tx_last_syndrome",
    "tx_last_msn",
    "rp_num_additive_increase",
    "rp_num_fast_recovery",
    "rp_num_hyper_increase",
    "rp_num_byte_threshold_db",
    "rp_num_alpha_timer_expiry",
    "rp_num_timer_T_expiry",
    "rp_num_max_rate_reached",
    "qp_err_disabled",
    "qp_err_dis_rsvd_rkey_err",
    "qp_err_dis_rkey_state_err",
    "qp_err_dis_rkey_pd_mismatch",
    "qp_err_dis_rkey_acc_ctrl_err",
    "qp_err_dis_rkey_va_err",
    "qp_err_dis_type2a_mw_qp_mismatch",
    "qp_err_dis_resp_rx",
    "rqcb4",
]

resp_rx_key_list = [
    "num_bytes",
    "num_pkts",
    "num_send_msgs",
    "num_write_msgs",
    "num_read_req_msgs",
    "num_atomic_fna_msgs",
    "num_atomic_cswap_msgs",
    "num_send_msgs_inv_rkey",
    "num_send_msgs_imm_data",
    "num_write_msgs_imm_data",
    "num_ack_requested",
    "num_ring_dbell",
    "num_pkts_in_cur_msg",
    "max_pkts_in_any_msg",
    "num_recirc_drop_pkts",
    "num_mem_window_inv",
    "num_dup_wr_send_pkts",
    "num_dup_rd_atomic_bt_pkts",
    "num_dup_rd_atomic_drop_pkts",
    "np_ecn_marked_packets"
    "rp_cnp_processed"
    "qp_err_disabled",
    "qp_err_dis_svc_type_err",
    "qp_err_dis_pyld_len_err",
    "qp_err_dis_last_pkt_len_err",
    "qp_err_dis_pmtu_err",
    "qp_err_dis_opcode_err",
    "qp_err_dis_wr_only_zero_len_err",
    "qp_err_dis_unaligned_atomic_va_err",
    "qp_err_dis_insuff_sge_err",
    "qp_err_dis_max_sge_err",
    "qp_err_dis_rsvd_key_err",
    "qp_err_dis_key_state_err",
    "qp_err_dis_key_pd_mismatch",
    "qp_err_dis_key_acc_ctrl_err",
    "qp_err_dis_key_va_err",
    "qp_err_dis_inv_rkey_rsvd_key_err",
    "qp_err_dis_ineligible_mr_err",
    "qp_err_dis_inv_rkey_state_err",
    "qp_err_dis_type1_mw_inv_err",
    "qp_err_dis_type2a_mw_qp_mismatch",
    "qp_err_dis_mr_mw_pd_mismatch",
    "qp_err_dis_mr_state_invalid",
    "qp_err_dis_mr_cookie_mismatch",
    "qp_err_dis_feedback",
    "qp_err_dis_table_error",
    "qp_err_dis_phv_intrinsic_error",
    "qp_err_dis_table_resp_error",
    "qp_err_dis_rsvd",
    "np_ecn_marked_packets",
    "dcqcn_rsvd",
    "recirc_bth_psn",
    "last_bth_opcode",
    "recirc_reason",
    "max_recirc_cnt_err",
    "pad",
]

def getQpList(data):

    qpList = []

    # validate the input file format
    if 'QP:' not in data:
        print "\tERROR: QP numbers not found. Please correct the input file format"
        exit()

    lines = data.split('\n')

    for line in lines:
        if 'QP:' not in line:
            continue

        qpNum = line.split(':')[1]
        qpList.append(qpNum)

    return qpList

def parse(data,
          qp,
          key_list,
          s_pattern,
          e_pattern,
          cb_dict):

    extract = 0 # this indicates whether the expected pattern is found, after which we can extract the data
    index = 0 # this is used to index into the list of keys
    qpFound = 0 # this is used to determine the start of data for a specific QP

    # validate s_pattern
    if s_pattern not in data:
        print "\tERROR: Start pattern not found. Please verify the input file format"
        exit()

    # validate e_pattern
    if e_pattern not in data:
        print "\tERROR: End pattern not found. Please verify the input file format"
        exit()

    lines = data.split('\n')

    for line in lines:

        if 'QP:' in line:
            qpNum = line.split(':')[1]
            if qpNum == qp:
                qpFound = 1

        if qpFound == 0:
            continue

        if index >= len(key_list):
            break

        if e_pattern in line:
            extract = 0
            index = 0
            qpFound = 0
            break

        if extract == 1:
            items = line.split('=')
            key = key_list[index]
            value = items[1]
            #print (index, key, value)
            cb_dict[key] = value
            index = index + 1

        elif s_pattern in line:
            extract = 1
            continue

    return

def parseAndValidate(data,
                     qp,
                     key_list,
                     s_pattern,
                     e_pattern,
                     cb_dict,
                     valFunc):

    parse(data, 
          qp, 
          key_list, 
          s_pattern, 
          e_pattern, 
          cb_dict)

    valFunc()

    return

def errIfSet(field):

    if '1' in cb_dict[field]:
        print "\t\tERROR: {0}, actual: {1}, expected: 0\n" \
               .format(field, cb_dict[field])
        return 1

    return 0

def errIfNoMatch(field1, field2):

    if cb_dict[field1] != cb_dict[field2]:
        print "\t\tERROR: {0}, actual: {1}, expected: {2}\n" \
               .format(field1, cb_dict[field1], cb_dict[field2])
        return 1

    return 0

def errIfNotInRange(field, lo, hi):

    val = int(cb_dict[field])
    if val < lo or \
       val > hi:
        print "\t\tERROR: {0}, actual: {1}, expected a value in the range {2}-{3}\n" \
               .format(field, val, lo, hi)
        return 1

    return 0

def validateRQCB1():

    global cb_dict
    cb_dict = rqcb1_dict
    err = 0

    print "\t====validateRQCB1===="

    err |= errIfSet('busy')
    err |= errIfSet('in_progress')
    err |= errIfSet('rqcb1_bt_in_progress')

    err |= errIfNoMatch('nxt_to_go_token_id', 'token_id')
    err |= errIfNoMatch('proxy_cindex', 'spec_cindex')

    if err == 0:
        print "\t\tPASS: validateRQCB1 passed all checks\n"

    return

def validateRQCB2():

    global cb_dict
    cb_dict = rqcb2_dict
    err = 0

    print "\t====validateRQCB2===="

    err |= errIfNotInRange('syndrome', 0, 31)

    if err == 0:
        print "\t\tPASS: validateRQCB2 passed all checks\n"

    return

def validateRespTxStats():

    global cb_dict
    cb_dict = resp_tx_dict
    err = 0

    print "\t====validateRespTxStats===="

    err |= errIfSet('qp_err_disabled')
    err |= errIfSet('qp_err_dis_rsvd_rkey_err')
    err |= errIfSet('qp_err_dis_rkey_state_err')
    err |= errIfSet('qp_err_dis_rkey_pd_mismatch')
    err |= errIfSet('qp_err_dis_rkey_acc_ctrl_err')
    err |= errIfSet('qp_err_dis_rkey_va_err')
    err |= errIfSet('qp_err_dis_type2a_mw_qp_mismatch')
    err |= errIfSet('qp_err_dis_resp_rx')

    if err == 0:
        print "\t\tPASS: validateRespTxStats passed all checks\n"

    return

def validateRespRxStats():

    global cb_dict
    cb_dict = resp_rx_dict
    err = 0

    print "\t====validateRespRxStats===="

    err |= errIfSet('qp_err_disabled')
    err |= errIfSet('qp_err_dis_svc_type_err')
    err |= errIfSet('qp_err_dis_pyld_len_err')
    err |= errIfSet('qp_err_dis_last_pkt_len_err')
    err |= errIfSet('qp_err_dis_pmtu_err')
    err |= errIfSet('qp_err_dis_opcode_err')
    err |= errIfSet('qp_err_dis_wr_only_zero_len_err')
    err |= errIfSet('qp_err_dis_unaligned_atomic_va_err')
    err |= errIfSet('qp_err_dis_insuff_sge_err')
    err |= errIfSet('qp_err_dis_max_sge_err')
    err |= errIfSet('qp_err_dis_rsvd_key_err')
    err |= errIfSet('qp_err_dis_key_state_err')
    err |= errIfSet('qp_err_dis_key_pd_mismatch')
    err |= errIfSet('qp_err_dis_key_acc_ctrl_err')
    err |= errIfSet('qp_err_dis_key_va_err')
    err |= errIfSet('qp_err_dis_inv_rkey_rsvd_key_err')
    err |= errIfSet('qp_err_dis_ineligible_mr_err')
    err |= errIfSet('qp_err_dis_inv_rkey_state_err')
    err |= errIfSet('qp_err_dis_type1_mw_inv_err')
    err |= errIfSet('qp_err_dis_type2a_mw_qp_mismatch')
    err |= errIfSet('qp_err_dis_mr_mw_pd_mismatch')
    err |= errIfSet('qp_err_dis_mr_state_invalid')
    err |= errIfSet('qp_err_dis_mr_cookie_mismatch')
    err |= errIfSet('qp_err_dis_feedback')
    err |= errIfSet('qp_err_dis_table_error')
    err |= errIfSet('qp_err_dis_phv_intrinsic_error')
    err |= errIfSet('qp_err_dis_table_resp_error')

    if err == 0:
        print "\t\tPASS: validateRespRxStats passed all checks\n"

    return

if __name__ == "__main__":

    parser = argparse.ArgumentParser()

    parser.add_argument('--file', type=str, help='input file with rdmactl output')
    parser.add_argument('--rqcb1', type=str, help='validate rqcb1', metavar='1')
    parser.add_argument('--rqcb2', type=str, help='validate rqcb2', metavar='1')
    parser.add_argument('--rqcb3', type=str, help='validate rqcb3', metavar='1')
    parser.add_argument('--resp_tx_stats', type=str, help='validate resp_tx_stats', metavar='1')
    parser.add_argument('--resp_rx_stats', type=str, help='validate resp_rx_stats', metavar='1')
    parser.add_argument('--q_state', type=str, help='validate the q_state for all cb\'s', metavar='1')
    parser.add_argument('--q_stats', type=str, help='validate the q_stats for all cb\'s', metavar='1')

    args=parser.parse_args()

    if (len(sys.argv) < 5):
        parser.print_help()
        exit()

    if args.file is None:
        print "ERROR: Please specify input file with rdmactl output as argument"
        exit()

    # open the file
    fName = args.file
    with open(fName, 'r') as f:
        data = f.read()
    f.close()

    qps = getQpList(data)

    for qp in qps:

        print '==============='
        print '    QP: {0}    '.format(qp)
        print '==============='

        if args.rqcb1:
            parseAndValidate(data,
                             qp,
                             rqcb1_key_list, 
                             'RdmaRQCB1state', 
                             'RdmaRQCB2state', 
                             rqcb1_dict, 
                             validateRQCB1)

        if args.rqcb2:
            parseAndValidate(data, 
                             qp,
                             rqcb2_key_list, 
                             'RdmaRQCB2state', 
                             'RdmaRQCB3state', 
                             rqcb2_dict, 
                             validateRQCB2)

        if args.rqcb3:
            print "Nothing to validate here"
            exit()

        if args.resp_tx_stats:
            parseAndValidate(data,
                             qp,
                             resp_tx_key_list,
                             'RdmaRespTxStats',
                             'RdmaRespRxStats',
                             resp_tx_dict,
                             validateRespTxStats)

        if args.resp_rx_stats:
            parseAndValidate(data,
                             qp,
                             resp_rx_key_list,
                             'RdmaRespRxStats',
                             'pad',
                             resp_rx_dict,
                             validateRespRxStats)

        if args.q_state:
            parseAndValidate(data, 
                             qp,
                             rqcb1_key_list, 
                             'RdmaRQCB1state', 
                             'RdmaRQCB2state', 
                             rqcb1_dict, 
                             validateRQCB1)

            parseAndValidate(data, 
                             qp,
                             rqcb2_key_list, 
                             'RdmaRQCB2state', 
                             'RdmaRQCB3state', 
                             rqcb2_dict, 
                             validateRQCB2)

        if args.q_stats:
            parseAndValidate(data,
                             qp,
                             resp_tx_key_list,
                             'RdmaRespTxStats',
                             'RdmaRespRxStats',
                             resp_tx_dict,
                             validateRespTxStats)

            parseAndValidate(data,
                             qp,
                             resp_rx_key_list,
                             'RdmaRespRxStats',
                             'pad',
                             resp_rx_dict,
                             validateRespRxStats)
