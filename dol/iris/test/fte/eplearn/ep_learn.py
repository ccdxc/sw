#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
import iris.test.callbacks.networking.modcbs as modcbs

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

def IsIpv4(testcase):
    return "ARP" in  testcase.module.iterator.Get().type

def IsIpv6(testcase):
    return "NDP" in  testcase.module.iterator.Get().type

def TestCaseStepVerify(tc, step):
    return True

def TestCaseStepTeardown(tc, step):
    return modcbs.TestCaseStepTeardown(tc, step)
