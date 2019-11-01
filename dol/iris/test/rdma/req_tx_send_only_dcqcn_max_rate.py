#! /usr/bin/python3

from iris.config.objects.rdma.dcqcn_profile_table import *
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

    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = copy.deepcopy(rs.lqp.rq.qstate.data)
    rs.lqp.rq.qstate.data.congestion_mgmt_type = 1;
    rs.lqp.rq.qstate.WriteWithDelay()
    tc.pvtdata.test_timer = 1

    rs.lqp.sq.qstate.Read()
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    rs.lqp.sq.qstate.data.congestion_mgmt_type = 1;
    rs.lqp.sq.qstate.WriteWithDelay()
    tc.pvtdata.msn = (tc.pvtdata.sq_pre_qstate.msn + 1)
    tc.pvtdata.sq_cindex = tc.pvtdata.sq_pre_qstate.c_index0

   # Read DCQCN Profile pre state
    tc.pvtdata.lif = rs.lqp.pd.ep.intf.lif
    dcqcn_profile = RdmaDcqcnProfileObject(tc.pvtdata.lif, 0)
    dcqcn_profile.Read()
    # set rp_threshold to 0 to trigger additive_increase in step 2, set max_rate_reached to 1 and shut down timer at last in test.
    tc.pvtdata.dcqcn_profile_pre_qstate = copy.deepcopy(dcqcn_profile.data)
    dcqcn_profile.data.rp_threshold = 0
    dcqcn_profile.WriteWithDelay()

    logger.info("RDMA DCQCN State read/write")
    # Feeding timestamp from dcqcn_cb since model doesn't support timestamps.
    # With below timestamps 500K more tokens should be added to available tokens
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_pre_qstate = rs.lqp.dcqcn_data
    rs.lqp.dcqcn_data.cur_timestamp = rs.lqp.dcqcn_data.last_cnp_timestamp + 833 * (tc.pvtdata.dcqcn_profile_pre_qstate.rp_rate_reduce_monitor_period)
    rs.lqp.dcqcn_data.cur_avail_tokens = 200
    rs.lqp.dcqcn_data.token_bucket_size = 150000 #150kb
    rs.lqp.dcqcn_data.target_rate = 100000
    rs.lqp.dcqcn_data.rate_enforced = 200002
    # Setting byte-count-threshold to 31 bytes to trigger dcqcn algorithm 
    # on sending 32 byte packet. Here dcqcn algorithm will be invoked for fast-recovery
    # since byte-counter-expiry count (which is 0) is less than F (which is 5) and timer-exp-cnt is 0.
    # After fast-recovery rate-enforced should be set to 55 gbps based on Rc = ((Rt + Rc) / 2)

    # to kick start timer, first receive a CNP packet to trigger cnp_recv_process
    # byte_counter_exp_cnt and timer_exp_cnt will be set to 0 in cnp_recv_process, have to send out 6 packets to trigger additive_increase and branch to skip_target_rate_inc.
    rs.lqp.dcqcn_data.byte_counter_thr = 31
    rs.lqp.dcqcn_data.byte_counter_exp_cnt = 0
    rs.lqp.dcqcn_data.timer_exp_cnt = 0
    rs.lqp.dcqcn_data.max_rate_reached = 1
    rs.lqp.dcqcn_data.alpha_value = 65535
    rs.lqp.dcqcn_data.sq_cindex = tc.pvtdata.sq_cindex
    rs.lqp.WriteDcqcnCb()

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    # Read EQ pre state
    rs.lqp.eq.qstate.Read()
    tc.pvtdata.eq_pre_qstate = rs.lqp.eq.qstate.data

    return

def TestCaseStepTrigger(tc, step):
    if (GlobalOptions.dryrun): return True
    if step.step_id == 0:
        logger.info("RDMA TestCaseStepTrigger() - Setting the system time for FAST_TIMER to 0")
        timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
        timer.Step(0)

    if step.step_id == 2:
        logger.info("RDMA TestCaseStepTrigger() - Fast Forwarding the system time by by 56 ticks for FAST_TIMER wheel")
        timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
        timer.Step(56)
    return

def TestCaseVerify(tc):
    logger.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data
    rs.lqp.sq.qstate.Read()
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_post_qstate = rs.lqp.dcqcn_data

    if step.step_id == 0:
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

            #########################   Verify DCQCN params  ############################
            # verify that timer_exp_cnt is set to 0.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'timer_exp_cnt', 0):
            return False

            # verify that byte_counter_exp_cnt is set to 0.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'byte_counter_exp_cnt', 0):
            return False

            # verify that cur_byte_counter is set to 0.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'cur_byte_counter', 0):
            return False

            # verify that target-rate is set to rate-enforced.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 200002):
            return False

            # verify that rate-enforced is cut by half.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 100004):
            return False

            # verify that num-cnp-received is incremented by 1.

        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_rcvd', 1):
            return False

            # verify that num-cnp-processed is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_processed', 1):
            return False

    if step.step_id == 1:
        ################# DCQCN verification ##############################
        # verify that byte_counter_exp_cnt is set to 1.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'byte_counter_exp_cnt', 1):
            return False

        # verify that target-rate is unchanged
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 200002):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 150003):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'max_rate_reached', 1):
            return False

    if step.step_id == 2:
        # max_rate reached, timer will not restart
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'alpha_value', 0):
            return False

        if not VerifyFieldsEqual(tc, tc.pvtdata.rq_post_qstate, 'c_index5', tc.pvtdata.rq_post_qstate, 'p_index5'):
            return False
    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    #Disable congestion mgmt in qstate
    rs = tc.config.rdmasession
    # Restore rqcb/sqcb state
    rs.lqp.rq.qstate.Read()
    rs.lqp.rq.qstate.data = copy.deepcopy(tc.pvtdata.rq_pre_qstate)
    rs.lqp.rq.qstate.WriteWithDelay()
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data = copy.deepcopy(tc.pvtdata.sq_pre_qstate)
    rs.lqp.sq.qstate.WriteWithDelay()

    tc.pvtdata.lif = rs.lqp.pd.ep.intf.lif
    dcqcn_profile = RdmaDcqcnProfileObject(tc.pvtdata.lif, 0)
    dcqcn_profile.Read()
    dcqcn_profile.data = copy.deepcopy(tc.pvtdata.dcqcn_profile_pre_qstate)
    dcqcn_profile.WriteWithDelay()
    return
