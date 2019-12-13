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
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data
    tc.pvtdata.slab_1 = rs.lqp.pd.ep.GetNewSlab()  
    tc.pvtdata.mr_slab_1 = rs.lqp.pd.mrs.Get('MR-' + tc.pvtdata.slab_1.GID())                           

    return

def TestCasePreTrigger(tc):
    if (GlobalOptions.dryrun): return
    rs = tc.config.rdmasession                                                                                                                       
    rs.lqp.rq.qstate.Read()
    # purposely disable QP access flag for ATOMIC
    tc.pvtdata.access_flags = rs.lqp.rq.qstate.data.access_flags
    rs.lqp.rq.qstate.data.access_flags &= (0 << 2| 1 << 1 | 1)
    rs.lqp.rq.qstate.WriteWithDelay()
    logger.info("In TestCasePreTrigger, updated access flags {0}".format(rs.lqp.rq.qstate.data.access_flags))
    return

def TestCaseTrigger(tc):
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    if (GlobalOptions.dryrun): return True
    logger.info("RDMA TestCaseVerify() Implementation.")
    PopulatePostQStates(tc)

    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    ring0_mask = (rs.lqp.num_rq_wqes - 1)
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

    # verify that e_psn is NOT incremented
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'e_psn', 0):
        return False

    # verify that proxy_cindex is not incremented
    if not VerifyFieldMaskModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'proxy_cindex', ring0_mask,  0):
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

    #verify that access_err is set to 1
    if not VerifyFieldModify(tc, tc.pvtdata.rq_pre_qstate, tc.pvtdata.rq_post_qstate, 'qp_err_dis_access_err', 1):
        return False

    # last bth opcode should be 20 (atomic_fetchadd)
    if not VerifyFieldAbsolute(tc, tc.pvtdata.rq_post_qstate, 'last_bth_opcode', 20):
        return False

    return True

def TestCaseTeardown(tc):
    if (GlobalOptions.dryrun): return
    logger.info("RDMA TestCaseTeardown() Implementation.")
    ResetErrQState(tc)
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    # reset QP access flag to original value
    rs.lqp.rq.qstate.data.access_flags = tc.pvtdata.access_flags
    rs.lqp.rq.qstate.WriteWithDelay()
    return
