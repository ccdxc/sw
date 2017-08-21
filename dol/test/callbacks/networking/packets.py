# /usr/bin/python3
import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api

def GetTtl(testcase, packet):
    return 129
def GetRoutedTtl(testcase, packet):
    return 128

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

def AddIngressHeadersByFlow(testcase, packet):
    return

def AddEgressHeadersByFlow(testcase, packet):
    return

def __get_packet_template_impl(flow):
    template = 'ETH'
    if flow.IsIP():
        template += "_%s_%s" % (flow.type, flow.proto)
    else:
        assert(0)
    return infra_api.GetPacketTemplate(template)
   
def GetPacketTemplateByFlow(testcase, packet):
    return __get_packet_template_impl(testcase.config.flow)

def GetPacketTemplateBySessionIflow(testcase, packet):
    return  __get_packet_template_impl(testcase.config.session.iconfig.flow)

def GetPacketTemplateBySessionRflow(testcase, packet):
    return  __get_packet_template_impl(testcase.config.session.rconfig.flow)
