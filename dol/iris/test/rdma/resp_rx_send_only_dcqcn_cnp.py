#! /usr/bin/python3

from iris.test.rdma.utils import *
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
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
    rs.lqp.rq.qstate.data.congestion_mgmt_type = 1;
    rs.lqp.rq.qstate.WriteWithDelay()

    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    logger.info("RDMA DCQCN State read/write")
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_pre_qstate = rs.lqp.dcqcn_data
    # Setting target rate to 100 gbps and rate-enforced to 10 gbps
    rs.lqp.dcqcn_data.target_rate = 100000
    rs.lqp.dcqcn_data.rate_enforced = 10000
    rs.lqp.dcqcn_data.byte_counter_exp_cnt = 50
    rs.lqp.dcqcn_data.cur_byte_counter = 20
    rs.lqp.dcqcn_data.timer_exp_cnt = 40
    # Feed initial integer alpha and g values.
    rs.lqp.dcqcn_data.alpha_value = 65535
    rs.lqp.dcqcn_data.g_val = 256

    rs.lqp.WriteDcqcnCb()

    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
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

    elif step.step_id == 1:
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 2):
            return False
    
        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 2):
            return False

        # verify that rp_cnp_processed is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rp_cnp_processed', 2):
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
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 5000):
            return False

            # verify that rate-enforced is cut by half.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 2500):
            return False

            # verify that num-cnp-received is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_rcvd', 2):
            return False

            # verify that num-cnp-processed is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_processed', 2):
            return False

            # verify that alpha value is set to 65535 based on dcqcn algorithm calculations.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'alpha_value', 65535):
            return False

    elif step.step_id == 2:
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 3):
            return False
    
        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 3):
            return False

        # verify that rp_cnp_processed is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rp_cnp_processed', 3):
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
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 2500):
            return False

            # verify that rate-enforced is cut by half.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 1250):
            return False

            # verify that num-cnp-received is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_rcvd', 3):
            return False

            # verify that num-cnp-processed is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_processed', 3):
            return False

            # verify that alpha value is set to 65535 based on dcqcn algorithm calculations.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'alpha_value', 65535):
            return False

    elif step.step_id == 3:
        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 4):
            return False
    
        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 4):
            return False

        # verify that rp_cnp_processed is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rp_cnp_processed', 4):
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
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'target_rate', 1250):
            return False

            # verify that rate-enforced is cut by half.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'rate_enforced', 625):
            return False

            # verify that num-cnp-received is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_rcvd', 4):
            return False

            # verify that num-cnp-processed is incremented by 1.
        if not VerifyFieldModify(tc, tc.pvtdata.dcqcn_pre_qstate, tc.pvtdata.dcqcn_post_qstate, 'num_cnp_processed', 4):
            return False

            # verify that alpha value is set to 65535 based on dcqcn algorithm calculations.
        if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'alpha_value', 65535):
            return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    #Disable congestion mgmt in qstate
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()                        
    rs.lqp.rq.qstate.data.congestion_mgmt_type = 0;                                                 
    rs.lqp.rq.qstate.WriteWithDelay()                       
    return
