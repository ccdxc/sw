#! /usr/bin/python3

import test.callbacks.networking.modcbs as modcbs


def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def Teardown(infra, module):
    modcbs.Teardown(infra, module)
    return

def Verify(infra, module):
    return modcbs.Verify(infra, module)

def TestCaseSetup(tc):
    modcbs.TestCaseSetup(tc)
    
    iterelem = tc.module.iterator.Get() 
    tc.pvtdata.fte_session_aware = False
    tc.pvtdata.vlan_strip = getattr(iterelem, 'vlan_strip', False)
    tc.pvtdata.flood = getattr(iterelem, 'flood', False)
    
    if tc.pvtdata.flood == True:
        eniclist = tc.config.src.segment.floodlist.GetEnicOifList()
    else:
        eniclist = tc.config.flow.GetMulticastEnicOifList()

    tc.pvtdata.pruned_oiflist = []
    for oif in eniclist:
        if oif == tc.config.src.endpoint.intf: continue
        tc.pvtdata.pruned_oiflist.append(oif)
        if tc.pvtdata.vlan_strip:
            oif.lif.vlan_strip_en = True
            oif.lif.Update()
    return

def TestCaseTeardown(tc):
    modcbs.TestCaseTeardown(tc)
    for oif in tc.pvtdata.pruned_oiflist:
        if tc.pvtdata.vlan_strip:
            oif.lif.vlan_strip_en = False
            oif.lif.Update()
    return

def TestCaseVerify(tc):
    return modcbs.TestCaseVerify(tc)
