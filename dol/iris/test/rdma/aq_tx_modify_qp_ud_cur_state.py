#! /usr/bin/python3

from iris.test.rdma.utils import *
import pdb
import copy
import random
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
import rdma_pb2				as rdma_pb2

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

    tc.pvtdata.test_qp = rs.lqp

    # Set attr_mask for mod_qp
    tc.pvtdata.attr_mask = 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_QKEY
    tc.pvtdata.attr_mask |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_CUR_STATE

    # Setup values to send down to mod_qp
    tc.pvtdata.cur_state = 4	# IBV_QPS_RTS + 1 (data path representation)
    tc.pvtdata.state = 4		# IBV_QPS_RTS + 1 (data path representation)
    tc.pvtdata.type_state = ((tc.pvtdata.cur_state - 1) << 4) | (tc.pvtdata.state - 1)
    tc.pvtdata.q_key = getattr(tc.pvtdata.rq_pre_qstate, 'rsq_base_addr/rqcb0_q_key') + 1

    # Change state to ERROR and check if Rx tables are updated
    rs.lqp.sq.qstate.data.state = 5 # QP_STATE_SQ_ERR
    rs.lqp.sq.qstate.data.sqcb1_state = 5 # QP_STATE_SQ_ERR
    rs.lqp.sq.qstate.WriteWithDelay()
    rs.lqp.rq.qstate.data.cb0_state = 5 # QP_STATE_SQ_ERR
    rs.lqp.rq.qstate.data.cb1_state = 5 # QP_STATE_SQ_ERR
    rs.lqp.rq.qstate.WriteWithDelay()

    # Assuming SQ, RQ share CQ
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.qp_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    tc.pvtdata.lif = rs.lqp.pd.ep.intf.lif
    tc.pvtdata.aq = tc.pvtdata.lif.aq
    PopulateAdminPreQStates(tc)

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

        # verify that q_key is not set in sqcb0,rqcb0
        if not VerifyFieldAbsolute(tc, rs.lqp.sq.qstate.data, 'sqcb0_header_template_addr/q_key', tc.pvtdata.q_key - 1):
            return False
        if not VerifyFieldAbsolute(tc, rs.lqp.rq.qstate.data, 'rsq_base_addr/rqcb0_q_key', tc.pvtdata.q_key - 1):
            return False

        # verify that state is not set in rqcb0, rqcb1, sqcb0, sqcb1
        if VerifyFieldAbsolute(tc, rs.lqp.rq.qstate.data, 'cb0_state', tc.pvtdata.state):
            return False
        if VerifyFieldAbsolute(tc, rs.lqp.rq.qstate.data, 'cb1_state', tc.pvtdata.state):
            return False
        if VerifyFieldAbsolute(tc, rs.lqp.sq.qstate.data, 'state', tc.pvtdata.state):
            return False
        if VerifyFieldAbsolute(tc, rs.lqp.sq.qstate.data, 'sqcb1_state', tc.pvtdata.state):
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

    return
