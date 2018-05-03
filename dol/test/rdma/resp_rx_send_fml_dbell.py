#! /usr/bin/python3

from test.rdma.utils import *
from infra.common.glopts import GlobalOptions
import copy
from infra.common.logging import logger as logger
def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = copy.deepcopy(rs.lqp.rq.qstate.data)
    tc.pvtdata.send_first_psn = tc.pvtdata.rq_pre_qstate.e_psn
    tc.pvtdata.send_mid_psn = tc.pvtdata.rq_pre_qstate.e_psn + 1
    tc.pvtdata.send_last_psn = tc.pvtdata.rq_pre_qstate.e_psn + 2

    #Set the immdt_as_dbell property on the QP
    rs.lqp.rq.qstate.data.immdt_as_dbell = 1
    rs.lqp.rq.qstate.WriteWithDelay();

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data
    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

    ############     RQ VALIDATIONS #################
    # verify that e_psn is incremented by 3
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 3):
        return False

    # verify that p_index0 is incremented by 1
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index0', ring0_mask,  1):
        return False

    # verify that c_index0 is incremented by 1
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'c_index0', ring0_mask,  1):
        return False

    # verify that proxy_cindex is incremented by 1
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 1):
        return False

    # verify that token_id is incremented by 3
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 3):
        return False

    # verify that nxt_to_go_token_id is incremented by 3
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 3):
        return False

    ############     CQ VALIDATIONS #################
    if not ValidateRespRxCQChecks(tc):
        return False


    return True

def TestCaseTeardown(tc):
    if (GlobalOptions.dryrun): return True

    rs = tc.config.rdmasession

    #Set the immdt_as_dbell property on the QP
    rs.lqp.rq.qstate.Read()
    rs.lqp.rq.qstate.data.immdt_as_dbell = 0
    rs.lqp.rq.qstate.WriteWithDelay();

    logger.info("RDMA TestCaseTeardown() Implementation.")
    return
