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

    tc.pvtdata.num_total_bytes = 0x40

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.congestion_mgmt_type_pre = rs.lqp.rq.qstate.data.congestion_mgmt_type
    rs.lqp.rq.qstate.data.congestion_mgmt_type = 1;
    rs.lqp.rq.qstate.WriteWithDelay()

    tc.pvtdata.dcqcn_profile = RdmaDcqcnProfileObject(rs.lqp.pd.ep.intf.lif, rs.lqp.rq.qstate.data.dcqcn_cfg_id)
    tc.pvtdata.pre_dcqcn_profile = copy.deepcopy(tc.pvtdata.dcqcn_profile.data)
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data
    tc.pvtdata.rq_post_qstate = tc.pvtdata.rq_pre_qstate

    logger.info("RDMA DCQCN State read/write")
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_pre_qstate = rs.lqp.dcqcn_data
    tc.pvtdata.dcqcn_profile = RdmaDcqcnProfileObject(rs.lqp.pd.ep.intf.lif, rs.lqp.rq.qstate.data.dcqcn_cfg_id)
    tc.pvtdata.pre_dcqcn_profile = copy.deepcopy(tc.pvtdata.dcqcn_profile.data)
    # Setting target rate to 100 gbps and rate-enforced to 10 gbps
    rs.lqp.dcqcn_data.target_rate = 100000
    rs.lqp.dcqcn_data.rate_enforced = 10000
    rs.lqp.dcqcn_data.byte_counter_exp_cnt = 50
    rs.lqp.dcqcn_data.cur_byte_counter = 20
    rs.lqp.dcqcn_data.timer_exp_cnt = 40
    # Feed initial integer alpha and g values.
    rs.lqp.dcqcn_data.alpha_value = 65535
    rs.lqp.dcqcn_data.g_val = 256
    tc.pvtdata.dcqcn_profile.data.rp_rate_reduce_monitor_period = 50
    rs.lqp.dcqcn_data.cur_timestamp = rs.lqp.dcqcn_data.last_cnp_timestamp +  (833 * tc.pvtdata.dcqcn_profile.data.rp_rate_reduce_monitor_period) - 1 # 1us = 833 ticks. cur_timestamp < last_cnp_timestamp + 50us

    rs.lqp.WriteDcqcnCb()

    tc.pvtdata.dcqcn_post_qstate = tc.pvtdata.dcqcn_pre_qstate
    # Clamp target rate when cnp is received
    tc.pvtdata.dcqcn_profile.data.rp_clamp_flags = 1
    tc.pvtdata.dcqcn_profile.WriteWithDelay()
    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    tc.pvtdata.rq_pre_qstate = tc.pvtdata.rq_post_qstate
    tc.pvtdata.dcqcn_pre_qstate = tc.pvtdata.dcqcn_post_qstate
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_post_qstate = rs.lqp.dcqcn_data

    if step.step_id == 0:
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False
    
        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
            return False
    
            # verify that rp_cnp_processed is not incremented.
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rp_cnp_processed', 1):
            return False
    
            #########################   Verify DCQCN params  ############################
            # verify that timer_exp_cnt is unchanged.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'timer_exp_cnt', 0):
            return False

            # verify that byte_counter_exp_cnt is unchanged.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'byte_counter_exp_cnt', 0):
            return False

            # verify that cur_byte_counter is unchanged.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'cur_byte_counter', 0):
            return False

            # verify that target-rate is unchanged.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 0):
            return False

            # verify that rate-enforced is unchanged.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 0):
            return False

            # verify that num-cnp-received is unchanged.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_rcvd', 0):
            return False

            # verify that num-cnp-processed is unchanged.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_processed', 0):
            return False

            # verify that alpha value is set to 65535 based on dcqcn algorithm calculations.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'alpha_value', 65535):
            return False

        # Advance timestamp to accomodate next CNP
        rs.lqp.dcqcn_data.cur_timestamp = rs.lqp.dcqcn_data.cur_timestamp + ((tc.pvtdata.dcqcn_profile.data.rp_rate_reduce_monitor_period + 1) * 833) # cur_timestamp > last_cnp_timestamp + 50us
        rs.lqp.WriteDcqcnCb()

    elif step.step_id == 1:
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False
    
        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        # verify that rp_cnp_processed is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rp_cnp_processed', 1):
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

            # verify that target-rate is clamped.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 100000):
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

    elif step.step_id == 2:
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False
    
        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        # verify that rp_cnp_processed is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rp_cnp_processed', 1):
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

            # verify that target-rate is unchanged.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 100000):
            return False

            # verify that rate-enforced is unchanged.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 5000):
            return False

            # verify that num-cnp-received is unchanged.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_rcvd', 0):
            return False

            # verify that num-cnp-processed is unchanged.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_processed', 0):
            return False

            # verify that alpha value is set to 65535 based on dcqcn algorithm calculations.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'alpha_value', 65535):
            return False

        # Advance timestamp to accomodate next CNP
        rs.lqp.dcqcn_data.cur_timestamp = rs.lqp.dcqcn_data.cur_timestamp + ((tc.pvtdata.dcqcn_profile.data.rp_rate_reduce_monitor_period + 1) * 833) # cur_timestamp > last_cnp_timestamp + 50us
        rs.lqp.WriteDcqcnCb()

    elif step.step_id == 3:
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False
    
        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        # verify that rp_cnp_processed is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rp_cnp_processed', 1):
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

            # verify that target-rate is clamped.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 100000):
            return False

            # verify that rate-enforced is cut by half.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 2500):
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

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    #Disable congestion mgmt in qstate
    rs = tc.config.rdmasession
    tc.pvtdata.dcqcn_profile.data = copy.deepcopy(tc.pvtdata.pre_dcqcn_profile)
    tc.pvtdata.dcqcn_profile.WriteWithDelay()
    rs.lqp.rq.qstate.Read()                        
    rs.lqp.rq.qstate.data.congestion_mgmt_type = tc.pvtdata.congestion_mgmt_type_pre;
    rs.lqp.rq.qstate.WriteWithDelay()                       
    return
