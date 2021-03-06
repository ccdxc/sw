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

    # ARM CQ and Set EQ's CI=PI for EQ enablement
    #rs.lqp.sq_cq.qstate.ArmCq()
    rs.lqp.eq.qstate.reset_cindex(0)

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.reset_proxy_s_pindex()
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    # Read EQ pre state
    rs.lqp.eq.qstate.Read()
    tc.pvtdata.eq_pre_qstate = rs.lqp.eq.qstate.data

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
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

    if step.step_id == 0:

        rs = tc.config.rdmasession
        rs.lqp.sq_cq.qstate.Read()
        tc.pvtdata.sq_cq_post_qstate = rs.lqp.sq_cq.qstate.data
    
        # verify that arm is set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_cq_post_qstate, 'arm', 1):
            return False

    elif step.step_id == 1:

        rs = tc.config.rdmasession
        rs.lqp.sq_cq.qstate.Read()
        tc.pvtdata.sq_cq_post_qstate = rs.lqp.sq_cq.qstate.data
    
        # verify that arm is set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_cq_post_qstate, 'arm', 1):
            return False

        # verify that sarm is set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_cq_post_qstate, 'sarm', 1):
            return False

    elif step.step_id == 2:

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

    elif step.step_id == 3:
        msn = tc.pvtdata.sq_pre_qstate.ssn - 1

        # verify that msn is incremented to that of ssn of this msg
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'msn', msn):
            return False

        # verify that c_index of rrq is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index4', tc.pvtdata.sq_post_qstate, 'c_index4'):
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

        # verify that tx_psn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC'):
            return False 

        ############     EQ VALIDATIONS #################
        if not ValidateEQChecks(tc):
            return False

    elif step.step_id == 4:

        if not ValidatePostSyncCQChecks(tc):
            return False 

    elif step.step_id == 5:

        rs = tc.config.rdmasession
        rs.lqp.sq_cq.qstate.Read()
        tc.pvtdata.sq_cq_post_qstate = rs.lqp.sq_cq.qstate.data
    
        # verify that arm is not set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_cq_post_qstate, 'arm', 0):
            return False

        # verify that sarm is not set
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_cq_post_qstate, 'sarm', 0):
            return False


    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    return
