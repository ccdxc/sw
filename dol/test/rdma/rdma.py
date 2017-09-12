#! /usr/bin/python3

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    iterelem = tc.module.iterator.Get()
    tc.info("RDMA TestCaseSetup() Iterator @ ", iterelem)
    return

def TestCaseTrigger(tc):
    #tc.config.src.lif
    #tc.config.dst.lif
    tc.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    tc.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseTeardown(tc):
    tc.info("RDMA TestCaseTeardown() Implementation.")
    return

