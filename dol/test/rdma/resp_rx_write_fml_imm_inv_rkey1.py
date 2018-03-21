#! /usr/bin/python3

from test.rdma.utils import *
from config.objects.rdma.keytable import *
from infra.common.glopts import GlobalOptions
import random
from infra.common.logging import logger as logger
def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession
    tc.pvtdata.imm_data = random.randrange(0, 0xffffffff)

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data
    tc.pvtdata.r_key = rs.lqp.pd.mrs.Get('MR-SLAB0000').rkey

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data
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

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

    # verify that e_psn is incremented by 3
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 3):
        return False

    # verify that proxy_cindex is incremented by 1
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask,  1):
        return False

    # verify that token_id is incremented by 3
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 3):
        return False

    # verify that nxt_to_go_token_id is incremented by 3
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 3):
        return False

    # verify that busy is 0
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'busy', 0):
        return False


    ############     CQ VALIDATIONS #################
    if not ValidateRespRxCQChecks(tc):
        return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession

    #Restore remote write permissions on the RKEY of MR
    kt_entry = RdmaKeyTableEntryObject(rs.lqp.pd.ep.intf.lif, tc.pvtdata.r_key)
    kt_entry.data.acc_ctrl = tc.pvtdata.r_key_temp_acc_ctrl
    kt_entry.WriteWithDelay()

    return
