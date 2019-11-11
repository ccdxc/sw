#! /usr/bin/python3

from iris.test.rdma.utils import *
from iris.config.objects.rdma.keytable import *
from infra.common.glopts import GlobalOptions
import random
from infra.common.logging import logger as logger
def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    PopulatePreQStates(tc)

    rs = tc.config.rdmasession
    tc.pvtdata.imm_data = random.randrange(0, 0xffffffff)
    tc.pvtdata.r_key = rs.lqp.pd.mrs.Get('MR-SLAB0000').rkey
    tc.pvtdata.mw_kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, (tc.pvtdata.r_key))

    return

def TestCaseStepTrigger(tc, step):
    logger.info("RDMA TestCaseStepTrigger() Implementation - step_id: %d." % step.step_id)
    rs = tc.config.rdmasession

    if step.step_id == 1:
        #Disable remote write permissions on the RKEY of MR-SLAB0001    
        kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.r_key)
        tc.pvtdata.r_key_temp_acc_ctrl = kt_entry.data.acc_ctrl
        kt_entry.data.acc_ctrl &= ~0x2 #ACC_CTRL_REMOTE_WRITE
        kt_entry.WriteWithDelay()


    
def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    PopulatePostQStates(tc)

    rs = tc.config.rdmasession
    ring0_mask = (rs.lqp.num_rq_wqes - 1)

    if step.step_id == 1:
    
        # verify that e_psn is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 3):
            return False
    
        # verify that proxy_cindex is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask,  1):
            return False
    
        # verify that token_id is incremented by 3
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 3):
            return False
    
        # verify that nxt_to_go_token_id is incremented by 2 (as the last packet
        # causes the error disable qp)
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 2):
            return False
    
        # verify that state is now moved to ERR (2)
        if not VerifyErrQState(tc):
            return False
 
        ############     STATS VALIDATIONS #################
        # verify that error disable stats are updated
        if not VerifyErrStatistics(tc):
            return False
    
        #verify that key_acc_ctrl_err is set to 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'qp_err_dis_key_acc_ctrl_err', 1):                                           
            return False
       
        # last bth opcode should be 9 (write_last_with_imm)
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'last_bth_opcode', 9):
            return False

        ############     CQ VALIDATIONS #################
        if not ValidateCQCompletions(tc, 1, 1):
            return False

    elif step.step_id == 2:

        if not ValidatePostSyncCQChecks(tc):
            return False 

    return True

def TestCaseTeardown(tc):
    if (GlobalOptions.dryrun): return
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    ResetErrQState(tc)

    #Restore remote write permissions on the RKEY of MR
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.r_key)
    kt_entry.data = tc.pvtdata.mw_kt_entry.data
    kt_entry.WriteWithDelay()

    return
