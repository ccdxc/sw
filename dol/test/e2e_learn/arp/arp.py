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
    
    src_ipaddrs = tc.config.src_endpoints[0].ipaddrs
    dst_ipaddrs = tc.config.dst_endpoints[0].ipaddrs
    
    dst_ep = tc.config.dst_endpoints[0]
    src_ep = tc.config.src_endpoints[0]
    src_ep.Get()
    dst_ep.Get()
    
    if (step.step_id in [2]):
        if ( (not src_ep.IsRemote() and len(src_ep.ipaddrs)) or \
              (not dst_ep.IsRemote() and len(dst_ep.ipaddrs))):
            logger.error("Endpoint has been configured with IP address still")
            return False
        #Restore values for futures testcases.
        tc.config.src_endpoints[0].ipaddrs = src_ipaddrs
        tc.config.dst_endpoints[0].ipaddrs = dst_ipaddrs
    else:     
        if not src_ep.ipaddrs[0].get() == src_ipaddrs[0].get() or \
             not dst_ep.ipaddrs[0].get() == dst_ipaddrs[0].get():    
            logger.error("Endpoint has not been configured with IP address")
            return False
        
    return True

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)
