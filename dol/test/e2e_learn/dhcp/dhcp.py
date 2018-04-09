#! /usr/bin/python3
# Test Module

import os
import pdb
import infra.api.api as InfraApi
import test.callbacks.networking.modcbs as modcbs
from infra.common.logging   import logger as logger

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def TestCaseSetup(tc):
    pass

def TestCaseTeardown(tc):
    pass

def TestCaseStepSetup(tc, step):
    return modcbs.TestCaseStepSetup(tc, step)

def TestCaseStepTrigger(tc, step):
    return modcbs.TestCaseStepTrigger(tc, step)

def TestCaseStepVerify(tc, step):
    if (step.step_id in [0,3]):
        ep = tc.config.dst_endpoints[0]
        ep.Get()   
        if len(ep.ipaddrs):
            logger.error("Endpoint has been configured with IP address %s" % (ep.ipaddrs[0].get()))
            return False 
    if (step.step_id in [1,2]):
        ep = tc.config.dst_endpoints[0]
        ep.Get()
        if not len(ep.ipaddrs):
            logger.error("Endpoint has not been configured with IP address")
            return False
        logger.info("Endpoint has been configured with IP address : %s" % (ep.ipaddrs[0].get()))
        
    return True

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)
