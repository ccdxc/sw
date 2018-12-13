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

    PopulateAdminPreQStates(tc)

    tc.pvtdata.save_aq_state = copy.deepcopy(tc.pvtdata.aq_pre_qstate)
    tc.pvtdata.save_aq_cq_state = copy.deepcopy(tc.pvtdata.aq_cq_pre_qstate)
    # Simulate CQ full by meddling with the ring indices
    logger.info("Setting proxy_pindex equal to c_index0\n")
    tc.pvtdata.aq.cq.qstate.data.proxy_pindex = tc.pvtdata.aq.cq.qstate.data.c_index0
    tc.pvtdata.aq.cq.qstate.data.cq_full_hint = 1
    tc.pvtdata.aq.cq.qstate.WriteWithDelay();

    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    logger.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")

    tc.pvtdata.aq.aq.qstate.Read()
    ring0_mask = (tc.pvtdata.aq.num_aq_wqes - 1)
    tc.pvtdata.aq_post_qstate = tc.pvtdata.aq.aq.qstate.data

    tc.pvtdata.aq.cq.qstate.Read()
    tc.pvtdata.aq_cq_post_qstate = tc.pvtdata.aq.cq.qstate.data
    if step.step_id == 0:
        # verify that error is 1
        if not VerifyFieldAbsolute(tc, tc.pvtdata.aq_post_qstate, 'error', 1):
            return False

        # verify that p_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.aq_pre_qstate, tc.pvtdata.aq_post_qstate, 'p_index0', ring0_mask,  1):
            return False

        # verify that c_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.aq_pre_qstate, tc.pvtdata.aq_post_qstate, 'c_index0', ring0_mask, 1):
            return False

        # verify that cq_full is set
        if not tc.pvtdata.aq_cq_post_qstate.cq_full == 1:
            return False

        ############## ASYNC EQ VALIDATIONS ###############
        # verify that async EQE is generated with EQE_CODE_CQ_ERR_FULL error
        if not ValidateAdminAsyncEQChecks(tc):
            return False

        tc.pvtdata.async_eq_pre_qstate = tc.pvtdata.async_eq_post_qstate
        tc.pvtdata.aq_pre_qstate = copy.deepcopy(tc.pvtdata.aq_post_qstate)
    elif step.step_id == 1:

        # verify that error is 1
        if not VerifyFieldAbsolute(tc, tc.pvtdata.aq_post_qstate, 'error', 1):
            return False

        # verify that p_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.aq_pre_qstate, tc.pvtdata.aq_post_qstate, 'p_index0', ring0_mask, 1):
            return False

        # verify that c_index is not incremented
        if not tc.pvtdata.aq_pre_qstate.c_index0 == tc.pvtdata.aq_post_qstate.c_index0:
            return False

        # verify that cq_full is set
        if not tc.pvtdata.aq_cq_post_qstate.cq_full == 1:
            return False

        # verify that CQ indices do not move
        if not tc.pvtdata.aq_cq_post_qstate.c_index0 == tc.pvtdata.aq_cq_pre_qstate.c_index0:
            return False
        if not tc.pvtdata.aq_cq_post_qstate.proxy_pindex == tc.pvtdata.aq_cq_pre_qstate.c_index0:
            return False

        # verify that no more async EQE is generated
        if not ValidateAdminAsyncEQChecks(tc, num_wqes=0):
            return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")

    # Restore aqcb state
    tc.pvtdata.aq.aq.qstate.data = copy.deepcopy(tc.pvtdata.save_aq_state)
    tc.pvtdata.aq.aq.qstate.WriteWithDelay();
    # Restore cqcb state
    tc.pvtdata.aq.cq.qstate.data = copy.deepcopy(tc.pvtdata.save_aq_cq_state)
    tc.pvtdata.aq.cq.qstate.WriteWithDelay();

    return

