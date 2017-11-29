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
    oiflist = tc.config.root.segment.floodlist.enic_list
    tc.pvtdata.pruned_oiflist = []
    for oif in oiflist:
        if oif == tc.config.root.intf: continue
        tc.pvtdata.pruned_oiflist.append(oif)
    return

def TestCaseTeardown(tc):
    #modcbs.TestCaseTeardown(tc)
    return

def TestCaseStepSetup(tc, step):
    return modcbs.TestCaseStepSetup(tc, step)

def TestCaseStepTrigger(tc, step):
    return modcbs.TestCaseStepTrigger(tc, step)

def TestCaseStepVerify(tc, step):
    ipaddrs = tc.config.root.ipaddrs
    tc.config.root.Get()
    assert (tc.config.root.ipaddrs[0].get() == ipaddrs[0].get())
    tc.config.root.ipaddrs = ipaddrs
    return True

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)
