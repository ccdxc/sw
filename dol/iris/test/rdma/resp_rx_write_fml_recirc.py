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
        # verify that e_psn is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 3):
            return False

        # verify that proxy_cindex is not incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
            return False

        # verify that token_id is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 3):
            return False

        # verify that nxt_to_go_token_id is NOT incremented 
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 0):
            return False

        # TODO stage0 gets invoked with recirc_cnt = 7 two times, 
        # causing num_recirc_drop_pkts to be incremented by 6
        # verify that num_recirc_drop_pkts is incremented by 3
        #if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_recirc_drop_pkts', ring1_mask, 3):
        #    return False

        # update nxt_to_go_token_id so that next request can be processed
        rs = tc.config.rdmasession
        rs.lqp.rq.qstate.Read()
        rs.lqp.rq.qstate.data.nxt_to_go_token_id = 0;
        rs.lqp.rq.qstate.data.token_id = 0;
        rs.lqp.rq.qstate.WriteWithDelay()

    elif step.step_id == 1:
        ############     RQ VALIDATIONS #################
        # verify that e_psn is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 3):
            return False

        # verify that proxy_cindex is not incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
            return False

        # verify that token_id is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 3):
            return False

        # verify that nxt_to_go_token_id is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 3):
            return False

        # verify that num_recirc_drop_pkts is NOT incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_recirc_drop_pkts', stats_mask, 0):
            return False

    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.rq_pre_qstate = copy.deepcopy(rs.lqp.rq.qstate.data)
    tc.pvtdata.rq_cq_pre_qstate = copy.deepcopy(rs.lqp.rq_cq.qstate.data)

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    rs.lqp.rq.qstate.data.nxt_to_go_token_id = 0;
    rs.lqp.rq.qstate.data.token_id = 0;
    rs.lqp.rq.qstate.WriteWithDelay()
    return
