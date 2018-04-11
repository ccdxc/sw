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
    ep = tc.config.dst_endpoints[0]
    ep.Get()
    if len(ep.ipaddrs) and any(ip_in_dhcp_range(ip.get()) for ip in ep.ipaddrs):
        logger.error("Endpoint has been configured with DHCP IP address range %s %s" % (ep.ipaddrs[0].get(), ep.macaddr.get()))
        return False 
    return True

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)
