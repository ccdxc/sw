#! /usr/bin/python3

from test.rdma.utils import *

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    tc.info("RDMA TestCaseSetup() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_pre_qstate = rs.lqp.rq.qstate.data
    return

def TestCaseTrigger(tc):
    tc.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    tc.info("RDMA TestCaseVerify() Implementation.")
    rs = tc.config.rdmasession
    rs.lqp.rq.qstate.Read()
    tc.pvtdata.rq_post_qstate = rs.lqp.rq.qstate.data

    # verify that e_psn is incremented by 1
    #if not VerifyFieldModify(tc, tc.pvtdata.pre_qstate, tc.pvtdata.post_qstate, 'e_psn', 1):
    #    return False

    # verify that c_index is incremented by 1
    #if not VerifyFieldModify(tc, tc.pvtdata.pre_qstate, tc.pvtdata.post_qstate, 'c_index0', 1):
    #    return False

    # verify that token_id is incremented by 1
    #if not VerifyFieldModify(tc, tc.pvtdata.pre_qstate, tc.pvtdata.post_qstate, 'token_id', 1):
    #    return False

    # verify that nxt_to_go_token_id is incremented by 1
    #if not VerifyFieldModify(tc, tc.pvtdata.pre_qstate, tc.pvtdata.post_qstate, 'nxt_to_go_token_id', 1):
    #    return False

    # verify that busy is 0
    #if not VerifyFieldAbsolute(tc, tc.pvtdata.post_qstate, 'busy', 0):
    #    return False

    return True

def TestCaseTeardown(tc):
    tc.info("RDMA TestCaseTeardown() Implementation.")
    return
