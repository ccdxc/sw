#! /usr/bin/python3
from infra.api.objects import PacketHeader
import infra.api.api as infra_api

def GetIpv4Ttl(a, b):
    return 129

def GetPacketTemplateByFlow():
    return

def AddHeaderByFlow_COMMON():
    return

def AddHeaderByFlow_QTAG():
    # vlan: ref://testcase/config/src/endpoint/encap_vlan
    return

def AddHeaderETHERNET():
    # src: ref://testcase/config/src/endpoint/macaddr
    # dst: ref://testcase/config/dst/endpoint/macaddr
    return

def AddHeaderIPV4():
    # src  : ref://testcase/config/flow/sip
    # dst  : ref://testcase/config/flow/dip
    # proto: ref://testcase/config/flow/proto
    return

def AddHeaderTCP():
    # sport   : ref://testcase/config/flow/sport
    # dport   : ref://testcase/config/flow/dport
    return

def AddIngressHeadersByFlow(tc, pkt):
    return

def AddEgressHeadersByFlow(tc, pkt):
    return

def GetPacketTemplateByFlow(tc, pkt):
    flow = tc.config.flow
    template = 'ETH'
    if flow.IsIP():
        template += "_%s_%s" % (flow.type, flow.proto)
    else:
        assert(0)
    return infra_api.GetPacketTemplate(template)
