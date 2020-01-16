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

    tc.pvtdata.write_rkey =  10
    tc.pvtdata.write_dma_len = 2112
    tc.pvtdata.write_va = 0x1122334455667788

    tc.pvtdata.read_rkey =  2
    tc.pvtdata.read_dma_len = 64
    tc.pvtdata.read_va = 0x0102030405060708
    SetIterPrivOperEnable(tc)

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data
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
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    ring0_mask = (rs.lqp.num_sq_wqes - 1)
    ring4_mask = (rs.lqp.num_rrq_wqes - 1)
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

    if step.step_id == 0:
        # verify that tx_psn is incremented by 7
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 7):
            return False
        
        # verify that p_index is incremented by 3
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  3):
            return False
        
        # verify that c_index is incremented by 3
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 3):
            return False
        
        # verify that ssn is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'ssn', 3):
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

        # verify that p_index of rrq is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index4', ring4_mask, 1):
            return False

        #verify err_retry_cntr is set to err_retry_count
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'err_retry_count', tc.pvtdata.sq_post_qstate, 'err_retry_ctr'):
            return False

        #verify rnr_retry_cntr is set to rnr_retry_count
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'rnr_retry_count', tc.pvtdata.sq_post_qstate, 'rnr_retry_ctr'):
            return False

        #verify rnr_timeout is not set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'rnr_timeout', 0):
            return False

    elif step.step_id == 1:
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        # verify that msn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'msn', 1):
            return False

        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'credits', tc.pvtdata.sq_post_qstate, 'credits'):
            return False

        # verify that rexmit_psn is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'rexmit_psn', 3):
            return False

        # verify err_retry_cntr is decremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'err_retry_ctr', -1):
            return False

        # verify rnr_retry_cntr is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'rnr_retry_ctr', tc.pvtdata.sq_post_qstate, 'rnr_retry_ctr'):
            return False

        # verify rnr_timeout is not set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'rnr_timeout', 0):
            return False

        # verify that p_index of rrq is bktracked
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'p_index4', 1):
            return False

        # verify that c_index of rrq is bktracked
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'c_index4', 0):
            return False

        # verify that SQ p_index is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index0', tc.pvtdata.sq_post_qstate, 'p_index0'):
            return False

        # verify that SQ c_index remains same after bktracking and retransmit
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index0', tc.pvtdata.sq_post_qstate, 'c_index0'):
            return False

        # validate cqcb pindex and color
        if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC1'):
            return False

    elif step.step_id == 2:
        # verify that msn is incremented to that of ssn of this msg
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'msn', 2):
            return False

        # verify that p_index of rrq is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index4', tc.pvtdata.sq_post_qstate, 'p_index4'):
            return False

        # verify that c_index of rrq is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index4', ring4_mask, 1):
            return False

        # verify that tx_psn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify rexmit_psn is incremented to that of tx_psn
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_post_qstate, 'rexmit_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
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
     
        # verify that SQ p_index is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index0', tc.pvtdata.sq_post_qstate, 'p_index0'):
            return False

        # verify that SQ c_index is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index0', tc.pvtdata.sq_post_qstate, 'c_index0'):
            return False
        
        # verify err_retry_cntr is re-loaded to configured value
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'err_retry_count', tc.pvtdata.sq_post_qstate, 'err_retry_ctr'):
            return False

        # verify rnr_retry_cntr is re-loaded to configured value
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'rnr_retry_count', tc.pvtdata.sq_post_qstate, 'rnr_retry_ctr'):
            return False

        # verify rnr_timeout is cleared
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'rnr_timeout', 0):
            return False

        # validate cqcb pindex and color
        if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC2'):
            return False

    elif step.step_id == 3:

        if not ValidatePostSyncCQChecks(tc):
            return False 

    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.sq_cq_pre_qstate = copy.deepcopy(rs.lqp.sq_cq.qstate.data)

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession 
    #Disable Privileged operations on this QP 
    rs.lqp.sq.qstate.reset_priv() 
    return
