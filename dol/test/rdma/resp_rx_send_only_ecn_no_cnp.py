#! /usr/bin/python3

from test.rdma.utils import *
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
    rs.lqp.rq.qstate.data.congestion_mgmt_enable = 1;
    rs.lqp.rq.qstate.Write()

    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    # Feeding timestamp from dcqcn_cb since model doesn't support timestamps.
    # CNP delay timer is set to 50us. Capri clock runs at 1 tick per ns.
    # So below clock values  should NOT generate CNP packet.
    rs.lqp.ReadDcqcnCb()
    rs.lqp.dcqcn_data.last_cnp_timestamp = 0xC350 # 50k ticks
    rs.lqp.dcqcn_data.cur_timestamp = 0x124f8 # 75k ticks
    rs.lqp.WriteDcqcnCb()

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

    ############     RQ VALIDATIONS #################
    # verify that e_psn is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 1):
        return False

    # verify that proxy_cindex is incremented by 1
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 1):
        return False

    # verify that token_id is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
        return False

    # verify that nxt_to_go_token_id is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
        return False

    # verify that busy is 0
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'busy', 0):
        return False

    ############     RQ STATS VALIDATIONS #################
    # verify that num_pkts is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_pkts', 1):
        return False

    # verify that num_bytes is incremented by pvtdata.num_total_bytes
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_bytes', tc.pvtdata.num_total_bytes):
        return False

    # verify that num_send_msgs is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'num_send_msgs', 1):
        return False

    # verify that num_pkts_in_cur_msg is 1
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'num_pkts_in_cur_msg', 1):
        return False

    # verify that max_pkts_in_any_msg is 1
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'max_pkts_in_any_msg', max([1, tc.pvtdata.rq_pre_qstate.max_pkts_in_any_msg])):
        return False

    ############     CQ VALIDATIONS #################
    if not ValidateRespRxCQChecks(tc):
        return False

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    # Disable congestion_mgmt in qstate
    rs.lqp.rq.qstate.Read()
    rs.lqp.rq.qstate.data.congestion_mgmt_enable = 0;
    rs.lqp.rq.qstate.Write()
    return
