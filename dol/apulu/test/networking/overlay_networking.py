#! /usr/bin/python3
# Overlay Networking Module
import pdb

import infra.common.defs as defs
import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.generator as generator
import apollo.test.callbacks.common.modcbs as modcbs
from apollo.config.store import EzAccessStore

def Setup(infra, module):
    if 'WORKFLOW_START' in module.name:
        topo.ChosenFlowObjs.select_objs = True
        topo.ChosenFlowObjs.use_selected_objs = False
        topo.ChosenFlowObjs.SetMaxLimits(module.testspec.selectors.maxlimits)
    modcbs.Setup(infra, module)
    return True

def TestCaseSetup(tc):
#    tc.AddIgnorePacketField('Ether', 'dst')
#    tc.AddIgnorePacketField('Ether', 'src')
    tc.AddIgnorePacketField('UDP', 'sport')
    tc.AddIgnorePacketField('UDP', 'chksum')
    tc.AddIgnorePacketField('IP', 'chksum') #Needed to pass NAT testcase
    # TODO: Ignore tos until all testspecs are updated to take tos from VPC
    tc.AddIgnorePacketField('IP', 'tos')
    if tc.config.root.FwdMode == 'IGW_NAPT':
        tc.AddIgnorePacketField('IP', 'src')
        tc.AddIgnorePacketField('UDP', 'sport')
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
    #if 'TO_SWITCH_PKT' in tc.packets.db.keys():
    #    print ("Testcase Step Verify - expected pkt")
    #    print (tc.packets.db['TO_SWITCH_PKT'])
    #    inner_eth_src = tc.packets.db['TO_SWITCH_PKT'].headers.eth.fields.src
    #    inner_eth_dst = tc.packets.db['TO_SWITCH_PKT'].headers.eth.fields.dst
    #    inner_ip_src = tc.packets.db['TO_SWITCH_PKT'].headers.ipv4.fields.src
    #    inner_ip_dst = tc.packets.db['TO_SWITCH_PKT'].headers.ipv4.fields.dst
    #    outer_eth_src = tc.packets.db['TO_SWITCH_PKT'].headers.outereth.fields.src
    #    outer_eth_dst = tc.packets.db['TO_SWITCH_PKT'].headers.outereth.fields.dst
    #    outer_ip_src = tc.packets.db['TO_SWITCH_PKT'].headers.outeripv4.fields.src
    #    outer_ip_dst = tc.packets.db['TO_SWITCH_PKT'].headers.outeripv4.fields.dst
    #    vxlan = tc.packets.db['TO_SWITCH_PKT'].headers.vxlan.fields.vni
    #else:
    #    print ("To SwitchPkt not found")
    #if 'RXPKT1' in tc.pcr.rxpkts.keys():
    #    print ("Testcase Step Verify - received pkt")
    #    rxpkthdr = tc.pcr.rxpkts['RXPKT1'].hdrs[0]
# [<Ether  dst=02:42:0b:01:01:02 src=02:42:0b:01:01:01 type=0x800 |>, <IP  version=4 ihl=5 tos=0x0 len=154 id=0 flags= frag=0 ttl=64 proto=udp chksum=0x4a24 src=100.100.1.1 dst=200.200.2.2 options=[] |>, <UDP  sport=0 dport=4789 len=134 chksum=0x0 |>, <VXLAN  flags_r0=0 flag_i=1 flags_r1=0 rsvd0=0 vni=80001 rsvd1=0 |>, <Ether  dst=00:ee:00:00:00:02 src=00:dd:00:00:00:02 type=0x800 |>, <IP  version=4 ihl=5 tos=0x7 len=104 id=1 flags= frag=0 ttl=64 proto=tcp chksum=0x6281 src=12.0.0.2 dst=12.0.0.6 options=[] |>, <TCP  sport=100 dport=200 seq=4660 ack=22136 dataofs=5 reserved=0 flags=A window=8192 chksum=0xdb5 urgptr=0 options=[] |>, <PAYLOAD  data=[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] |>]        
    #    outereth = rxpkthdr[0]
    #    return defs.status.OVERRIDE
    #else:
    #    print ("Received Pkt not found")
    #    return defs.status.ERROR
    if 'TO_SWITCH_PKT' in tc.packets.db.keys():
        if 'RXPKT1' in tc.rxpkts.keys():
            print ("Packet Verify - received pkt")
            return defs.status.OVERRIDE
        print ("Packet Verify - did not receive pkt")
        return defs.status.ERROR
    return defs.status.SUCCESS

def TestCaseStepTeardown(tc, step):
    return True

def TestCaseVerify(tc):
    if 'WORKFLOW_START' in tc.module.name:
        topo.ChosenFlowObjs.select_objs = False
        topo.ChosenFlowObjs.use_selected_objs = True
    elif 'WORKFLOW_END' in tc.module.name:
        topo.ChosenFlowObjs.Reset()
        generator.__read(EzAccessStore.GetDUTNode())
    return True
