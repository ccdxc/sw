#! /usr/bin/python3

from iris.test.rdma.utils import *
import pdb
import copy
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseStepTrigger(tc, step):
    logger.info("RDMA TestCaseStepTrigger() Implementation.")
    if (GlobalOptions.dryrun): return True

    if (step.step_id == 0):
        rs = tc.config.rdmasession
        rs.lqp.sq.qstate.Read()
        rs.lqp.sq.qstate.data.disable_credits = 0
        rs.lqp.sq.qstate.data.lsn = rs.lqp.sq.qstate.data.msn
        rs.lqp.sq.qstate.data.lsn_rx = rs.lqp.sq.qstate.data.msn
        rs.lqp.sq.qstate.data.lsn_tx = rs.lqp.sq.qstate.data.msn
        rs.lqp.sq.qstate.WriteWithDelay()

        tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)

    return

def TestCaseVerify(tc):
    logger.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    ring0_mask = (rs.lqp.num_sq_wqes - 1)
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

    if step.step_id == 0:
        msn = tc.pvtdata.sq_pre_qstate.msn

        # verify that tx_psn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 1):
            return False

        # verify that p_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  1):
            return False

        # verify that c_index is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index0', tc.pvtdata.sq_post_qstate, 'c_index0'):
            return False

        # verify that ssn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'ssn', tc.pvtdata.sq_post_qstate, 'ssn'):
            return False

        # verify that lsn is 0 credits initially
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'lsn', tc.pvtdata.sq_post_qstate, 'msn'):
            return False

        # verify that lsn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'lsn_rx', tc.pvtdata.sq_post_qstate, 'msn'):
            return False

        # verify that lsn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'lsn_tx', tc.pvtdata.sq_post_qstate, 'msn'):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 1
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 1):
            return False

    elif step.step_id == 1:
        msn = tc.pvtdata.sq_post_qstate.msn
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        # verify that msn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'msn', tc.pvtdata.sq_post_qstate, 'msn'):
            return False

        # verify that rexmit_psn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'rexmit_psn', tc.pvtdata.sq_post_qstate, 'rexmit_psn'):
            return False

        # verify that tx_psn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify that p_index is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index0', tc.pvtdata.sq_post_qstate, 'p_index0'):
            return False

        # verify that c_index is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index0', tc.pvtdata.sq_post_qstate, 'c_index0'):
            return False

        # verify that ssn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'ssn', tc.pvtdata.sq_post_qstate, 'ssn'):
            return False

        # verify that lsn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'lsn', tc.pvtdata.sq_post_qstate, 'lsn'):
            return False

        # verify that lsn_rx is incremented by 128 credits
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'lsn_rx', (msn+128)):
            return False

        # verify that lsn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'lsn_tx', tc.pvtdata.sq_post_qstate, 'lsn_tx'):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 1
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 1):
            return False

    elif step.step_id == 2:
        # verify that tx_psn is incremented by 2
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 2):
            return False

        # verify that p_index is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index0', tc.pvtdata.sq_post_qstate, 'p_index0'):
            return False

        # verify that c_index is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', 1):
            return False

        # verify that ssn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'ssn', 1):
            return False

        # verify that lsn is incremented by 128 credits
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'lsn', tc.pvtdata.sq_post_qstate, 'lsn'):
            return False

        # verify that lsn_rx is updated by 64 credits
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'lsn_rx', tc.pvtdata.sq_post_qstate, 'lsn_rx'):
            return False

        # verify that lsn_tx is incremented by 128 credits
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'lsn_tx', tc.pvtdata.sq_post_qstate, 'lsn_tx'):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

    elif step.step_id == 3:
        msn = tc.pvtdata.sq_post_qstate.ssn - 1
        # verify that msn is incremented to last msg sent
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'msn', msn):
            return False

        # verify that tx_psn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify rexmit_psn is incremented to tx_psn
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_post_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'rexmit_psn'):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        # verify that lsn is  same as before
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'lsn', tc.pvtdata.sq_post_qstate, 'lsn'):
            return False

        # verify that lsn_rx is updated by 64 credits
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'lsn_rx', (msn+96)):
            return False

        # verify that lsn_tx same as before
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'lsn_tx', tc.pvtdata.sq_post_qstate, 'lsn_tx'):
            return False

        # verify that SQ p_index is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index0', tc.pvtdata.sq_post_qstate, 'p_index0'):
            return False

        # verify that SQ c_index is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index0', tc.pvtdata.sq_post_qstate, 'c_index0'):
            return False

        # validate cqcb pindex and color
        if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC'):
            return False

    elif step.step_id == 4:

        if not ValidatePostSyncCQChecks(tc):
            return False

    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.sq_cq_pre_qstate = copy.deepcopy(rs.lqp.sq_cq.qstate.data)

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    if (GlobalOptions.dryrun): return True

    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data.disable_credits = 1
    rs.lqp.sq.qstate.WriteWithDelay()
    return
