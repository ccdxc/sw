#! /usr/bin/python3

from iris.test.rdma.utils import *
import pdb
import copy
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
from iris.config.objects.rdma.keytable import *
import infra.common.objects as objects
import math
            
def Setup(infra, module): 
    return

def Teardown(infra, module):
    return      
        
def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession

    # Enable FRPMR on QP.
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data.priv_oper_enable = 1;
    rs.lqp.sq.qstate.WriteWithDelay()

    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.dst_qp = tc.config.rdmasession.rqp.id

    tc.pvtdata.wrid = 0x1234

    # Get new key object for FRPMR.
    tc.pvtdata.key  = rs.lqp.pd.GetNewKey()
    tc.pvtdata.l_key = tc.pvtdata.key.lkey
    tc.pvtdata.mr_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.l_key))

    tc.pvtdata.user_key = 192

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data
        
    return  

def TestCaseStepTrigger(tc, step):
    logger.info("RDMA TestCaseStepTrigger() Implementation with step_id: %d" % (step.step_id))
    if (GlobalOptions.dryrun): return True

    if step.step_id == 1:
        rs = tc.config.rdmasession
        rs.lqp.sq.qstate.Read()
        tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

        logger.info("RDMA TestCaseStepTrigger() - Setting SQCB state frpmr_in_progress to 1")
        rs.lqp.sq.qstate.data.frpmr_in_progress = 1
        # Decrement cindex and sqd_cindex in sqcb0 to revert FRPMR execution
        rs.lqp.sq.qstate.data.c_index0 -= 1
        rs.lqp.sq.qstate.data.sqd_cindex -= 1
        rs.lqp.sq.qstate.WriteWithDelay()
    
        # Set KT entry to free to execute FRPMR again
        kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.l_key)
        kt_entry.data.state = 1 #KEY_STATE_FREE
        kt_entry.WriteWithDelay()

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
        # verify that tx_psn is incremented by 1 
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 1):
            return False

        # verify that p_index is incremented by 2
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  2):
            return False

        # verify that c_index is incremented by 2
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 2):
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

        # verify that p_index of rrq is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index4', tc.pvtdata.sq_post_qstate, 'p_index4'):
            return False

        # verify that frpmr_reset_spec_cindex is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'frpmr_in_progress', 0):
            return False

        # validate cqcb pindex and color
        if not ValidateCQCompletions(tc, 1, 0):
            return False

    elif step.step_id == 1:

        # verify that tx_psn is not incremented
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 0):
            return False

        # verify that p_index is not incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask,  0):
            return False

        # verify that c_index is not incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 0):
            return False

        # verify that ssn is not incremented
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'ssn', 0):
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

    elif step.step_id == 2:
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

        # validate cqcb pindex and color
        if not ValidateCQCompletions(tc, 1, 0):
            return False

        if not ValidateReqRxCQChecks(tc, 'EXP_SEND_CQ_DESC'):
            return False
        ############     SKIP EQ VALIDATIONS #################
        #if not ValidateEQChecks(tc):
        #    return False

    elif step.step_id == 3:
        if not ValidatePostSyncCQChecks(tc):
            return False

    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.sq_cq_pre_qstate = copy.deepcopy(rs.lqp.sq_cq.qstate.data)

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    if (GlobalOptions.dryrun): return
    rs = tc.config.rdmasession
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.l_key)
    kt_entry.data = tc.pvtdata.mr_kt_entry.data
    kt_entry.WriteWithDelay()
    return
