#! /usr/bin/python3

from iris.test.rdma.utils import *
from iris.config.objects.rdma.dcqcn_profile_table import *
import pdb
import copy
import random
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
import rdma_pb2	as rdma_pb2

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.rq_pre_qstate = copy.deepcopy(rs.lqp.rq.qstate.data)

    rs.lqp.rq.qstate.data.access_flags = 0
    rs.lqp.rq.qstate.WriteWithDelay()
    tc.pvtdata.test_qp = rs.lqp

    # Set attr_mask for mod_qp
    tc.pvtdata.attr_mask = 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_RQ_PSN
    tc.pvtdata.attr_mask |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_AV
    tc.pvtdata.attr_mask |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_RETRY_CNT
    tc.pvtdata.attr_mask |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_RNR_RETRY
    tc.pvtdata.attr_mask |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_ACCESS_FLAGS

    # Setup values to send down to mod_qp
    if tc.pvtdata.rq_pre_qstate.e_psn != 0:
        tc.pvtdata.rq_psn = random.randint(0, tc.pvtdata.rq_pre_qstate.e_psn + 1)
    else:
        tc.pvtdata.rq_psn = 10  # random
    tc.pvtdata.ah_id_len = rs.lqp.ah_handle | len(rs.lqp.HdrTemplate) << 24
    if not GlobalOptions.dryrun:
        tc.pvtdata.dma_addr = rs.lqp.hdr_slab.phy_address[0]
    else:
        tc.pvtdata.dma_addr = 0
    # retry = (rnr_retry_count << 4) + err_retry_count
    tc.pvtdata.rnr_retry = (tc.pvtdata.sq_pre_qstate.rnr_retry_count + 4) % 8
    tc.pvtdata.err_retry = (tc.pvtdata.sq_pre_qstate.err_retry_count + 3) % 8
    tc.pvtdata.retry = (tc.pvtdata.rnr_retry << 4) + tc.pvtdata.err_retry

    # Assuming SQ, RQ share CQ
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.qp_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    tc.pvtdata.lif = rs.lqp.pd.ep.intf.lif
    tc.pvtdata.aq = tc.pvtdata.lif.aq
    PopulateAdminPreQStates(tc)

    tc.pvtdata.dcqcn_profile = RdmaDcqcnProfileObject(tc.pvtdata.lif, 0).data
    tc.pvtdata.pre_dcqcn_data = copy.deepcopy(rs.lqp.dcqcn_data)
    tc.pvtdata.rate_enforced = tc.pvtdata.dcqcn_profile.rp_qp_rate
    tc.pvtdata.access_flags = 11 # REMOTE_WRITE | REMOTE_READ and 1 bit out of range
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
    tc.pvtdata.aq.aq.qstate.Read()
    rs.lqp.sq.qstate.Read()
    rs.lqp.rq.qstate.Read()
    rs.lqp.ReadDcqcnCb()
    ring0_mask = (tc.pvtdata.aq.num_aq_wqes - 1)
    tc.pvtdata.aq_post_qstate = tc.pvtdata.aq.aq.qstate.data
    if step.step_id == 1:
        # verify that token_id is equal to next_token_id
        if not VerifyFieldsEqual(tc, tc.pvtdata.aq_post_qstate, 'token_id', tc.pvtdata.aq_post_qstate, 'next_token_id'):
            return False

        # verify that p_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.aq_pre_qstate, tc.pvtdata.aq_post_qstate, 'p_index0', ring0_mask,  1):
            return False

        # verify that c_index is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.aq_pre_qstate, tc.pvtdata.aq_post_qstate, 'c_index0', ring0_mask, 1):
            return False

        if not ValidateAdminCQChecks(tc, 'EXP_CQ_DESC'):
            return False

        # verify that rq_psn is set in rqcb1
        if not VerifyFieldAbsolute(tc, rs.lqp.rq.qstate.data, 'e_psn', tc.pvtdata.rq_psn):
            return False

        # verify that rnr_retry is set in sqcb1 and sqcb2
        if not VerifyFieldAbsolute(tc, rs.lqp.sq.qstate.data, 'rnr_retry_count', tc.pvtdata.rnr_retry):
            return False
        if not VerifyFieldAbsolute(tc, rs.lqp.sq.qstate.data, 'rnr_retry_ctr', tc.pvtdata.rnr_retry):
            return False

        # verify that err_retry is set in sqcb1 and sqcb2
        if not VerifyFieldAbsolute(tc, rs.lqp.sq.qstate.data, 'err_retry_count', tc.pvtdata.err_retry):
            return False
        if not VerifyFieldAbsolute(tc, rs.lqp.sq.qstate.data, 'err_retry_ctr', tc.pvtdata.err_retry):
            return False

        # verify that access_flags is set in rqcb1
        if not VerifyFieldAbsolute(tc, rs.lqp.rq.qstate.data, 'access_flags', tc.pvtdata.access_flags & 0x3):
            return False

        # verify that dcqcn cb values are sane
        if not VerifyFieldAbsolute(tc, rs.lqp.dcqcn_data, 'rate_enforced', tc.pvtdata.rate_enforced):
            return False

        if not VerifyFieldAbsolute(tc, rs.lqp.dcqcn_data, 'target_rate', 100000):
            return False

        if not VerifyFieldAbsolute(tc, rs.lqp.dcqcn_data, 'token_bucket_size', 150000):
            return False

        if not VerifyFieldAbsolute(tc, rs.lqp.dcqcn_data, 'cur_avail_tokens', 150000):
            return False

        if not VerifyFieldsEqual(tc, rs.lqp.dcqcn_data, 'byte_counter_thr', tc.pvtdata.dcqcn_profile, 'rp_byte_reset'):
            return False

        if not VerifyFieldsEqual(tc, rs.lqp.dcqcn_data, 'alpha_value', tc.pvtdata.dcqcn_profile, 'rp_initial_alpha_value'):
            return False

        if not VerifyFieldAbsolute(tc, rs.lqp.dcqcn_data, 'log_sq_size', rs.lqp.log_sq_size):
            return False

    elif step.step_id == 2:

        if not ValidatePostSyncAdminCQChecks(tc):
            return False 

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.data = copy.deepcopy(tc.pvtdata.sq_pre_qstate)
    rs.lqp.sq.qstate.WriteWithDelay()
    rs.lqp.rq.qstate.data = copy.deepcopy(tc.pvtdata.rq_pre_qstate)
    rs.lqp.rq.qstate.WriteWithDelay()
    rs.lqp.dcqcn_data = copy.deepcopy(tc.pvtdata.pre_dcqcn_data)
    rs.lqp.WriteDcqcnCb()

    return
