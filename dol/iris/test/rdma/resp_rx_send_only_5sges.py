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
    PopulatePreQStates(tc)

    rs = tc.config.rdmasession

    # Slab 1
    tc.pvtdata.slab_1 = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr_slab_1 = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab_1.GID())
    tc.pvtdata.l_key1 = tc.pvtdata.mr_slab_1.lkey

    # Slab 2
    tc.pvtdata.slab_2 = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr_slab_2 = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab_2.GID())
    tc.pvtdata.l_key2 = tc.pvtdata.mr_slab_2.lkey

    # Slab 3
    tc.pvtdata.slab_3 = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr_slab_3 = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab_3.GID())
    tc.pvtdata.l_key3 = tc.pvtdata.mr_slab_3.lkey

    # Slab 4
    tc.pvtdata.slab_4 = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr_slab_4 = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab_4.GID())
    tc.pvtdata.l_key4 = tc.pvtdata.mr_slab_4.lkey

    # Slab 5
    tc.pvtdata.slab_5 = rs.lqp.pd.ep.GetNewSlab()
    tc.pvtdata.mr_slab_5 = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab_5.GID())
    tc.pvtdata.l_key5 = tc.pvtdata.mr_slab_5.lkey

    if (GlobalOptions.dryrun):
        tc.pvtdata.mr_va1 = 0
        tc.pvtdata.mr_va2 = 0
        tc.pvtdata.mr_va3 = 0
        tc.pvtdata.mr_va4 = 0
        tc.pvtdata.mr_va5 = 0
        return True
    else:
        tc.pvtdata.mr_va1 = tc.pvtdata.slab_1.address + 1088
        tc.pvtdata.mr_va2 = tc.pvtdata.slab_2.address + 1088
        tc.pvtdata.mr_va3 = tc.pvtdata.slab_3.address + 1088
        tc.pvtdata.mr_va4 = tc.pvtdata.slab_4.address + 1088
        tc.pvtdata.mr_va5 = tc.pvtdata.slab_5.address + 1088

    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    PopulatePostQStates(tc)

    rs = tc.config.rdmasession
    ring0_mask = (rs.lqp.num_rq_wqes - 1)

    if step.step_id == 0:
    
        ############     RQ VALIDATIONS #################
        # verify that e_psn is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 1):
            return False

        # verify that msn is NOT incremented
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'msn', 0):
            return False
    
        # verify that proxy_cindex is incremented
        if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask,  1):
            return False
    
       # verify that token_id is incremented by 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'token_id', 1):
            return False
    
        # verify that nxt_to_go_token_id is NOT incremented
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'nxt_to_go_token_id', 0):
            return False

        # verify that state is now moved to ERR (2)
        if not VerifyErrQState(tc):
            return False

        ############     STATS VALIDATIONS #################
        # verify that error disable stats are updated
        if not VerifyErrStatistics(tc):
            return False

        #verify that max_sge_err is set to 1
        if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'qp_err_dis_max_sge_err', 1):
            return False

        # last bth opcode should be 4 (send_only)
        if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'last_bth_opcode', 4):
            return False

        ############     CQ VALIDATIONS #################
        if not ValidateCQCompletions(tc, 1, 1):
            return False

    elif step.step_id == 1:

        if not ValidatePostSyncCQChecks(tc):
            return False 

    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    ResetErrQState(tc)

    return
