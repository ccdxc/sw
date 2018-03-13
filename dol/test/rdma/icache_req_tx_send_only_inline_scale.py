#! /usr/bin/python3

from test.rdma.utils import *
import pdb
import copy
from infra.common.glopts import GlobalOptions

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    rs = tc.config.rdmasession
    rs.lqp.sq.qstate.Read()
    tc.module.pvtdata.sq_pre_qstate = copy.deepcopy(rs.lqp.sq.qstate.data)
    tc.module.pvtdata.msn = (tc.module.pvtdata.sq_pre_qstate.msn + 1)

    # DO NOT ARM CQ and Set EQ's CI=PI for EQ enablement
    #rs.lqp.sq_cq.qstate.ArmCq()
    #rs.lqp.eq.qstate.reset_cindex(0)

    # Read CQ pre state
    rs.lqp.sq_cq.qstate.Read()
    tc.module.pvtdata.sq_cq_pre_qstate = rs.lqp.sq_cq.qstate.data

    return

def TestCaseTrigger(tc):
    tc.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    tc.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseStepVerify(tc, step):
    tc.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseTeardown(tc):
    tc.info("RDMA TestCaseTeardown() Implementation.")
    return
