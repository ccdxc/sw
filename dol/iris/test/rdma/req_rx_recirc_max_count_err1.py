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
    tc.pvtdata.va = 0x0102030405060708
    tc.pvtdata.dma_len = 64
    tc.pvtdata.r_key = 2
    tc.pvtdata.msn = (tc.pvtdata.sq_pre_qstate.msn + 1)
    # Program invalid nxt_to_go_token_id so that incoming-packet goes for recirc
    rs.lqp.sq.qstate.data.nxt_to_go_token_id = (rs.lqp.rq.qstate.data.nxt_to_go_token_id - 1) & 0xFF;
    rs.lqp.sq.qstate.Write()

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
    logger.info("RDMA TestCaseStepVerify() Implementation for Step: %d" %step.step_id)
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    ring0_mask = (rs.lqp.num_sq_wqes - 1)
    ring4_mask = (rs.lqp.num_rrq_wqes - 1)
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data
    rs.lqp.rq.qstate.data.nxt_to_go_token_id = (rs.lqp.rq.qstate.data.nxt_to_go_token_id - 1) & 0xFF;        
    if step.step_id == 0:
        # verify that tx_psn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 1):
            return False

        # verify that p_index sq is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index0', ring0_mask, 1):
            return False

        # verify that c_index sq is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index0', ring0_mask, 1):
            return False

        # verify that p_index of rrq is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'p_index4', ring4_mask, 1):
            return False

        # verify that ssn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'ssn', 1):
            return False

        # verify that lsn incremented
        if tc.pvtdata.sq_pre_qstate.disable_credits != 1:
            if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'lsn', 1):
                return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

    elif step.step_id == 1:
        msn = tc.pvtdata.sq_pre_qstate.ssn - 1

        # verify that c_index of rrq is not incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'c_index4', ring4_mask, 0):
            return False

        # verify rexmit_psn is not incremented
        if not VerifyFieldModify(tc, tc.pvtdata.sq_post_qstate, tc.pvtdata.sq_post_qstate, 'rexmit_psn', 0):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that tx_psn is not incremented
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 0):
            return False

        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is not incremented
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 0):
            return False

         # make sure sqcb1 state has moved to ERR 
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb1_state', 2):
            return False

         # make sure sqcb0 state has also moved to ERR 
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'state', 2):
            return False

        # verify that max_recirc_cnt_err is captured
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb5_max_recirc_cnt_err', 1):
            return False 

         # recirc reason should be 2 (inorder_work_done) 
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb5_recirc_reason', 2):
            return False 

         # recirc bth opcode should be 16 (READ-RESP-ONLY) 
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb5_recirc_bth_opcode', 16):
            return False

        # recirc bth psn should be the psn of the packet (which is same as pre_qstate.tx_psn - 1)
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'sqcb5_recirc_bth_psn',
                                   tc.pvtdata.sq_pre_qstate.tx_psn - 1):
            return False

    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    return True


def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    # update nxt_to_go_token_id so that next request can be processed on this QP
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data.nxt_to_go_token_id = 0;
    rs.lqp.sq.qstate.data.token_id = 0;
    rs.lqp.sq.qstate.WriteWithDelay()
    ResetErrQState(tc)
    return
