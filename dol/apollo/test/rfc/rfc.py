#! /usr/bin/python3
# RFC Module
import pdb
import apollo.test.callbacks.common.modcbs as modcbs
import apollo.test.utils.vpp as vpp

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return True

def TestCaseSetup(tc):
    tc.AddIgnorePacketField('UDP', 'sport')
    tc.AddIgnorePacketField('UDP', 'chksum')
    tc.AddIgnorePacketField('TCP', 'chksum')
    tc.AddIgnorePacketField('IP', 'chksum') #Needed to pass NAT testcase

    iterelem = tc.module.iterator.Get()
    if iterelem:
        tc.pvtdata.flow_verify_vpp = getattr(iterelem, "flow_verify_vpp", False)
    return True

def TestCaseTeardown(tc):
    return True

def TestCasePreTrigger(tc):
    return True

def TestCaseStepSetup(tc, step):
    return True

def TestCaseStepTrigger(tc, step):
    return True

def TestCaseStepVerify(tc, step):
    return True

def TestCaseStepTeardown(tc, step):
    return True

def TestCaseVerify(tc):
    if hasattr(tc.pvtdata, 'flow_verify_vpp') and tc.pvtdata.flow_verify_vpp:
        pkt = tc.packets.db["IN_PKT"].GetScapyPacket();
        vnic=tc.config.localmapping.VNIC.VnicId-1
        return vpp.CheckFlowExists(pkt, lkp_id=vnic)
    return True
