#! /usr/bin/python3
import pdb
from infra.common.logging import logger as logger

def GetTxExpectedPacket(tc, args = None):
    return tc.packets.Get(args.expktid)

def GetRxExpectedPacket(tc, args = None):
    return tc.packets.Get(args.expktid)

def GetTxExpectedPorts(tc, args = None):
    assert(tc.config.dst.endpoint.remote is True)
    return tc.config.src.intf.pinnedif.ports

def GetRxTriggerPorts(tc, args = None):
    assert(tc.config.src.endpoint.remote is True)
    return tc.config.dst.endpoint.pinintf.ports

def GetCpuPacket(tc, args = None):
    return tc.packets.Get(args.expktid)

def __get_enic(tc, args):
    if args is None:
        return None
    
    oiflist = tc.pvtdata.pruned_oiflist
    if len(oiflist) == 0 or len(oiflist) < args.idx:
        pdb.set_trace()
        return None
    enic = oiflist[args.idx]
    return enic

def GetPromiscuousEnicRx(tc, args = None):
    enic = tc.config.src.tenant.GetPromiscuousEnic()
    rx = enic.lif.queue_types.Get("RX")
    logger.info("PromiscuousRxRing: Enic:%s QueueType:%s" % (enic.GID(), rx.GID()))
    return rx

def GetEncapVlanIdForMulticastCopy(tc, pkt, args = None):
    enic = __get_enic(tc, args)
    logger.info("MulticastCopy: Enic:%s EncapVlanId:%d" % (enic.GID(), enic.encap_vlan_id))
    return enic.encap_vlan_id

def GetMulticastEnicRx(tc, args = None):
    #assert(tc.config.src.endpoint.remote == False)
    #enic = tc.config.src.endpoint.intf
    enic = __get_enic(tc, args)
    rx = enic.lif.queue_types.Get("RX")
    logger.info("MulticastCopy: Enic:%s QueueType:%s" % (enic.GID(), rx.GID()))
    return rx

def GetPortsForMulticastCopy(tc, args = None):
    if args is None:
        return None
    oiflist = tc.config.src.segment.floodlist.uplink_list
    if len(oiflist) < args.idx:
        return None
    upintf = oiflist[args.idx]
    logger.info("MulticastCopy: Returning Ports for %s" % upintf.GID())
    return oiflist[args.idx].ports

def GetTxPortsForMulticastCopy(tc, args = None):
    if args is None:
        return None
    upintf = tc.config.src.endpoint.pinintf
    logger.info("MulticastCopy: Returning Ports for %s" % upintf.GID())
    return upintf.ports

def GetRxUplinkPorts(tc, args = None):
    assert(tc.config.src.endpoint.remote is True)
    if tc.pvtdata.scenario != 'RPF_FAILURE':
        return tc.config.src.segment.pinnedif.ports
    
    # RPF Failure testcase.
    pin_ports = tc.config.src.segment.pinnedif.ports
    rpf_fail_ports = set([1, 2, 3, 4]) - set(pin_ports)
    return list(rpf_fail_ports)[:1]

def GetExpectedBufferSizeForMulticastCopy(tc, pkt, args=None):
    assert (tc.config.src.endpoint.remote is True)
    if args != None and tc.pvtdata.scenario != 'RPF_FAILURE':
        return tc.packets.Get(args.id).size
    else:
        return 0

def GetExpectedBufferDataForMulticastCopy(tc, pkt, args=None):
    assert (tc.config.src.endpoint.remote is True)
    if args != None and tc.pvtdata.scenario != 'RPF_FAILURE':
        return tc.packets.Get(args.id).rawbytes
    else:
        return None

def GetMulticastPacketDMAC(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    grpobj = tc.config.flow.GetMulticastGroup()
    return grpobj.group.get()

def GetMulticastPacketSMAC(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return tc.config.src.endpoint.macaddr

def GetMulticastPacketDIP(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    grpobj = tc.config.flow.GetMulticastGroup()
    return grpobj.gip.get()

def GetMulticastPacketSIP(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return tc.config.src.endpoint.GetIpAddress()

def GetMulticastPacketDIP6(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    grpobj = tc.config.flow.GetMulticastGroup()
    return grpobj.gip.get()

def GetMulticastPacketSIP6(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return tc.config.src.endpoint.GetIpv6Address()

def GetMulticastPacketUdpSport(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return 42000

def GetMulticastPacketUdpDport(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return 43000

def GetMulticastPacketTcpSport(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return 42000

def GetMulticastPacketTcpDport(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return 43000

def GetPacketIcmpCode(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return 1

def GetPacketIcmpType(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return 1

def GetPacketIcmpId(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return 1

def GetPacketIcmp6Code(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return 1

def GetPacketIcmp6Type(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return 1

def GetPacketIcmp6Id(tc, pkt, args = None):
    assert(tc.config.flow.IsMulticast())
    return 1

