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

    rs.lqp.sq.qstate.data.err_retry_count = 0
    rs.lqp.sq.qstate.data.rnr_retry_count = 0
    rs.lqp.sq.qstate.data.err_retry_ctr = 0
    rs.lqp.sq.qstate.data.rnr_retry_ctr = 0
    rs.lqp.sq.qstate.WriteWithDelay()

    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.msn = (tc.pvtdata.sq_pre_qstate.msn + 1)

    tc.pvtdata.test_timer = 1

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data
    return

def TestCaseStepTrigger(tc, step):
    logger.info("RDMA TestCaseStepTrigger() Implementation with step_id: %d" % (step.step_id))
    if (GlobalOptions.dryrun): return True
    if step.step_id == 0:
        if tc.pvtdata.test_timer:
            logger.info("RDMA TestCaseStepTrigger() - Setting the system time for SLOW_TIMER to 0")
            timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
            timer.Step(0)

    if step.step_id == 1:
        if tc.pvtdata.test_timer:
            logger.info("RDMA TestCaseStepTrigger() - Fast Forwarding the system time by by 101 ticks for SLOW_TIMER wheel")
            timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
            timer.Step(101)

    return

def TestCaseVerify(tc):
    logger.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseStepVerify(tc, step):
    logger.info("RDMA TestCaseStepVerify() Implementation with step_id: %d" % (step.step_id))
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    PopulatePostQStates(tc)
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    ring0_mask = (rs.lqp.num_sq_wqes - 1)

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

        # verify that lsn is not incremented for send
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'lsn', 0):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that p_index of rrq is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index4', tc.pvtdata.sq_post_qstate, 'p_index4'):
            return False

        # verify that retry_time_on is set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'timer_on', 1):
            return False

    elif step.step_id == 1:
        # verify that tx_psn is unchanged
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify that p_index is unchanged
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index0', tc.pvtdata.sq_post_qstate, 'p_index0'):
            return False

        # verify that c_index is unchanged
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index0', tc.pvtdata.sq_post_qstate, 'c_index0'):
            return False

        # verify that ssn is unchanged
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'ssn', tc.pvtdata.sq_post_qstate, 'ssn'):
            return False

        # verify that lsn is unchanged
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'lsn', tc.pvtdata.sq_post_qstate, 'lsn'):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that p_index of rrq is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index4', tc.pvtdata.sq_post_qstate, 'p_index4'):
            return False

        # verify that retry_time_on is set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'timer_on', 1):
            return False

        # verify qp_err_disable is set
        #if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'qp_err_disabled', 1):
        #    return False

        # verify qp_err_dis_rnr_retry_exceed is set
        #if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'qp_err_dis_err_retry_exceed', 1):
        #    return False

        # There will be two completions. One in sq_cq for actual error and another in
        # rq_cq for flush error
        if not ValidateCQCompletions(tc, 1, 1):
            return False

        # verify that state is now moved to ERR (2)
        if not VerifyErrQState(tc):
            return False

    elif step.step_id == 2:

        if not ValidatePostSyncCQChecks(tc):
            return False

    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.sq_cq_pre_qstate = copy.deepcopy(rs.lqp.sq_cq.qstate.data)

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    if (GlobalOptions.dryrun): return

    ResetErrQState(tc)
    return
