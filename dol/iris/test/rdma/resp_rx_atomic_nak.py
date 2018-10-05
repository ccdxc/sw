#! /usr/bin/python3

import copy
from iris.test.rdma.utils import *
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger
def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("RDMA TestCaseSetup() Implementation.")
    PopulatePreQStates(tc)

    rs = tc.config.rdmasession

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
    PopulatePostQStates(tc)
    logger.info("step id: %d" %(step.step_id))

    rs = tc.config.rdmasession
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    ring1_mask = (rs.lqp.num_rsq_wqes - 1)

    rs.lqp.ReadAtomicResData()
    tc.pvtdata.atomic_post_qstate = rs.lqp.atomic_res_data

    if step.step_id == 0:
        ############     RQ VALIDATIONS #################
        # verify that e_psn is incremented  by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 1):
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
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 0):
            return False

        # verify that busy is set to 0
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'busy', 0):
            return False

        # verify that rsq_pindex is incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rsq_pindex', ring1_mask, 1):
            return False

        # verify that state is now moved to ERR (2)
        if not VerifyErrQState(tc):
            return False

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

    ResetErrQState(tc)
    return
