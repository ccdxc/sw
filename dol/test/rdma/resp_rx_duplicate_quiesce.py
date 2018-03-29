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

    # Read RQ pre state
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data

    tc.pvtdata.slab_1 = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr_slab_1 = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab_1.GID())

    tc.pvtdata.slab_2 = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr_slab_2 = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab_2.GID())

    # Read CQ pre state
    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_pre_qstate = rs.lqp.rq_cq.qstate.data

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
    logger.info("step id: %d" %(step.step_id))
    rs = tc.config.rdmasession
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    ring1_mask = (rs.lqp.num_rsq_wqes - 1)

    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

    rs.lqp.rq_cq.qstate.Read()
    tc.pvtdata.rq_cq_post_qstate = rs.lqp.rq_cq.qstate.data

    if step.step_id == 0:
        logger.info("after receiving AT1/AT2/AT3/SEND..")

        # verify that e_psn is incremented by 4
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 4):
            return False

        # verify that proxy_cindex is incremented by 1
        # for send
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 1):
            return False

        # verify that token_id is incremented by 4
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 4):
            return False

        # verify that nxt_to_go_token_id is incremented by 4
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 4):
            return False

        # verify that busy is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'busy', 0):
            return False

        # verify that rsq p_index and c_index is incremented by 3
        # for AT1/AT2/AT3
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index1', ring1_mask, 3):
            return False

        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'c_index1', ring1_mask, 3):
            return False

        # verify that rsq p_index prime is incremented by 0
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rsq_pindex_prime', ring1_mask, 0):
            return False

        # verify that adjust_rsq_c_index_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'adjust_rsq_c_index_in_progress', 0):
            return False

        # verify that rsq_quiesce is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'rsq_quiesce', 0):
            return False
    
        # verify that read_rsp_lock is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_lock', 0):
            return False
    
        # verify that read_rsp_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_in_progress', 0):
            return False
    
    elif step.step_id == 1:
        logger.info("after receiving duplicate AT1..")

        # verify that e_psn is incremented by 0
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 0):
            return False

        # verify that proxy_cindex is incremented by 0
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
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

        # verify that rsq p_index and c_index is backtracked by 2
        # (compared to previous step)
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index1', ring1_mask, -2):
            return False

        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'c_index1', ring1_mask, -2):
            return False

        # verify that rsq p_index prime is set to 
        # same value as previous rsq p_index
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'rsq_pindex_prime', tc.pvtdata.rq_pre_qstate.p_index1):
            return False

        # verify that adjust_rsq_c_index_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'adjust_rsq_c_index_in_progress', 0):
            return False

        # verify that rsq_quiesce is 1
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'rsq_quiesce', 1):
            return False
    
        # verify that read_rsp_lock is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_lock', 0):
            return False
    
        # verify that read_rsp_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_in_progress', 0):
            return False

    elif step.step_id == 2:
        logger.info("after receiving duplicate AT2..")

        # verify that e_psn is NOT incremented
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 0):
            return False

        # verify that proxy_cindex is incremented by 0
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
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

        # verify that rsq p_index and c_index is forwarded by 1
        # (compared to previous step)
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index1', ring1_mask, 1):
            return False

        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'c_index1', ring1_mask, 1):
            return False

        # verify that rsq p_index prime stays same
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rsq_pindex_prime', ring1_mask, 0):
            return False

        # verify that adjust_rsq_c_index_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'adjust_rsq_c_index_in_progress', 0):
            return False

        # verify that rsq_quiesce is still 1
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'rsq_quiesce', 1):
            return False
    
        # verify that read_rsp_lock is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_lock', 0):
            return False
    
        # verify that read_rsp_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_in_progress', 0):
            return False

    elif step.step_id == 3:
        logger.info("after receiving AT4..")

        # verify that e_psn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 1):
            return False

        # verify that proxy_cindex is incremented by 0
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
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

        # verify that rsq p_index and c_index stays same
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index1', ring1_mask, 0):
            return False

        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'c_index1', ring1_mask, 0):
            return False

        # verify that rsq p_index prime is incremented by 1
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rsq_pindex_prime', ring1_mask, 1):
            return False

        # verify that adjust_rsq_c_index_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'adjust_rsq_c_index_in_progress', 0):
            return False

        # verify that rsq_quiesce is still 1
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'rsq_quiesce', 1):
            return False
    
        # verify that read_rsp_lock is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_lock', 0):
            return False
    
        # verify that read_rsp_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_in_progress', 0):
            return False

    elif step.step_id == 4:
        logger.info("after receiving duplicate AT3..")

        # verify that e_psn is NOT incremented
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 0):
            return False

        # verify that proxy_cindex is incremented by 0
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
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

        # verify that rsq p_index and c_index is forwarded by 1
        # (compared to previous step)
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index1', ring1_mask, 1):
            return False

        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'c_index1', ring1_mask, 1):
            return False

        # verify that rsq p_index prime stays same
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rsq_pindex_prime', ring1_mask, 0):
            return False

        # verify that adjust_rsq_c_index_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'adjust_rsq_c_index_in_progress', 0):
            return False

        # verify that rsq_quiesce is still 1
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'rsq_quiesce', 1):
            return False
    
        # verify that read_rsp_lock is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_lock', 0):
            return False
    
        # verify that read_rsp_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_in_progress', 0):
            return False

    elif step.step_id == 5:
        logger.info("after receiving duplicate AT4..")

        # verify that e_psn is NOT incremented
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 0):
            return False

        # verify that proxy_cindex is incremented by 0
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask, 0):
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

        # verify that rsq p_index and c_index is forwarded by 1
        # (compared to previous step)
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'p_index1', ring1_mask, 1):
            return False

        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'c_index1', ring1_mask, 1):
            return False

        # verify that rsq p_index prime stays same
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'rsq_pindex_prime', ring1_mask, 0):
            return False

        # verify that rsq p_index has now reached to prime
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'p_index1', tc.pvtdata.rq_post_qstate.rsq_pindex_prime):
            return False

        # verify that adjust_rsq_c_index_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'adjust_rsq_c_index_in_progress', 0):
            return False

        # verify that rsq_quiesce is now reset to 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'rsq_quiesce', 0):
            return False
    
        # verify that read_rsp_lock is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_lock', 0):
            return False
    
        # verify that read_rsp_in_progress is 0
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'read_rsp_in_progress', 0):
            return False


    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.rq_pre_qstate = copy.deepcopy(rs.lqp.rq.qstate.data)
    tc.pvtdata.rq_cq_pre_qstate = copy.deepcopy(rs.lqp.rq_cq.qstate.data)
    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    return
