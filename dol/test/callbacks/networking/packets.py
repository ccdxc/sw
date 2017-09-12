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

def IsPriorityTagged(pvtdata):
    if 'priotag' in pvtdata.__dict__:
        return pvtdata.priotag
    return False

def __get_template(tid):
    if tid == None: return None
    return infra_api.GetPacketTemplate(tid)

def __get_packet_encap_vlan(testcase, cfg):
    if cfg.segment.native == False or IsPriorityTagged(testcase.pvtdata):
        return __get_template('ENCAP_QTAG')
    
    if cfg.endpoint:
        if cfg.endpoint.remote == True: 
            return None

        if cfg.endpoint.intf.IsUseg() or\
            cfg.endpoint.intf.IsPvlan():
            return __get_template('ENCAP_QTAG')
    return None

def __get_packet_encap_vxlan(testcase, cfg):
    if testcase.config.flow.IsNat() == False and\
        cfg.endpoint.remote == False:
        return None
    if cfg.tenant.IsOverlayVxlan():
        return __get_template('ENCAP_VXLAN')
    return None

def __get_packet_encaps(testcase, cfg):
    encaps = []

    # Check for VLAN encap
    encap = __get_packet_encap_vlan(testcase, cfg)
    if encap:
        encaps.append(encap)

    # Check for VXLAN encap
    encap = __get_packet_encap_vxlan(testcase, cfg)
    if encap:
        encaps.append(encap)
    
    if len(encaps):
        return encaps
    return None

def GetPacketEncaps(testcase, packet):
    return __get_packet_encaps(testcase, testcase.config.src)

def GetExpectedPacketEncaps(testcase, packet):
    ptag = IsPriorityTagged(testcase.pvtdata)
    testcase.pvtdata.priotag = False
    encaps = __get_packet_encaps(testcase, testcase.config.dst)
    testcase.pvtdata.priotag = ptag
    return encaps

def GetExpectedPacketCos(testcase, packet):
    if testcase.config.flow.eg_qos.cos_rw.get():
        return testcase.config.flow.eg_qos.cos.get()

    # If we sent pkt with a tag, we should receive with same cos we sent with
    if testcase.config.src.segment.native == False or \
            IsPriorityTagged(testcase.pvtdata) == True:
        return testcase.config.flow.eg_qos.cos.get() 

    return 0

def __get_expected_packet(testcase):
    if testcase.config.flow.IsDrop():
        return None
    return testcase.packets.Get('EXP_PKT')

def GetL2UcExpectedPacket(testcase):
    return __get_expected_packet(testcase)

def GetL3UcExpectedPacket(testcase):
    return __get_expected_packet(testcase)

def GetVlanId(testcase, packet):
    if IsPriorityTagged(testcase.pvtdata):
        return 0
    return testcase.config.src.segment.vlan_id

def GetPacketRawBytes(testcase, packet):
    packet.Build(testcase)
    return packet.GetPacketRawBytes()

def GetPacketSize(testcase, packet):
    packet.Build(testcase)
    return packet.GetPacketSize()

def GetEthertypeByFlow(testcase, packet):
    if testcase.config.flow.IsMac():
        return testcase.config.flow.ethertype
    elif testcase.config.flow.IsIPV4():
        return 0x800
    return 0x86dd
