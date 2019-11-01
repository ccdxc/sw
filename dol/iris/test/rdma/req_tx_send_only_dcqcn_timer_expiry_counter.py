#! /usr/bin/python3

from iris.test.rdma.utils import *
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
    tc.pvtdata.dcqcn_profile_pre_qstate = copy.deepcopy(dcqcn_profile.data)
    dcqcn_profile.WriteWithDelay()

    # receive CNP packet to ring the first DCQCN_RATE_COMPUTE_RING doorbell, in cnp_recv_process alpha value will be set to 65535 initially, 
    logger.info("RDMA DCQCN State read/write")
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_pre_qstate = rs.lqp.dcqcn_data
    # Setting target rate to 100 gbps and rate-enforced to 10 gbps
    rs.lqp.dcqcn_data.cur_timestamp = rs.lqp.dcqcn_data.last_cnp_timestamp + 833 * (tc.pvtdata.dcqcn_profile_pre_qstate.rp_rate_reduce_monitor_period)
    rs.lqp.dcqcn_data.target_rate = 100000
    rs.lqp.dcqcn_data.rate_enforced = 10000
    rs.lqp.dcqcn_data.byte_counter_exp_cnt = 0
    rs.lqp.dcqcn_data.cur_byte_counter = 20
    rs.lqp.dcqcn_data.timer_exp_cnt = 0
    rs.lqp.dcqcn_data.num_alpha_exp_cnt = 5
    rs.lqp.dcqcn_data.byte_counter_thr = 1024
    rs.lqp.dcqcn_data.cur_avail_tokens = 10000
    # Feed initial integer alpha and g values.
    rs.lqp.dcqcn_data.alpha_value = 65535
    rs.lqp.dcqcn_data.max_rate_reached = 1
    rs.lqp.dcqcn_data.sq_cindex = tc.pvtdata.sq_cindex
    rs.lqp.WriteDcqcnCb()

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    # Read AQ pre state
    tc.pvtdata.lif = rs.lqp.pd.ep.intf.lif
    tc.pvtdata.aq = tc.pvtdata.lif.aq
    PopulateAdminPreQStates(tc)
    return

def TestCaseStepTrigger(tc, step):
    logger.info("RDMA TestCaseStepTrigger() Implementation with step_id: %d" % (step.step_id))
    if (GlobalOptions.dryrun): return True
    if step.step_id == 0:
        logger.info("RDMA TestCaseStepTrigger() - Setting the system time for FAST_TIMER to 0")
        timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
        timer.Step(0)

    if step.step_id == 1:
        logger.info("RDMA TestCaseStepTrigger() - Fast Forwarding the system time by by 56 ticks for FAST_TIMER wheel")
        timer = tc.infra_data.ConfigStore.objects.db['FAST_TIMER']
        timer.Step(56)
    return

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
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 10000):
            return False

            # verify that rate-enforced is cut by half.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 5000):
            return False

            # verify that num-cnp-received is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_rcvd', 1):
            return False

            # verify that num-cnp-processed is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_processed', 1):
            return False

            # verify that alpha value is set to 65535 based on dcqcn algorithm calculations.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'alpha_value', 65535):
            return False
    
    if step.step_id == 1:
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'alpha_value', 0):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'num_alpha_exp_cnt', 0):
            return False

        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'timer_exp_cnt', 1):
            return False

        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index5', 1):
            return False

            # verify that target-rate is set to rate-enforced.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 10000):
            return False

            # verify that rate-enforced is cut by half.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 7500):
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
    return

