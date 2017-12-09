#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
import test.callbacks.networking.modcbs as modcbs

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def TestCaseSetup(tc):
    modcbs.TestCaseSetup(tc)
    tc.pvtdata.dhcp_server =  (tc.config.root.segment.GetRemoteEps())[0]
    return

def TestCaseTeardown(tc):
    #modcbs.TestCaseTeardown(tc)
    return

def TestCaseStepSetup(tc, step):
    return modcbs.TestCaseStepSetup(tc, step)

def TestCaseStepTrigger(tc, step):
    return modcbs.TestCaseStepTrigger(tc, step)

def IsIpv4(testcase):
    return True

def IsIpv6(testcase):
    return False

def TestCaseStepVerify(tc, step):
    if "ACK" not in tc.module.iterator.Get().type:
        return True
    ipaddrs = tc.config.root.ipaddrs
    ipv6addrs = tc.config.root.ipv6addrs
    tc.config.root.Get()
    if IsIpv4(tc):
        assert (tc.config.root.ipaddrs[0].get() == ipaddrs[0].get())
    elif IsIpv6(tc):
        assert (tc.config.root.ipv6addrs[0].get() == ipv6addrs[0].get())
    else:
        assert(0)
    tc.config.root.ipaddrs = ipaddrs
    tc.config.root.ipv6addrs = ipv6addrs
    return True

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)
