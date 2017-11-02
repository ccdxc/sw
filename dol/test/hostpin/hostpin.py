#! /usr/bin/python3

import test.callbacks.networking.modcbs as modcbs

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def TestCaseSetup(tc):
    modcbs.TestCaseSetup(tc)
    oiflist = tc.config.src.segment.floodlist.enic_list
    tc.pvtdata.pruned_oiflist = []
    for oif in oiflist:
        if oif == tc.config.src.endpoint.intf: continue
        tc.pvtdata.pruned_oiflist.append(oif)
    return

def TestCaseTeardown(tc):
    modcbs.TestCaseTeardown(tc)
    return

