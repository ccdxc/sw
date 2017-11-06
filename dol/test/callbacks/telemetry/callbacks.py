#! /usr/bin/python3
import pdb
import test.callbacks.common.pktslicer as pktslicer
import test.telemetry.telemetry as telemetry

def GetExpectedPacket(testcase, args):
    case = testcase.pvtdata.span_case
    ssns = telemetry.spanSessionData.getSessions(case)
    if args.sessionid > len(ssns):
        return None
    (id, direc, spantype, pkt, intf, pktlen) = ssns[args.sessionid -1]
    if pktlen == 0:
        return testcase.packets.Get(pkt)
    else:
        pktname = "TRUNCATE_PKT" + str(args.sessionid)
        return testcase.packets.Get(pktname)

def GetExpectedPort(testcase, args):
    case = testcase.pvtdata.span_case
    ssns = telemetry.spanSessionData.getSessions(case)
    if args.sessionid > len(ssns):
        return None
    (id, direc, spantype, pkt, intf, pktlen) = ssns[args.sessionid -1]
    ssn = "session"+str(id)
    erspanid = 1
    if spantype == "ERSPAN":
        for i in range(0, args.sessionid):
            (id, direc, spantype, pkt, intf, pktlen) = ssns[args.sessionid -1]
            if spantype == "ERSPAN":
                erspanid = erspanid + 1
        tnl = telemetry.data.getErspanSession(args.sessionid)
        return tnl.ports
 
    if direc == "ingress":
        if 'ports' not in testcase.config.ingress_mirror.__dict__[ssn].intf.__dict__:
            ports = []
            ports.append(testcase.config.ingress_mirror.__dict__[ssn].intf)
            return ports
        else:
            return testcase.config.ingress_mirror.__dict__[ssn].intf.ports
    elif direc == "egress":
        if 'ports' not in testcase.config.egress_mirror.__dict__[ssn].intf.__dict__:
            ports = []
            ports.append(testcase.config.egress_mirror.__dict__[ssn].intf)
            return ports
        else:
            return testcase.config.egress_mirror.__dict__[ssn].intf.ports
    return None

def GetRspanVlan(testcase, packet):
    return testcase.config.ingress_mirror.__dict__["session2"].segment.vlan_id

def GetTruncatePktSize(testcase, pkt, args):
    case = testcase.pvtdata.span_case
    ssns = telemetry.spanSessionData.getSessions(case)
    if args.sessionid > len(ssns):
        return 0
    (id, direc, spantype, pkt, intf, pktlen) = ssns[args.sessionid -1]
    return pktlen
    

def GetTruncatePacketPayload(testcase, pkt, args):
    case = testcase.pvtdata.span_case
    ssns = telemetry.spanSessionData.getSessions(case)
    if args.sessionid > len(ssns):
        args.end = 0
        args.pktid = "TRUNCATE_PKT1"
        return pktslicer.GetPacketSlice(testcase, pkt, args)
    (id, direc, spantype, pkt, intf, pktlen) = ssns[args.sessionid -1]
    args.end = pktlen
    retpkt = testcase.packets.Get(pkt)
    args.pktid = pkt
    return pktslicer.GetPacketSlice(testcase, retpkt, args)

def GetTunnelSrcMac(testcase, pkt, args):
    tnl = telemetry.data.getErspanSession(args.id)
    if tnl is not None:
        return tnl.remote_ep.segment.macaddr
    return None


def GetTunnelDstMac(testcase, pkt, args):
    tnl = telemetry.data.getErspanSession(args.id)
    if tnl is not None:
        return tnl.remote_ep.macaddr
    return None

def GetTunnelVlanEncap(testcase, pkt, args):
    tnl = telemetry.data.getErspanSession(args.id)
    if tnl is not None:
        return tnl.remote_ep.segment.vlan_id
    return None

def GetTunnelSourceIP(testcase, pkt, args):
    tnl = telemetry.data.getErspanSession(args.id)
    if tnl is not None:
        return tnl.GetSrcIp()
    return None

def GetTunnelDestIP(testcase, pkt, args):
    tnl = telemetry.data.getErspanSession(args.id)
    if tnl is not None:
        return tnl.GetDestIp()
    return None

