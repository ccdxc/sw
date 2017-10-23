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

def __get_packet_template_impl(flow, args):
    template = 'ETH'
    if flow.IsIP():
        template += "_%s_%s" % (flow.type, flow.proto)
    else:
        assert(0)
  
    if args is not None:
        template += "_%s" % (args.proto)

    return infra_api.GetPacketTemplate(template)

def GetPacketTemplateByFlow(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.flow, args)

def GetPacketTemplateBySessionIflow(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.session.iconfig.flow, args)

def GetPacketTemplateBySessionRflow(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.session.rconfig.flow, args)

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
    if cfg.segment.IsFabEncapVxlan():
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
    if testcase.config.flow.txqos.cos is not None:
        return testcase.config.flow.txqos.cos

    # If we sent pkt with a tag, we should receive with same cos we sent with
    if testcase.config.src.segment.native == False or \
        IsPriorityTagged(testcase.pvtdata) == True:
        return testcase.config.flow.txqos.cos

    # For Host TX cases they have a qtag
    if testcase.config.src.endpoint and\
        testcase.config.src.endpoint.remote == False:
        return testcase.config.flow.txqos.cos

    return 0

def GetExpectedVlanId(testcase, packet):
    if testcase.config.dst.segment.IsFabEncapVxlan():
        return testcase.config.dst.endpoint.intf.vlan_id
    return testcase.config.dst.segment.vlan_id

def GetExpectedPacketQtag(testcase, args=None):
    assert(testcase.config.dst.endpoint.remote == False)    # Always Host RX
    if testcase.config.src.segment.native is False or IsPriorityTagged(testcase.pvtdata):
        pri = testcase.config.flow.txqos.cos
    else:
        pri = 0
    vlan_id = testcase.config.dst.endpoint.intf.encap_vlan_id
    qtag = (pri << 13) + vlan_id
    assert(qtag <= 0xffff)
    return qtag

def __get_expected_packet(testcase, args, config=None):
    root = getattr(testcase.config, 'flow', None)
    if root is None:
       if config is None:
           config = testcase.config.session.iconfig
       root = getattr(config, 'flow', None)

    if root.IsDrop():
        return None

    if args is None:
        return testcase.packets.Get('EXP_PKT')

    return testcase.packets.Get(args.expktid)

def GetL2UcExpectedPacket(testcase, args = None):
    return __get_expected_packet(testcase, args)

def GetCpuPacket(testcase, args = None):
    root = getattr(testcase.config, 'flow', None)
    if root is None:
       root = getattr(testcase.config.session.iconfig, 'flow', None)

    if root.IsFteEnabled():
        return testcase.packets.Get(args.expktid)
    return None

def GetCpuPacketbyIflow(testcase, args = None):
    root = getattr(testcase.config, 'flow', None)
    if root is None:
        root = getattr(testcase.config.session.iconfig, 'flow', None)

    if root.IsFteEnabled():
        return testcase.packets.Get(args.expktid)
 
    return None

def GetCpuPacketbyRflow(testcase, args = None):
    root = getattr(testcase.config, 'flow', None)
    if root is None:
        root = getattr(testcase.config.session.rconfig, 'flow', None)

    if root.IsFteEnabled():
        return testcase.packets.Get(args.expktid)

    return None

def GetL3UcExpectedPacket(testcase, args = None):
    return __get_expected_packet(testcase, args)

def GetVlanId(testcase, packet):
    if IsPriorityTagged(testcase.pvtdata):
        return 0
    elif testcase.config.src.segment.IsFabEncapVxlan():
        return testcase.config.src.endpoint.intf.vlan_id
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

def GetIngressPortIPSGDrop(testcase):
    sif = testcase.config.src.endpoint.GetInterface()
    # Only for Uplinks change the port
    if sif.type == 'UPLINK' or \
            sif.type == 'UPLINK_PC':
        sport = set([1, 2, 3, 4]) - set(sif.ports)

    return list(sport)[:1]

def GetExpectDelay(testcase):
    root = getattr(testcase.config, 'flow', None)
    if root is None:
       root = getattr(testcase.config.session.iconfig, 'flow', None)

    if root.IsFteEnabled():
        return 2
    return 0

def PacketIsVlanTagged(testcase, args=None):
    return 0 if __get_packet_encap_vlan(testcase, testcase.config.dst) is None else 1
