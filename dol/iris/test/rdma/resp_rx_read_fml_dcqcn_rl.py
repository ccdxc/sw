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

    rs.lqp.rq.qstate.data.congestion_mgmt_type = 1;
    rs.lqp.rq.qstate.WriteWithDelay()

    logger.info("RDMA DCQCN State read/write")
    # Feeding timestamp from dcqcn_cb since model doesn't support timestamps.
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_pre_qstate = rs.lqp.dcqcn_data

    tc.pvtdata.dcqcn_profile = RdmaDcqcnProfileObject(rs.lqp.pd.ep.intf.lif, rs.lqp.rq.qstate.data.dcqcn_cfg_id)
    tc.pvtdata.pre_dcqcn_profile = copy.deepcopy(tc.pvtdata.dcqcn_profile.data)

    rs.lqp.dcqcn_data.last_sched_timestamp = 0x0
    rs.lqp.dcqcn_data.cur_timestamp = 0x7D0000 # 8192k ticks
    rs.lqp.dcqcn_data.rate_enforced = 1  # 1 Mbps (rate_enforced is in Mbps)
    rs.lqp.dcqcn_data.cur_avail_tokens = 0
    rs.lqp.dcqcn_data.num_sched_drop = 0
    rs.lqp.dcqcn_data.token_bucket_size = 150000 #150kb
    rs.lqp.dcqcn_data.byte_counter_thr = 3072
    tc.pvtdata.dcqcn_profile.data.rp_byte_reset = 3072
    tc.pvtdata.dcqcn_profile.WriteWithDelay()
    rs.lqp.dcqcn_data.delta_tokens_last_sched = 0
    rs.lqp.WriteDcqcnCb()
    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data
    return

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
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_post_qstate = rs.lqp.dcqcn_data

    ############     RQ VALIDATIONS #################
    # verify that e_psn is incremented by 3
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 3):
        return False

    # verify that p_index is not incremented
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index0', ring0_mask,  0):
        return False

    # verify that c_index is not incremented
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask,  0):
        return False

    # verify that token_id is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
        return False

    # verify that nxt_to_go_token_id is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
        return False

    ############     RSQ VALIDATIONS #################
    logger.info("RSQ VALIDATIONS:")
    ring1_mask = (rs.lqp.num_rsq_wqes - 1)
    # verify that p_index is incremented by 1
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index1', ring1_mask,  1):
        return False

    # verify that c_index is incremented by 1
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'c_index1', ring1_mask,  1):
        return False


    # verify that cur_avail_tokens in dcqcn state is 7680. Since at end of 2 iteration 2*9834=19668 bits will be accumulated 
    # with core-clock running at 833 MHz.
    # 8*2112=16896  bits will be consumed by packet. So 19668-16896=2772 tokens remain.
    if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'cur_avail_tokens', 2772):
        return False

    # verify that last_sched_timestamp in dcqcn state is set to cur_timestamp of iteration 2 which is 2*8192000 ticks
    if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'last_sched_timestamp', 0xFA0000):
        return False

    # verify that packets dropped = 1 to accumulate the required tokens to send out Middle and Last packets eventually.
    # It requires 1 iterations to accumulate enough tokens to send 1024B packet at rate 1 Mbps,
    # for clock which ticks 833 times per us and program scheduled every 8192K ticks.
    if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'num_sched_drop', 1):
        return False

   ############     CQ VALIDATIONS #################
    if not ValidateNoCQChanges(tc):
        return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    tc.pvtdata.dcqcn_profile.data = copy.deepcopy(tc.pvtdata.pre_dcqcn_profile)
    tc.pvtdata.dcqcn_profile.WriteWithDelay()
    rs.lqp.rq.qstate.Read()
    rs.lqp.rq.qstate.data.congestion_mgmt_type = 0;
    rs.lqp.rq.qstate.WriteWithDelay()
    return
