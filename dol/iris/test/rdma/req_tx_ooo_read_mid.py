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
    tc.pvtdata.msn = (tc.pvtdata.sq_pre_qstate.msn + 1)
    tc.pvtdata.r_key1 =  2
    tc.pvtdata.dma_len1 = 2100
    tc.pvtdata.va1 = 0x0102030405060708

    tc.pvtdata.rexmit_r_key1 =  2
    tc.pvtdata.rexmit_dma_len1 = 1076
    tc.pvtdata.rexmit_va1 = 0x0102030405060b08

    tc.pvtdata.r_key2 =  3
    tc.pvtdata.dma_len2 = 2200
    tc.pvtdata.va2 = 0x3132333435363738
    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    tc.pvtdata.test_timer = 1
    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    logger.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseStepTrigger(tc, step):
    logger.info("RDMA TestCaseStepTrigger() Implementation with step_id: %d" % (step.step_id))
    if (GlobalOptions.dryrun): return True
    if step.step_id == 0:
        if tc.pvtdata.test_timer:
            logger.info("RDMA TestCaseStepTrigger() - Setting the system time for FAST_TIMER to 0")
            timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
            timer.Step(0)

    if step.step_id == 2:
        if tc.pvtdata.test_timer:
            logger.info("RDMA TestCaseStepTrigger() - Fast Forwarding the system time by by 101 ticks for FAST_TIMER wheel")
            timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
            timer.Step(101)

    return

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    ring0_mask = (rs.lqp.num_sq_wqes - 1)
    ring4_mask = (rs.lqp.num_rrq_wqes - 1)
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

    if step.step_id == 0:
        # verify that tx_psn is incremented by 6
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 6):
            return False

        # verify that p_index is incremented by 2
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  2):
            return False

        # verify that c_index is incremented by 2
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 2):
            return False

        # verify that ssn is incremented by 2
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'ssn', 2):
            return False

        # verify that lsn is incremented by 2
        if tc.pvtdata.sq_pre_qstate.disable_credits != 1:
            if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'lsn', 2):
                return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that p_index of rrq is incremented by 2
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index4', ring4_mask, 2):
            return False

    elif step.step_id == 1:
        # verify that msn is not incrememnted
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'msn', tc.pvtdata.sq_post_qstate, 'msn'):
            return False

        # verify rexmit_psn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'rexmit_psn', 1):
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

        # verify that token_id is incremented by 2
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 2):
            return False

        # verify that nxt_to_go_token_id is incremented by 2
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 2):
            return False

        # validate cqcb pindex and color
        #if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC_1'):
        #    return False

        # verify that sqcb1_bkttrack_in_progress is not set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb1_bktrack_in_progress', 0):
            return False

        # verify that sqcb0_bkttrack_in_progress is not set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb0_bktrack_in_progress', 0):
            return False

    elif step.step_id == 2:
        # verify that msn is unchanged
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'msn', tc.pvtdata.sq_post_qstate, 'msn'):
            return False

        # verify that c_index of rrq is reset to 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'c_index4', 0):
            return False

        # verify rexmit_psn is unchanged
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'rexmit_psn', tc.pvtdata.sq_post_qstate, 'rexmit_psn'):
            return False

        # verify that tx_psn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
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

        # verify that ssn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'ssn', tc.pvtdata.sq_post_qstate, 'ssn'):
            return False

        # verify that token_id is unchanged
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'token_id', tc.pvtdata.sq_post_qstate, 'token_id'):
            return False

        # verify that nxt_to_go_token_id is unchanged
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'nxt_to_go_token_id', tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id'):
            return False

        # verify that sqcb1_bkttrack_in_progress is not set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb1_bktrack_in_progress', 0):
            return False

        # verify that sqcb0_bkttrack_in_progress is not set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb0_bktrack_in_progress', 0):
            return False


    elif step.step_id == 3:
        msn = tc.pvtdata.sq_pre_qstate.ssn - 2

        # verify that msn is incremented to that of ssn of this msg
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'msn', msn):
            return False

        # verify that c_index of rrq is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index4', ring4_mask, 1):
            return False

        # verify rexmit_psn is incremented by 2
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'rexmit_psn', 2):
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

        # verify that token_id is incremented by 2
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 2):
            return False

        # verify that nxt_to_go_token_id is incremented by 2
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 2):
            return False

        # validate cqcb pindex and color
        if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC_1'):
            return False

    elif step.step_id == 4:
        msn = tc.pvtdata.sq_pre_qstate.ssn - 1

        # verify that msn is incremented to that of ssn of this msg
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'msn', msn):
            return False

        # verify that c_index of rrq is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index4', ring4_mask, 1):
            return False

        # verify rexmit_psn is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'rexmit_psn', 3):
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

        # verify that token_id is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 3):
            return False

        # verify that nxt_to_go_token_id is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 3):
            return False

        # validate cqcb pindex and color
        if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC_2'):
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
    return
