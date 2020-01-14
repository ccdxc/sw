# /usr/bin/python3
import pdb
from scapy.layers.l2 import Dot1Q

import apollo.config.objects.vpc as vpc
import apollo.config.utils as utils
import apollo.config.topo as topo
from apollo.config.store import EzAccessStore

from infra.common.logging import logger
import iris.test.callbacks.common.pktslicer as pktslicer

def __get_mirror_object(testcase, args):
    vnic = testcase.config.localmapping.VNIC
    objs = vnic.RxMirrorObjs if args.direction == 'RX' else vnic.TxMirrorObjs
    return objs.get(args.id, None)

def GetSPANPortID(testcase, args):
    mirrorObj = __get_mirror_object(testcase, args)
    if not mirrorObj:
        return topo.PortTypes.NONE
    if mirrorObj.SpanType == 'RSPAN':
        return utils.GetPortIDfromInterface(mirrorObj.Interface)
    elif mirrorObj.SpanType == 'ERSPAN':
        # underlay vpc, return switchport
        spanvpc = vpc.client.GetVpcObject(EzAccessStore.GetDUTNode(), mirrorObj.VPCId)
        if spanvpc.IsUnderlayVPC():
            return topo.PortTypes.SWITCH
        # TODO: tenant vpc support post impl & p4 support
        # tenant vpc and local mapping, return hostport
        # tenant vpc and remote mapping, return switchport
    return topo.PortTypes.NONE

def GetSPANVlanID(testcase, packet, args=None):
    mirrorObj = __get_mirror_object(testcase, args)
    # For ERSPAN, vlan is 0
    return mirrorObj.VlanId if mirrorObj and mirrorObj.SpanType == 'RSPAN' else 0

def GetERSPANDirection(testcase, packet, args=None):
    # TODO: check 'd' value. shouldn't it be opposite?
    return 1 if args.direction == 'RX' else 0

def GetERSPANDscp(testcase, packet, args=None):
    mirrorObj = __get_mirror_object(testcase, args)
    return mirrorObj.Dscp if mirrorObj and mirrorObj.SpanType == 'ERSPAN' else 0

def GetERSPANSessionId(testcase, packet, args=None):
    mirrorObj = __get_mirror_object(testcase, args)
    return (mirrorObj.SpanID - 1) if mirrorObj and mirrorObj.SpanType == 'ERSPAN' else 0

def GetERSPANSrcIP(testcase, packet, args=None):
    mirrorObj = __get_mirror_object(testcase, args)
    return str(mirrorObj.SrcIP) if mirrorObj and mirrorObj.SpanType == 'ERSPAN' else "0"

def GetERSPANDstIP(testcase, packet, args=None):
    mirrorObj = __get_mirror_object(testcase, args)
    return str(mirrorObj.DstIP) if mirrorObj and mirrorObj.SpanType == 'ERSPAN' else "0"

def GetERSPANDstMac(testcase, packet, args=None):
    mirrorObj = __get_mirror_object(testcase, args)
    if not mirrorObj or mirrorObj.SpanType != 'ERSPAN':
        return "00:00:00:00:00:00"
    # underlay vpc, return TEP MAC
    spanvpc = vpc.client.GetVpcObject(EzAccessStore.GetDUTNode(), mirrorObj.VPCId)
    if spanvpc.IsUnderlayVPC():
        return "00:02:0b:0a:0d:0e"
    # TODO: tenant vpc support post impl & p4 support
    # tenant vpc and local mapping, return localmapping/VNIC/MACAddr
    # tenant vpc and remote mapping, return TEP MAC behind remotemapping
    return "00:00:00:00:00:00"

def GetExpectedPacket(testcase, args):
    mirrorObj = __get_mirror_object(testcase, args)
    if not mirrorObj:
        return None
    return testcase.packets.Get(mirrorObj.SpanType + '_MIRROR_PKT_' + args.direction + '_' + str(args.id))

def GetERSPANCos(testcase, inpkt, args):
    pkt = testcase.packets.db[args.pktid].GetScapyPacket()
    return pkt[Dot1Q].prio

def __is_erspan_truncate(basepktsize, truncatelen):
    if truncatelen == 0 or (truncatelen >= (basepktsize - utils.ETHER_HDR_LEN)):
        # No truncate
        return 0
    else:
        # truncate
        return 1

def GetERSPANTruncate(testcase, inpkt, args):
    mirrorObj = __get_mirror_object(testcase, args)
    if not mirrorObj or mirrorObj.SpanType != 'ERSPAN':
        return 0
    basepkt = testcase.packets.Get(args.basepktid)
    return __is_erspan_truncate(basepkt.size, mirrorObj.SnapLen)

def GetERSPANPayloadSize(testcase, inpkt, args):
    mirrorObj = __get_mirror_object(testcase, args)
    if not mirrorObj or mirrorObj.SpanType != 'ERSPAN':
        return 0
    snaplen = mirrorObj.SnapLen
    basepkt = testcase.packets.Get(args.basepktid)
    if __is_erspan_truncate(basepkt.size, snaplen):
        plen = snaplen + utils.ETHER_HDR_LEN
    else:
        plen = basepkt.size
    return plen

def GetERSPANPayload(testcase, inpkt, args):
    mirrorObj = __get_mirror_object(testcase, args)
    if not mirrorObj or mirrorObj.SpanType != 'ERSPAN':
        return []
    snaplen = mirrorObj.SnapLen
    basepkt = testcase.packets.Get(args.basepktid)
    args.pktid = args.basepktid
    if __is_erspan_truncate(basepkt.size, snaplen):
        args.end = snaplen + utils.ETHER_HDR_LEN
    else:
        args.end = basepkt.size
    return pktslicer.GetPacketSlice(testcase, basepkt, args)

def __is_rspan_truncate(basepktsize, truncatelen):
    if truncatelen == 0 or (truncatelen >= (basepktsize - utils.ETHER_HDR_LEN - utils.DOT1Q_HDR_LEN)):
        # No truncate
        return 0
    else:
        # truncate
        return 1

def GetRSPANPayloadSize(testcase, inpkt, args):
    mirrorObj = __get_mirror_object(testcase, args)
    if not mirrorObj or mirrorObj.SpanType != 'RSPAN':
        return 0
    snaplen = mirrorObj.SnapLen
    pkt = testcase.packets.Get(args.pktid)
    if __is_rspan_truncate(pkt.size, snaplen):
        plen = snaplen + utils.ETHER_HDR_LEN + utils.DOT1Q_HDR_LEN
    else:
        plen = pkt.size
    return plen

def GetRSPANPayload(testcase, inpkt, args):
    mirrorObj = __get_mirror_object(testcase, args)
    if not mirrorObj or mirrorObj.SpanType != 'RSPAN':
        return []
    snaplen = mirrorObj.SnapLen
    pkt = testcase.packets.Get(args.pktid)
    args.pktid = args.pktid
    if __is_rspan_truncate(pkt.size, snaplen):
        args.end = snaplen + utils.ETHER_HDR_LEN + utils.DOT1Q_HDR_LEN
    else:
        args.end = pkt.size
    return pktslicer.GetPacketSlice(testcase, pkt, args)
