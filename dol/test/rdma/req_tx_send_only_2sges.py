#! /usr/bin/python3

from test.rdma.utils import *
import pdb

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    tc.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    tc.pvtdata.pre_qstate = rs.lqp.sq.qstate.data
    return

def TestCaseTrigger(tc):
    tc.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    tc.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    tc.pvtdata.post_qstate = rs.lqp.sq.qstate.data

    # verify that tx_psn is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.pre_qstate, tc.pvtdata.post_qstate, 'tx_psn', 1):
        return False

    # verify that p_index is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.pre_qstate, tc.pvtdata.post_qstate, 'p_index0', 1):
        return False

    # verify that c_index is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.pre_qstate, tc.pvtdata.post_qstate, 'c_index0', 1):
        return False

    # verify that ssn is incremented by 1
    if not VerifyFieldModify(tc, tc.pvtdata.pre_qstate, tc.pvtdata.post_qstate, 'ssn', 1):
        return False

    # verify that lsn is not incremented for send
    if not VerifyFieldModify(tc, tc.pvtdata.pre_qstate, tc.pvtdata.post_qstate, 'lsn', 0):
        return False

    # verify that busy is 0
    if not VerifyFieldAbsolute(tc, tc.pvtdata.post_qstate, 'busy', 0):
        return False

    # verify that in_progress is 0
    if not VerifyFieldAbsolute(tc, tc.pvtdata.post_qstate, 'in_progress', 0):
        return False

    return True

def TestCaseTeardown(tc):
    tc.info("RDMA TestCaseTeardown() Implementation.")
    return
