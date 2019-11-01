#! /usr/bin/python3

from iris.test.rdma.utils import *
import pdb
import copy
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
from iris.config.objects.rdma.dcqcn_profile_table import *
def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    rs.lqp.rq.qstate.Read()
    rs.lqp.sq.qstate.data.congestion_mgmt_type = 1;
    rs.lqp.sq.qstate.WriteWithDelay()
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.msn = (tc.pvtdata.sq_pre_qstate.msn + 1)
    tc.pvtdata.sq_cindex = tc.pvtdata.sq_pre_qstate.c_index0

    logger.info("RDMA DCQCN State read/write")
    # Feeding timestamp from dcqcn_cb since model doesn't support timestamps.
    # With below timestamps 500K more tokens should be added to available tokens
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_pre_qstate = rs.lqp.dcqcn_data

    tc.pvtdata.dcqcn_profile = RdmaDcqcnProfileObject(rs.lqp.pd.ep.intf.lif, rs.lqp.rq.qstate.data.dcqcn_cfg_id)
    tc.pvtdata.pre_dcqcn_profile = copy.deepcopy(tc.pvtdata.dcqcn_profile.data)

    rs.lqp.dcqcn_data.last_sched_timestamp = 0xC350 # 50k ticks
    rs.lqp.dcqcn_data.cur_timestamp = 0x186A0 # 100k ticks
    rs.lqp.dcqcn_data.cur_avail_tokens = 200
    rs.lqp.dcqcn_data.token_bucket_size = 150000 #150kb
    # Setting target rate to 100 gbps and rate-enforced to 10 gbps
    rs.lqp.dcqcn_data.target_rate = 100000
    rs.lqp.dcqcn_data.rate_enforced = 10000
    rs.lqp.dcqcn_data.max_rate_reached = 0
    # Setting byte-count-threshold to 31 bytes to trigger dcqcn algorithm 
    # on sending 32 byte packet. Here dcqcn algorithm will be invoked for fast-recovery
    # since byte-counter-expiry count (which is 1) is less than F (which is 5) and timer-exp-cnt is 0.
    # After fast-recovery rate-enforced should be set to 55 gbps based on Rc = ((Rt + Rc) / 2)
    rs.lqp.dcqcn_data.byte_counter_thr = 31
    rs.lqp.dcqcn_data.timer_exp_cnt = 0
    rs.lqp.dcqcn_data.sq_cindex = tc.pvtdata.sq_cindex
    rs.lqp.WriteDcqcnCb()

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    # Read CQ pre state
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
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_post_qstate = rs.lqp.dcqcn_data 

    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

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
        if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index4', tc.pvtdata.sq_post_qstate, 'p_index4'):
            return False

        # verify that rp_num_byte_threshold_db is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'rp_num_byte_threshold_db', 1):
            return False

        ################# RQ verification ##############################
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rp_num_fast_recovery', 1):
            return False

        ################# DCQCN verification ###############################

        # verify that last_sched_timestamp in dcqcn state is set to cur_timestamp
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'last_sched_timestamp', 0x186A0):
            return False

        # verify that byte_counter_exp_cnt is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'byte_counter_exp_cnt', 1):
            return False

        # verify that target_rate is NOT incremented during fast-recovery.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 0):
            return False

        # verify that rate-enforced is set to 55 gbps after fast-recovery.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 55000):
            return False

    elif step.step_id == 1:
        # verify that rp_num_byte_threshold_db is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'rp_num_byte_threshold_db', 2):
            return False

        ################# RQ verification ##############################
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rp_num_fast_recovery', 2):
            return False

        # verify that byte_counter_exp_cnt is incremented again by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'byte_counter_exp_cnt', 2):
            return False

        # verify that target_rate is NOT incremented during fast-recovery.                            
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 0):                                                                                                      return False                           

        # verify that rate-enforced is incremented according to fast-recovery.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 77500):
            return False

    elif step.step_id == 2:
        # verify that rp_num_byte_threshold_db is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'rp_num_byte_threshold_db', 3):
            return False

        ################# RQ verification ##############################
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rp_num_fast_recovery', 3):
            return False

        # verify that byte_counter_exp_cnt is incremented again by 1.                                 
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'byte_counter_exp_cnt', 3):
            return False

        # verify that target_rate is NOT incremented during fast-recovery.                            
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 0):                                                                                                      return False

        # verify that rate-enforced is incremented according to fast-recovery.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 88750):
            return False

    elif step.step_id == 3:
        # verify that rp_num_byte_threshold_db is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'rp_num_byte_threshold_db', 4):
            return False

        ################# RQ verification ##############################
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rp_num_fast_recovery', 4):
            return False

        # verify that byte_counter_exp_cnt is incremented again by 1.                                 
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'byte_counter_exp_cnt', 4):
            return False

        # verify that target_rate is NOT incremented during fast-recovery.                            
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 0):                                                                                                      return False

        # verify that rate-enforced is incremented according to fast-recovery.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 94375):
            return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    tc.pvtdata.dcqcn_profile.data = copy.deepcopy(tc.pvtdata.pre_dcqcn_profile)
    tc.pvtdata.dcqcn_profile.WriteWithDelay()
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data.congestion_mgmt_type = 0;
    rs.lqp.sq.qstate.WriteWithDelay()
    return
