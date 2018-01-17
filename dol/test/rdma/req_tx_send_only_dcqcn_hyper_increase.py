#! /usr/bin/python3

from test.rdma.utils import *
import pdb
import copy
from infra.common.glopts import GlobalOptions

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    tc.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data.congestion_mgmt_enable = 1;
    rs.lqp.sq.qstate.Write()
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.msn = (tc.pvtdata.sq_pre_qstate.msn + 1)

    tc.info("RDMA DCQCN State read/write")
    # Feeding timestamp from dcqcn_cb since model doesn't support timestamps.
    # With below timestamps 500K more tokens should be added to available tokens
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_pre_qstate = rs.lqp.dcqcn_data
    rs.lqp.dcqcn_data.last_sched_timestamp = 0xC350 # 50k ticks
    rs.lqp.dcqcn_data.cur_timestamp = 0x186A0 # 100k ticks
    rs.lqp.dcqcn_data.cur_avail_tokens = 200
    rs.lqp.dcqcn_data.token_bucket_size = 150000 #150kb
    # Setting target rate to 90 gbps and rate-enforced to 80 gbps
    rs.lqp.dcqcn_data.target_rate = 90000
    rs.lqp.dcqcn_data.rate_enforced = 80000
    # Setting byte-count-threshold to 31 bytes to trigger dcqcn algorithm 
    # on sending 32 byte packet. Here dcqcn algorithm will be invoked for hyper-increase
    # since both byte-counter-expiry count (which is 1+5=6) and timer-expirty-cnt is 6 
    # is greater than F (which is 5)
    # After hyper-increase, target-rate should be set to 90.015 gbps as rate increases by
    # 5 * 3 = 15Mbps (3 is multiplicative factor to additive increase rate-increase of 5 Mbps.
    # rate-enforced should be set to 85.007 gbps based on Rc = ((Rt + Rc) / 2)
    rs.lqp.dcqcn_data.byte_counter_thr = 31
    rs.lqp.dcqcn_data.byte_counter_exp_cnt = 5
    rs.lqp.dcqcn_data.timer_exp_cnt = 6
    
    rs.lqp.WriteDcqcnCb()

    # ARM CQ and Set EQ's CI=PI for EQ enablement
    rs.lqp.sq_cq.qstate.ArmCq()
    rs.lqp.eq.qstate.reset_cindex(0)

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    # Read EQ pre state
    rs.lqp.eq.qstate.Read()
    tc.pvtdata.eq_pre_qstate = rs.lqp.eq.qstate.data

    return

def TestCaseTrigger(tc):
    tc.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    tc.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    tc.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    ring0_mask = (rs.lqp.num_sq_wqes - 1)
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_post_qstate = rs.lqp.dcqcn_data 

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
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index5', tc.pvtdata.sq_post_qstate, 'p_index5'):
            return False

        ################# DCQCN verification ##############################

        # verify that last_sched_timestamp in dcqcn state is set to cur_timestamp
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'last_sched_timestamp', 0x186A0):
            return False

        # verify that byte_counter_exp_cnt is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'byte_counter_exp_cnt', 1):
            return False

        # verify that target-rate is set to 90.015 gbps after additive-increase.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 90015):
            return False

        # verify that rate-enforced is set to 85.007 gbps after additive-increase.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 85007):
            return False

    elif step.step_id == 1:

        # verify that byte_counter_exp_cnt is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'byte_counter_exp_cnt', 2):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 90030):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 87518):
            return False

    elif step.step_id == 2:

        # verify that byte_counter_exp_cnt is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'byte_counter_exp_cnt', 3):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 90045):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 88781):
            return False

    elif step.step_id == 3:

        # verify that byte_counter_exp_cnt is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'byte_counter_exp_cnt', 4):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 90060):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 89420):
            return False

    return True

def TestCaseTeardown(tc):
    tc.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data.congestion_mgmt_enable = 0;
    rs.lqp.sq.qstate.Write()
    return
