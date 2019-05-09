#! /usr/bin/python3
from infra.common.logging import logger as logger

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    iterelem = tc.module.iterator.Get()
    logger.info("NVME TestCaseSetup() Iterator @ ", iterelem)
    return

def TestCaseTrigger(tc):
    logger.info("NVME TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    logger.info("NVME TestCaseVerify() Implementation.")
    return True

def TestCaseTeardown(tc):
    logger.info("NVME TestCaseTeardown() Implementation.")
    return

