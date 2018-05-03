#! /usr/bin/python3

from test.rdma.utils import *
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession

    tc.pvtdata.num_total_bytes = 0x40

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    # ARM CQ and Set EQ's CI=PI for EQ enablement
    rs.lqp.rq_cq.qstate.ArmCq()
    rs.lqp.eq.qstate.reset_cindex(0)

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data

    # Read EQ pre state
    rs.lqp.eq.qstate.Read()
    tc.pvtdata.eq_pre_qstate = rs.lqp.eq.qstate.data
    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

    if step.step_id == 0:
       return True

    ############     RQ VALIDATIONS #################
    # verify that e_psn is incremented by 2
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 2):
        return False

    # verify that proxy_cindex is incremented by 2
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 2):
        return False

    # verify that token_id is incremented by 2
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 2):
        return False

    # verify that nxt_to_go_token_id is incremented by 2
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 2):
        return False

    ############     RQ STATS VALIDATIONS #################
    # verify that num_pkts is incremented by 2
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_pkts', 2):
        return False

    # verify that num_bytes is incremented by pvtdata.num_total_bytes
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_bytes', 2 * tc.pvtdata.num_total_bytes):
        return False

    # verify that num_send_msgs is incremented by 2
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_send_msgs', 2):
        return False

    # verify that num_pkts_in_cur_msg is 1
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'num_pkts_in_cur_msg', 1):
        return False

    # verify that max_pkts_in_any_msg is 1
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'max_pkts_in_any_msg', max([1, tc.pvtdata.rq_pre_qstate.max_pkts_in_any_msg])):
        return False

    ############     CQ VALIDATIONS #################
    if not ValidateRespRx2CQChecks(tc):
        return False

    ############     EQ VALIDATIONS #################
    if not ValidateEQChecks(tc):
        return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    return
