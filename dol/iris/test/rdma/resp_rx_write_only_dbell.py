#! /usr/bin/python3

from iris.test.rdma.utils import *
from infra.common.glopts import GlobalOptions
import random
from infra.common.logging import logger as logger
def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession

    #fmt: <lif(11), qtype(3), qid(18)>
    tc.pvtdata.imm_data = (rs.lqp.pd.ep.intf.lif.hw_lif_id << 21) | (1 << 18) | (rs.lqp.id) 

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

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

    # verify that e_psn is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 1):
        return False

    # verify that pindex is incremented by 0
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index0', ring0_mask,  0):
        return False

    # verify that cindex is incremented by 0
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'c_index0', ring0_mask,  0):
        return False

    # verify that proxy_cindex is incremented by 0
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask,  0):
        return False

    # verify that token_id is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
        return False

    # verify that nxt_to_go_token_id is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
        return False

    return True

def TestCaseTeardown(tc):
    if (GlobalOptions.dryrun): return True

    rs = tc.config.rdmasession

    #Set the immdt_as_dbell property on the QP
    rs.lqp.rq.qstate.Read()
    rs.lqp.rq.qstate.data.immdt_as_dbell = 0
    rs.lqp.rq.qstate.data.proxy_cindex = tc.pvtdata.rq_post_qstate.p_index0;
    rs.lqp.rq.qstate.WriteWithDelay();

    logger.info("RDMA TestCaseTeardown() Implementation.")
    return
