#! /usr/bin/python3
# Test Module

import os
import pdb
import infra.api.api as InfraApi
import iris.test.callbacks.networking.modcbs as modcbs
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

def ip_in_dhcp_range(ip):
    return "169.0" in ip
    
def TestCaseStepVerify(tc, step):
    logger.info("SRC EP DST EP %s(%s) %s(%s)" %  (tc.config.src_endpoints[0],tc.config.src_endpoints[0].macaddr.get(),
                                                   tc.config.dst_endpoints[0], tc.config.dst_endpoints[0].macaddr.get()))
    
    src_ipaddrs = tc.config.src_endpoints[0].ipaddrs
    dst_ipaddrs = tc.config.dst_endpoints[0].ipaddrs
    
    dst_ep = tc.config.dst_endpoints[0]
    src_ep = tc.config.src_endpoints[0]
    src_ep.Get()
    dst_ep.Get()
    
    if (step.step_id in [2]):
        ret = true
        if (not src_ep.IsRemote()):
            if len(src_ep.ipaddrs) == 0:
                logger.info("SRC EP IP not learnt")
                return false
        if (not dst_ep.IsRemote()):
            if len(dst_ep.ipaddrs) == 0:
                logger.info("DST EP IP not learnt")
                return false
        #Restore values for futures testcases.
        tc.config.src_endpoints[0].ipaddrs = src_ipaddrs
        tc.config.dst_endpoints[0].ipaddrs = dst_ipaddrs
        
    return True

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)
