#! /usr/bin/python3

from iris.test.rdma.utils import *
from infra.common.glopts import GlobalOptions
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
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

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

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

    ############     RQ VALIDATIONS #################
    # verify that e_psn is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 0):
        return False

    # verify that proxy_cindex is incremented by 1
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
        return False

    # verify that token_id is incremented by 0
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 0):
        return False

    # verify that nxt_to_go_token_id is incremented by 0
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 0):
        return False

    ############     RQ STATS VALIDATIONS #################
    # verify that num_pkts is not incremented
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_pkts', 0):
        return False

    # verify that num_bytes is not incremented
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_bytes', 0):
        return False

    # verify that num_send_msgs is not incremented
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_send_msgs', 0):
        return False

    # verify that max_pkts_in_any_msg is 0
    #if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'max_pkts_in_any_msg',0):
    #    return False

    ############     CQ VALIDATIONS #################
    if not ValidateNoCQChanges(tc):
        return False

    ############     EQ VALIDATIONS #################
    if not ValidateNoEQChanges(tc):
        return False

    return True

def TestCaseTeardown(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    logger.info("Setting proxy_cindex/spec_cindex equal to p_index0\n")
    rs.lqp.rq.qstate.data.proxy_cindex = tc.pvtdata.rq_post_qstate.p_index0;
    rs.lqp.rq.qstate.WriteWithDelay();
    return
