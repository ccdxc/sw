#! /usr/bin/python3

import copy
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
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    tc.pvtdata.slab_1 = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr_slab_1 = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab_1.GID())

    logger.info("RDMA atomic resource data read/write")
    rs.lqp.ReadAtomicResData()
    tc.pvtdata.atomic_pre_qstate = rs.lqp.atomic_res_data
    # Simulate the condition where all atomic resources are in use
    rs.lqp.atomic_res_data.data0 = 0x0101010101010101
    rs.lqp.atomic_res_data.data1 = 0x0101010101010101
    rs.lqp.atomic_res_data.data2 = 0x0101010101010101
    rs.lqp.atomic_res_data.data3 = 0x0101010101010101

    rs.lqp.WriteAtomicResData()

    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseStepTrigger(tc, step):
    logger.info("RDMA TestCaseStepTrigger() Implementation with step_id: %d" % (step.step_id))
    if (GlobalOptions.dryrun): return True
    return

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    logger.info("step id: %d" %(step.step_id))

    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

    rs.lqp.ReadAtomicResData()
    tc.pvtdata.atomic_post_qstate = rs.lqp.atomic_res_data

    if step.step_id == 0:
        ############     RQ VALIDATIONS #################
        # verify that e_psn is NOT incremented 
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 0):
            return False

        # verify that msn is NOT incremented 
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'msn', 0):
            return False

        # verify that proxy_cindex is NOT incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask,  0):
            return False

        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        # verify that disable_speculation is set to 0
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'disable_speculation', 0):
            return False

        # verify that rsq_pindex is NOT incremented
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rsq_pindex', 0):
            return False

        rs.lqp.ReadAtomicResData()
        tc.pvtdata.atomic_pre_qstate = rs.lqp.atomic_res_data
        # Free up one atomic resource
        rs.lqp.atomic_res_data.data3 = 0x01010101010101
        rs.lqp.WriteAtomicResData()

    elif step.step_id == 1:
        ############     RQ VALIDATIONS #################
        # verify that e_psn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 1):
            return False

        # verify that msn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'msn', 1):
            return False

        # verify that proxy_cindex is NOT incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask,  0):
            return False

        # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False

        # verify that nxt_to_go_token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 1):
            return False

        # verify that disable_speculation is set to 0
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'disable_speculation', 0):
            return False

        # verify that rsq_pindex is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rsq_pindex', 1):
            return False

    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.rq_pre_qstate = copy.deepcopy(rs.lqp.rq.qstate.data)
    tc.pvtdata.rq_cq_pre_qstate = copy.deepcopy(rs.lqp.rq_cq.qstate.data)
    tc.pvtdata.atomic_pre_qstate = copy.deepcopy(rs.lqp.atomic_res_data)

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    #logger.info("RDMA atomic resource data read/write")

    rs = tc.config.rdmasession
    rs.lqp.ReadAtomicResData()

    tc.pvtdata.atomic_pre_qstate = rs.lqp.atomic_res_data
    # Free up all the atomic resources
    rs.lqp.atomic_res_data.data0 = 0x0
    rs.lqp.atomic_res_data.data1 = 0x0
    rs.lqp.atomic_res_data.data2 = 0x0
    rs.lqp.atomic_res_data.data3 = 0x0

    rs.lqp.WriteAtomicResData()

    return
