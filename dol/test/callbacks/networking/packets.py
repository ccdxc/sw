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
    return __get_packet_template_impl(testcase.config.session.iconfig.flow)

def GetPacketTemplateBySessionRflow(testcase, packet):
    return __get_packet_template_impl(testcase.config.session.rconfig.flow)

def __get_template(tid):
    if tid == None: return None
    return infra_api.GetPacketTemplate(tid)

def __get_packet_encap_vlan(cfg):
    if cfg.segment.native == False:
        return __get_template('ENCAP_QTAG')
    return None

def __get_packet_encap_vxlan(cfg):
    if cfg.tenant.IsOverlayVxlan():
        return __get_template('ENCAP_VXLAN')
    return None

def __get_packet_encaps(cfg):
    encaps = []
    # Check for VLAN encap
    encap = __get_packet_encap_vlan(cfg)
    if encap: encaps.append(encap)
    # Check for VXLAN encap
    encap = __get_packet_encap_vxlan(cfg)
    if encap: encaps.append(encap)
    
    if len(encaps): return encaps
    return None
   

def GetPacketEncaps(testcase, packet):
    return __get_packet_encaps(testcase.config.src)

def GetExpectedPacketEncaps(testcase, packet):
    return __get_packet_encaps(testcase.config.dst)

def GetExpectedPacketCos(testcase, packet):
    if testcase.config.flow.eg_qos.cos_rw.get():
        return testcase.config.flow.eg_qos.cos.get()

    if not testcase.config.flow.IsL2() and testcase.config.src.segment.native:
        # L3 Packet: Untag to Tag: Cos will always be zero.
        return 0
    return testcase.config.flow.eg_qos.cos.get() 


def GetL2UcExpectedPacket(testcase):
    if testcase.config.flow.IsDrop():
        return None
    return testcase.packets.Get('EXP_PKT')

