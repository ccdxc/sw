#! /usr/bin/python3

from iris.test.nvme.utils import *
import pdb
import copy
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    logger.info("NVME TestCaseSetup() Implementation.")
    tc.pvtdata.nlb = 10
    PopulatePreQStates(tc)
    return

def TestCaseTrigger(tc):
    logger.info("NVME TestCaseTrigger() Implementation.")
    return

def TestCaseVerify(tc):
    logger.info("NVME TestCaseVerify() Implementation.")
    PopulatePostQStates(tc)
    if not ValidateWriteTxChecks(tc):
        return False
    return True

def TestCaseStepVerify(tc, step):
    if (GlobalOptions.dryrun): return True
    logger.info("NVME TestCaseVerify() Implementation.")
    return True

def TestCaseTeardown(tc):
    logger.info("NVME TestCaseTeardown() Implementation.")
    return
