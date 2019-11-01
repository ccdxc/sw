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
    tc.pvtdata.sq_pre_qstate_save = copy.deepcopy(rs.lqp.sq.qstate.data)
    # Enable DCQCN
    rs.lqp.sq.qstate.data.congestion_mgmt_type = 1;
    rs.lqp.sq.qstate.WriteWithDelay()
    tc.pvtdata.sq_cindex = tc.pvtdata.sq_pre_qstate.c_index0

    tc.pvtdata.write_rkey =  10
    tc.pvtdata.write_dma_len = 2112
    tc.pvtdata.write_va = 0x1122334455667788

    tc.pvtdata.read1_rkey =  33
    tc.pvtdata.read1_dma_len = 4095
    tc.pvtdata.read1_va = 0x1112131415161718

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    logger.info("RDMA DCQCN State read/write")
    # Feeding timestamp from dcqcn_cb since model doesn't support timestamps.
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_pre_qstate = rs.lqp.dcqcn_data
    rs.lqp.dcqcn_data.last_sched_timestamp = 0x0
    rs.lqp.dcqcn_data.cur_timestamp = 0x186A0 # 100k ticks
    rs.lqp.dcqcn_data.rate_enforced = 70  # 70 Mbps (rate_enforced is in Mbps)
    rs.lqp.dcqcn_data.cur_avail_tokens = 0
    rs.lqp.dcqcn_data.num_sched_drop = 0
    rs.lqp.dcqcn_data.token_bucket_size = 150000 #150kb
    rs.lqp.dcqcn_data.sq_cindex = tc.pvtdata.sq_cindex
    rs.lqp.WriteDcqcnCb()
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
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_post_qstate = rs.lqp.dcqcn_data 

    if step.step_id == 0:
        # verify that tx_psn is incremented by 11
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 10):
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
            if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'lsn', 1):
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

        # verify that cur_avail_tokens in dcqcn state is 8208. Since at end of each iteration 8400 bits (5 times) will be accumulated.
        # with core-clock frequency of 833 ticks per us and rate of 70 Mbps (120 * 70).
        # 8192 bits will be consumed by Send, Write FM packets and 512 bits by Send, Write Last packets. Total = 33792 bits.
        # So 42000-33792 = 8208 tokens remain.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'cur_avail_tokens',8208): 
            return False

        # verify that last_sched_timestamp in dcqcn state is set to cur_timestamp of iteration 5 which is 500k ticks
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'last_sched_timestamp', 0x7A120):
            return False

        # verify that packet was dropped 4 times to accumulate the required tokens to send it out eventually.
        # It requires total 5 iterations to accumulate enough tokens to send 64B packet at rate 1 Mbps,
        # for clock which ticks 833 times per us and program scheduled every 100K ticks.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'num_sched_drop',4):
            return False

    elif step.step_id == 1:
        # verify that msn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'msn', tc.pvtdata.sq_post_qstate, 'msn'):
            return False

        # verify that p_index of rrq is bktracked
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'p_index4', 1):
            return False

        # verify that c_index of rrq is bktracked 
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'c_index4', 0):
            return False

        # verify rexmit_psn incremented is  increment by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'rexmit_psn', 1):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 1
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
     
        # verify that SQ p_index is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index0', tc.pvtdata.sq_post_qstate, 'p_index0'):
            return False

        # verify that SQ c_index remains same after bktracking and retransmit
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index0', tc.pvtdata.sq_post_qstate, 'c_index0'):
            return False

        # verify that cur_avail_tokens in dcqcn state is 7808. Since at end of each iteration 8400 bits (3 times) will be accumulated.
        # with core-clock frequency of 833 ticks per us. 8208 bits roll over from previous step.
        # 8192 bits will be consumed by Send M, Write FM packets and 512 bits by Send, Write Last packets. Total = 25600 bits.
        # So 25200 + 8208 - 25600 = 7808 tokens remain.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'cur_avail_tokens',7808): 
            return False

        # verify that last_sched_timestamp in dcqcn state is set to cur_timestamp of iteration 8 which is 800k ticks
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'last_sched_timestamp', 0xC3500):
            return False

        # verify that packet was dropped 4 times to accumulate the required tokens to send it out eventually.
        # It requires total 5 iterations to accumulate enough tokens to send 64B packet at rate 1 Mbps,
        # for clock which ticks 833 times per us and program scheduled every 100K ticks.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'num_sched_drop',7):
            return False

    elif step.step_id == 2:
        # verify that msn is incremented by 2
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'msn', 2):
            return False

        # verify that p_index of rrq is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index4', tc.pvtdata.sq_post_qstate, 'p_index4'):
            return False

        # verify that c_index of rrq is incremented by 1
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index4', tc.pvtdata.sq_post_qstate, 'c_index4'):
            return False

        # verify that tx_psn is not incremented
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'tx_psn'):
            return False

        # verify rexmit_psn is incremented by 5
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'rexmit_psn', 5):
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
        
        # validate cqcb pindex and color
        if not ValidateReqRxCQChecks(tc, 'EXP_CQ_DESC_1'):
            return False

    elif step.step_id == 3:
        # verify that msn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'msn', 1):
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

        # verify rexmit_psn is incremented to tx_psn
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_post_qstate, 'tx_psn', tc.pvtdata.sq_post_qstate, 'rexmit_psn'):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'busy', 0):
            return False

        # verify that in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'in_progress', 0):
            return False

        # verify that token_id is incremented by 4
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'token_id', 4):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'nxt_to_go_token_id', 4):
            return False
     
        # verify that SQ p_index is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index0', tc.pvtdata.sq_post_qstate, 'p_index0'):
            return False

        # verify that SQ c_index is not modified
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'c_index0', tc.pvtdata.sq_post_qstate, 'c_index0'):
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
    #Disable congestion mgmt in qstate
    rs = tc.config.rdmasession
    # Restore rqcb/sqcb state
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data = copy.deepcopy(tc.pvtdata.sq_pre_qstate_save)
    rs.lqp.sq.qstate.WriteWithDelay()
    rs.lqp.ReadDcqcnCb()
    rs.lqp.dcqcn_data = tc.pvtdata.dcqcn_pre_qstate
    rs.lqp.WriteDcqcnCb()
    return
