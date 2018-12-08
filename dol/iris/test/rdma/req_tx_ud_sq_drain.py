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
    tc.pvtdata.q_key = 0x11111111  #TODO:Need to get from rqp->q_key
    tc.pvtdata.dst_qp = tc.config.rdmasession.rqp.id
    tc.pvtdata.ah_handle = tc.config.rdmasession.ah_handle

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    # Read EQ pre state
    rs.lqp.eq.qstate.Read()
    tc.pvtdata.async_eq_pre_qstate = rs.lqp.pd.ep.intf.lif.async_eq.qstate.data
    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseStepTrigger(tc, step):
    logger.info("RDMA TestCaseStepTrigger() Implementation with step_id: %d" % (step.step_id))
    if (GlobalOptions.dryrun): return True

    if step.step_id == 1:
        rs = tc.config.rdmasession
        rs.lqp.sq.qstate.Read()
        tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

        logger.info("RDMA TestCaseStepTrigger() - Setting SQCB state to QP_STATE_SQD")
        rs.lqp.sq.qstate.data.state = 6
        rs.lqp.sq.qstate.data.sqcb1_state = 6
        rs.lqp.sq.qstate.WriteWithDelay()

    if step.step_id == 2:
        rs = tc.config.rdmasession
        rs.lqp.sq.qstate.Read()

        logger.info("RDMA TestCaseStepTrigger() - Setting SQCB state back to QP_STATE_RTS")
        rs.lqp.sq.qstate.data.state = 4
        rs.lqp.sq.qstate.data.sqcb1_state = 4
        rs.lqp.sq.qstate.data.sqcb0_sq_drained = 0
        rs.lqp.sq.qstate.data.sqcb1_sq_drained = 0
        rs.lqp.sq.qstate.WriteWithDelay()

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
    ring4_mask = (rs.lqp.num_rrq_wqes - 1)
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

    if step.step_id == 0:
        # verify that tx_psn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 1):
            return False

        # verify that p_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  1):
            return False

        # verify that c_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 1):
            return False

        # verify that ssn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'ssn', 1):
            return False

        # verify that lsn is not incremented
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'lsn', 0):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # validate cqcb pindex and color
        if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC1'):
            return False

    elif step.step_id == 1:
        # verify cb0 and cb1 state are modified to SQ drain QP_STATE_SQD
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'state', 6):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb1_state', 6):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that tx_psn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify that SQ p_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  1):
            return False

        # verify that SQ c_index still remains same and no message is processed
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index0', tc.pvtdata.sq_post_qstate, 'c_index0'):
            return False

        # verify affiliated async event for SQ Drain
        if not ValidateAsyncEQChecks(tc):
            return False

    elif step.step_id == 2:
        # verify cb0 and cb1 state are modified to RTS
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'state', 4):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb1_state', 4):
            return False

        # verify that tx_psn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 1):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that SQ p_index is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index0', tc.pvtdata.sq_post_qstate, 'p_index0'):
            return False

        # verify that SQ c_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 1):
            return False

        # validate cqcb pindex and color
        if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC2'):
            return False

    elif step.step_id == 5:
        if not ValidatePostSyncCQChecks(tc):
            return False

    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.sq_cq_pre_qstate = copy.deepcopy(rs.lqp.sq_cq.qstate.data)

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    return
