#! /usr/bin/python3

from test.rdma.utils import *
from infra.common.glopts import GlobalOptions

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    tc.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession

    tc.pvtdata.num_total_bytes = 0x3b
    tc.pvtdata.roce_opt_ts_list = [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88]
    tc.pvtdata.roce_opt_ts_value = 0x11223344
    tc.pvtdata.roce_opt_ts_echo = 0x55667788
    tc.pvtdata.roce_opt_mss = 0xabcd

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    #tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    # Clear & Write back roce_opt values in RQCB3
    rs.lqp.rq.qstate.data.roce_opt_ts_value = 0
    rs.lqp.rq.qstate.data.roce_opt_ts_echo = 0
    rs.lqp.rq.qstate.data.roce_opt_mss = 0
    rs.lqp.rq.qstate.Write()

    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data
    return

def TestCaseTrigger(tc):
    tc.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    tc.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

    ############     RQ VALIDATIONS #################
    # verify that e_psn is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 1):
        return False

    # verify that proxy_cindex is incremented by 1
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 1):
        return False

    # verify that token_id is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
        return False

    # verify that nxt_to_go_token_id is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
        return False

    # verify that busy is 0
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'busy', 0):
        return False

    ############     RQ STATS VALIDATIONS #################
    # verify that num_pkts is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_pkts', 1):
        return False

    # verify that num_bytes is incremented by pvtdata.num_total_bytes
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_bytes', tc.pvtdata.num_total_bytes):
        return False

    # verify that num_send_msgs is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_send_msgs', 1):
        return False

    # verify that num_pkts_in_cur_msg is 1
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'num_pkts_in_cur_msg', 1):
        return False

    # verify that max_pkts_in_any_msg is 1
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'max_pkts_in_any_msg', max([1, tc.pvtdata.rq_pre_qstate.max_pkts_in_any_msg])):
        return False

    # verify that supplied roce_opt_ts_value is written to the RQCB3->roce_opt_ts_value
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'roce_opt_ts_value', tc.pvtdata.roce_opt_ts_value):
        return False

    # verify that supplied roce_opt_ts_echo is written to the RQCB3->roce_opt_ts_echo
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'roce_opt_ts_echo', tc.pvtdata.roce_opt_ts_echo):
        return False

    # verify that supplied roce_opt_mss is written to the RQCB3->roce_opt_mss
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'roce_opt_mss', tc.pvtdata.roce_opt_mss):
        return False

    ############     CQ VALIDATIONS #################
    if not ValidateRespRxCQChecks(tc):
        return False

    return True

def TestCaseTeardown(tc):
    tc.info("RDMA TestCaseTeardown() Implementation.")
    return
