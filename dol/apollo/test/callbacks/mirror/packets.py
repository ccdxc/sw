# /usr/bin/python3
import pdb
import apollo.config.objects.vpc as vpc
import apollo.config.utils as utils

from infra.common.logging import logger
import iris.test.callbacks.common.pktslicer as pktslicer

def __get_mirror_object(testcase, args):
    vnic = testcase.config.localmapping.VNIC
    objs = vnic.RxMirrorObjs if args.direction == 'RX' else vnic.TxMirrorObjs
    return objs.get(args.id, None)

def GetSPANPortID(testcase, args):
    mirrorObj = __get_mirror_object(testcase, args)
    if not mirrorObj:
        return utils.PortTypes.NONE
    if mirrorObj.SpanType == 'RSPAN':
        return utils.GetPortIDfromInterface(mirrorObj.Interface)
    elif mirrorObj.SpanType == 'ERSPAN':
        # substrate vpc, return switchport
        spanvpc = vpc.client.GetVpcObject(mirrorObj.VPCId)
        if spanvpc.IsSubstrateVPC():
            return utils.PortTypes.SWITCH
        # TODO: tenant vpc support post impl & p4 support
        # tenant vpc and local mapping, return hostport
        # tenant vpc and remote mapping, return switchport
    return utils.PortTypes.NONE

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
    # substrate vpc, return TEP MAC
    spanvpc = vpc.client.GetVpcObject(mirrorObj.VPCId)
    if spanvpc.IsSubstrateVPC():
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

def GetERSPANPayloadSize(testcase, inpkt, args):
    mirrorObj = __get_mirror_object(testcase, args)
    if not mirrorObj or mirrorObj.SpanType != 'ERSPAN':
        return 0
    snaplen = mirrorObj.SnapLen
    basepkt = testcase.packets.Get(args.basepktid)
    # TODO: mirror snaplen is not working. remove this once it works
    snaplen = basepkt.size
    if snaplen == 0 or basepkt.size < snaplen:
        snaplen = basepkt.size
    logger.info("GetErspanPayloadSize returning %d %d" % (basepkt.size, mirrorObj.SnapLen))
    return snaplen

def GetERSPANPayload(testcase, inpkt, args):
    mirrorObj = __get_mirror_object(testcase, args)
    if not mirrorObj or mirrorObj.SpanType != 'ERSPAN':
        return []
    snaplen = mirrorObj.SnapLen
    basepkt = testcase.packets.Get(args.basepktid)
    # TODO: mirror snaplen is not working. remove this once it works
    snaplen = basepkt.size
    args.pktid = args.basepktid
    args.end = snaplen
    if snaplen == 0 or basepkt.size < snaplen:
        args.end = basepkt.size
    logger.info("GetErspanPayload returning %s size %d" % (args.pktid, args.end))
    return pktslicer.GetPacketSlice(testcase, basepkt, args)
