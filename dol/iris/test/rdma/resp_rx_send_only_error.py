#! /usr/bin/python3

from iris.test.rdma.utils import *
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger

def Setup(infra, module):
    iterelem = module.iterator.Get()

    logger.info("Iterator Selectors")

    if iterelem:
        if 'base' in iterelem.rdmasession.__dict__:
            logger.info("- rdmasession.base: %s" % iterelem.rdmasession.base)
            module.testspec.selectors.rdmasession.base.Extend(iterelem.rdmasession.base)
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

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data
    
    return

def TestCasePreTrigger(tc):
    if (GlobalOptions.dryrun): return
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    logger.info("In TestCasePreTrigger, cb1 state {0}".format(rs.lqp.rq.qstate.data.cb1_state))
    # purposely initialize the CB to error state
    rs.lqp.rq.qstate.data.cb1_state = 2 # ERROR
    rs.lqp.rq.qstate.WriteWithDelay()
    logger.info("In TestCasePreTrigger, updated cb1 state {0}".format(rs.lqp.rq.qstate.data.cb1_state))
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

    
    ############     RQ VALIDATIONS #################
    # verify that e_psn is NOT incremented
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 0):
        return False
    
    # verify that proxy_cindex is NOT incremented
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
        return False
    
    # verify that token_id is NOT incremented
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 0):
        return False
    
    # verify that nxt_to_go_token_id is NOT incremented
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 0):
        return False
    
    ############     RQ STATS VALIDATIONS #################
    # verify that num_pkts is NOT incremented
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_pkts', 0):
        return False
    
    # verify that num_bytes is incremented by pvtdata.num_total_bytes
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_bytes', 0):
        return False
    
    # verify that num_send_msgs is NOT incremented
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_send_msgs', 0):
        return False
    
    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    ResetErrQState(tc)
    return
