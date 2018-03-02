#! /usr/bin/python3

from test.rdma.utils import *
import pdb
import copy
from infra.common.glopts import GlobalOptions

def Setup(infra, module):
    iterelem = module.iterator.Get()

    module.logger.info("Iterator Selectors")

    if iterelem:
        if 'base' in iterelem.rdmasession.__dict__:
            module.logger.info("- rdmasession.base: %s" % iterelem.rdmasession.base)
            module.testspec.selectors.rdmasession.base.Extend(iterelem.rdmasession.base)
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    tc.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.pvtdata.msn = (tc.pvtdata.sq_pre_qstate.msn + 1)

    rs.lqp.sq.qstate.Read()
    rs.lqp.sq.qstate.data.poll_for_work = 1
    rs.lqp.sq.qstate.Write()

    # DO NOT ARM CQ and Set EQ's CI=PI for EQ enablement
    #rs.lqp.sq_cq.qstate.ArmCq()
    #rs.lqp.eq.qstate.reset_cindex(0)

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    return

def TestCaseTrigger(tc):
    tc.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    tc.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    tc.info("RDMA TestCaseVerify() Implementation. Step ID: ", step.step_id)
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    ring0_mask = (rs.lqp.num_sq_wqes - 1)
    tc.pvtdata.sq_post_qstate = rs.lqp.sq.qstate.data

    # verify that tx_psn is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.sq_pre_qstate, tc.pvtdata.sq_post_qstate, 'tx_psn', 1):
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

    # verify that p_index of rrq is not incremented
    if not VerifyFieldsEqual(tc, tc.pvtdata.sq_pre_qstate, 'p_index5', tc.pvtdata.sq_post_qstate, 'p_index5'):
        return False

    # verify that poll_in_progress is 0
    if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'poll_in_progress', 0):
        return False

    # verify that poll_success is 0
    if not VerifyFieldAbsolute(tc, tc.pvtdata.sq_post_qstate, 'poll_success', 0):
        return False

    # update current as pre_qstate ... so next step_id can use it as pre_qstate
    tc.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)

    return True

def TestCaseTeardown(tc):
    tc.info("RDMA TestCaseTeardown() Implementation.")

    rs = tc.config.rdmasession
    tc.info("Clearing poll_for_work in SQCB\n")
    rs.lqp.sq.qstate.data.poll_for_work = 0
    rs.lqp.sq.qstate.Write()

    return
