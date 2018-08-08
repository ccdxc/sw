#! /usr/bin/python3

from iris.test.rdma.utils import *

from iris.config.objects.rdma.keytable import *                                                                    
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
    tc.pvtdata.send_only_psn = tc.pvtdata.rq_pre_qstate.e_psn

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data

    if (GlobalOptions.dryrun): return
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
    
        ############     RQ VALIDATIONS #################
        # verify that e_psn is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 3):
            return False
    
        # verify that proxy_cindex is NOT incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
            return False
    
    
        # verify that token_id is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 3):
            return False
    
        # verify that nxt_to_go_token_id is incremented by 2
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 2):
            return False
    
        # verify that state is now moved to ERR (2)
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'cb1_state', 2):
            return False


    elif step.step_id == 1:

        if not ValidatePostSyncCQChecks(tc):
            return False 

    return True


def TestCaseTeardown(tc):
    if (GlobalOptions.dryrun): return
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.data.proxy_cindex = tc.pvtdata.rq_post_qstate.p_index0;
    rs.lqp.rq.qstate.data.spec_cindex = tc.pvtdata.rq_post_qstate.p_index0;
    rs.lqp.rq.qstate.data.cb0_state = rs.lqp.rq.qstate.data.cb1_state = 6;
    rs.lqp.rq.qstate.data.token_id = rs.lqp.rq.qstate.data.nxt_to_go_token_id;
    rs.lqp.rq.qstate.WriteWithDelay();
    return
