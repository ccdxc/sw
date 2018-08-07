#! /usr/bin/python3
from infra.common.logging import logger as logger

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    iterelem = tc.module.iterator.Get()
    logger.info("RDMA TestCaseSetup() Iterator @ ", iterelem)
    return

def TestCaseTrigger(tc):
    #tc.config.src.lif
    #tc.config.dst.lif
    logger.info("RDMA TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    logger.info("RDMA TestCaseVerify() Implementation.")
    return True

def TestCaseTeardown(tc):
    logger.info("RDMA TestCaseTeardown() Implementation.")
    return

