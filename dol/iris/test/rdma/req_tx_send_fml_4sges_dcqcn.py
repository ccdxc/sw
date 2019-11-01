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
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    rs.lqp.sq.qstate.Read()
    rs.lqp.rq.qstate.Read()
    rs.lqp.sq.qstate.data.congestion_mgmt_type = 1;
    rs.lqp.sq.qstate.WriteWithDelay()

    logger.info("RDMA DCQCN State read/write")
    # Feeding timestamp from dcqcn_cb since model doesn't support timestamps.
    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_pre_qstate = rs.lqp.dcqcn_data

    tc.pvtdata.dcqcn_profile = RdmaDcqcnProfileObject(rs.lqp.pd.ep.intf.lif, rs.lqp.rq.qstate.data.dcqcn_cfg_id)
    tc.pvtdata.pre_dcqcn_profile = copy.deepcopy(tc.pvtdata.dcqcn_profile.data)

    rs.lqp.dcqcn_data.last_sched_timestamp = 0x0
    rs.lqp.dcqcn_data.cur_timestamp = 0x12AB0D8 # 19575k ticks
    rs.lqp.dcqcn_data.rate_enforced = 1  # 1 Mbps (rate_enforced is in Mbps)
    rs.lqp.dcqcn_data.cur_avail_tokens = 0
    rs.lqp.dcqcn_data.num_sched_drop = 0
    rs.lqp.dcqcn_data.token_bucket_size = 150000 #150kb
    rs.lqp.dcqcn_data.byte_counter_thr = 3072
    tc.pvtdata.dcqcn_profile.data.rp_byte_reset = 3072
    tc.pvtdata.dcqcn_profile.WriteWithDelay()
    rs.lqp.dcqcn_data.delta_tokens_last_sched = 0
    rs.lqp.WriteDcqcnCb()
    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    ring0_mask = (rs.lqp.num_sq_wqes - 1)
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

    rs.lqp.ReadDcqcnCb()
    tc.pvtdata.dcqcn_post_qstate = rs.lqp.dcqcn_data

    # verify that tx_psn is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 3):
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

    # verify that cur_avail_tokens in dcqcn state is 75. Since at end of 5 iteration 4*120.048 + 19575k / 833 = 23499 + 480.192 = 23979 bits will be accumulated 
    # with core-clock running at 833 MHz.
    # 8*2988=23904  bits will be consumed by packet. So 23979-23904=75 tokens remain.
    if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'cur_avail_tokens', 75):
        return False

    # verify that last_sched_timestamp in dcqcn state is set to cur_timestamp of iteration 5 which is 19975k ticks
    if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'last_sched_timestamp', 0x130CB58):
        return False

    # verify that packets dropped = 4 to accumulate the required tokens to send out Middle and Last packets eventually.
    # It requires 5 iterations to accumulate enough tokens to send 2988B packet at rate 1 Mbps,
    # for clock which ticks 833 times per us and program scheduled every 100K ticks.
    if not VerifyFieldAbsolute(tc, tc.pvtdata.dcqcn_post_qstate, 'num_sched_drop', 4):
        return False
    
    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    #Disable congestion mgmt in qstate
    rs = tc.config.rdmasession
    tc.pvtdata.dcqcn_profile.data = copy.deepcopy(tc.pvtdata.pre_dcqcn_profile)
    tc.pvtdata.dcqcn_profile.WriteWithDelay()
    # Restore rqcb/sqcb state
    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data = copy.deepcopy(tc.pvtdata.sq_pre_qstate)
    rs.lqp.sq.qstate.WriteWithDelay()
    rs.lqp.ReadDcqcnCb()
    rs.lqp.dcqcn_data = tc.pvtdata.dcqcn_pre_qstate
    rs.lqp.WriteDcqcnCb()
    return
