#! /usr/bin/python3

import copy
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
    rs.lqp.rq.qstate.Read()
    rs.lqp.rq.qstate.data.nxt_to_go_token_id = (rs.lqp.rq.qstate.data.nxt_to_go_token_id - 1) & 0xFF;
    rs.lqp.rq.qstate.WriteWithDelay()

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data
    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseStepVerify() Implementation.")
    logger.info("step id: %d" %(step.step_id))

    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    stats_mask = ((2 ** 16) - 1)
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

    if step.step_id == 0:
        ############     RQ VALIDATIONS #################    
        # verify that e_psn is not incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 0):
            return False

        # verify that proxy_cindex is not incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
            return False

        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is NOT incremented 
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 0):
            return False

        # make sure rqcb1 state has moved to ERR
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'cb1_state', 2):
            return False

        # verify that max_recirc_cnt_err is captured
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'max_recirc_cnt_err', 1):
            return False

        # recirc reason should be 2 (inorder_work_not_done)
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'recirc_reason', 1):
            return False
    
        # recirc bth opcode should be 6 (send_first)
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'recirc_bth_opcode', 0):
            return False

        # recirc bth psn should be the psn of the packet (which is same as pre_qstate.e_psn)
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'recirc_bth_psn',
                                   tc.pvtdata.rq_pre_qstate.e_psn):
            return False

        # update nxt_to_go_token_id so that next request can be processed
        rs = tc.config.rdmasession
        rs.lqp.rq.qstate.Read()
        rs.lqp.rq.qstate.data.nxt_to_go_token_id = 0;
        rs.lqp.rq.qstate.data.token_id = 0;
        rs.lqp.rq.qstate.WriteWithDelay()

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    ResetErrQState(tc)
    return
