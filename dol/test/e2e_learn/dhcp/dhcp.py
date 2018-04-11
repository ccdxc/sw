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

def ip_in_dhcp_range(ip):
    return "169.0" in ip
    
def TestCaseStepVerify(tc, step):
    logger.info("SRC EP DST EP %s(%s) %s(%s)" %  (tc.config.src_endpoints[0],tc.config.src_endpoints[0].macaddr.get(),
                                                   tc.config.dst_endpoints[0], tc.config.dst_endpoints[0].macaddr.get()))
    if (step.step_id in [0,3]):
        ep = tc.config.dst_endpoints[0]
        ep.Get()
        logger.info("Verify for EP MAc address %s" %  ep.macaddr.get())
        if len(ep.ipaddrs) and any(ip_in_dhcp_range(ip.get()) for ip in ep.ipaddrs):
            logger.error("Endpoint has been configured with DHCP IP address range %s %s" % (ep.ipaddrs[0].get(), ep.macaddr.get()))
            return False 
    if (step.step_id in [1,2]):
        ep = tc.config.dst_endpoints[0]
        ep.Get()
        if not len(ep.ipaddrs) and all(not ip_in_dhcp_range(ip.get()) for ip in ep.ipaddrs):
            logger.error("Endpoint has not been configured with IP address")
            return False
        logger.info("Endpoint has been configured with IP address : %s" % (ep.ipaddrs[0].get()))
        
    return True

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)
