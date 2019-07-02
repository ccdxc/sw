#! /usr/bin/python3
import sys
import enum
import ipaddress
import socket
from random import sample
import types_pb2 as types_pb2
import tunnel_pb2 as tunnel_pb2
from infra.common.logging import logger
from apollo.config.store import Store
from infra.common.glopts import GlobalOptions

IP_VERSION_6 = 6
IP_VERSION_4 = 4

L3PROTO_MIN = 0

L4PORT_MIN = 0
L4PORT_MAX = 65535

ICMPTYPE_MIN = 0
ICMPTYPE_MAX = 255

ICMPCODE_MIN = 0
ICMPCODE_MAX = 255

ETHER_HDR_LEN = 14
DOT1Q_HDR_LEN = 4

IPV4_MINADDR = ipaddress.ip_address("0.0.0.0")
IPV4_MAXADDR = ipaddress.ip_address("255.255.255.255")

IPV6_MINADDR = ipaddress.ip_address("0::0")
IPV6_MAXADDR = ipaddress.ip_address("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff")

IPV4_DEFAULT_ROUTE = ipaddress.ip_network("0.0.0.0/0")
IPV6_DEFAULT_ROUTE = ipaddress.ip_network("0::/0")

IPPROTO_TO_NAME_TBL = {num:name[8:] for name,num in vars(socket).items() if name.startswith("IPPROTO")}

class PortTypes(enum.IntEnum):
    NONE = 0
    HOST = 1
    SWITCH = 2

"""
    # Eth1/1 0x11010001 ==> 1 Hostport
    # Eth2/1 0x11020001 ==> 2 Switchport
"""
INTF2PORT_TBL = { 0x11010001: PortTypes.HOST, 0x11020001: PortTypes.SWITCH}

class L3MatchType(enum.IntEnum):
    PFX = 0
    PFXRANGE = 1
    TAG = 2

class rrobiniter:
    def __init__(self, objs):
        assert len(objs) != 0
        self.objs = objs
        self.iterator = iter(objs)
        self.size = len(objs)
    def rrnext(self):
        while True:
            try:
                return next(self.iterator)
            except:
                self.iterator = iter(self.objs)
                continue
    def size(self):
        return self.size

def GetFilteredObjects(objs, maxlimits, random=True):
    if maxlimits is None or maxlimits is 0 or maxlimits >= len(objs):
        num = len(objs)
    else:
        num = maxlimits
    if random:
        return sample(objs, k=num)
    return objs[0:num]

def GetIPProtoName(proto):
    """
        returns IP Protocol name for the given protocol number
    """
    return IPPROTO_TO_NAME_TBL[proto]

def __get_subnet(ip, prev=False):
    """
        returns next subnet of 'ip' if 'prev' is False else previous subnet
    """
    totalhosts = -1 if prev else 1
    totalhosts *= ip.num_addresses
    newpfx = str(ip.network_address + totalhosts) + '/' + str(ip.prefixlen)
    return ipaddress.ip_network(newpfx)

def GetNextSubnet(ip):
    return __get_subnet(ip)

def GetPreviousSubnet(ip):
    return __get_subnet(ip, prev=True)

def GetTunnelType(e):
    if e == 'internet-gateway':
        return tunnel_pb2.TUNNEL_TYPE_IGW
    elif e == 'workload':
        return tunnel_pb2.TUNNEL_TYPE_WORKLOAD
    elif e == 'service' or e == 'remoteservice':
        return tunnel_pb2.TUNNEL_TYPE_SERVICE
    else:
        return tunnel_pb2.TUNNEL_TYPE_NONE

def GetTunnelTypeString(e):
    if e == tunnel_pb2.TUNNEL_TYPE_IGW:
        return "internet-gateway"
    elif e == tunnel_pb2.TUNNEL_TYPE_WORKLOAD:
        return "workload"
    elif e == tunnel_pb2.TUNNEL_TYPE_SERVICE:
        return "service"
    elif e == tunnel_pb2.TUNNEL_TYPE_NONE:
        return "None"
    else:
        logger.error("ERROR: Invalid/Unknown Tunnel Type: %s" % e)
        sys.exit(1)
        return None

def GetEncapType(e):
    if e == 'vxlan':
        return types_pb2.ENCAP_TYPE_VXLAN
    elif e == 'mplsoudp':
        return types_pb2.ENCAP_TYPE_MPLSoUDP
    else:
        logger.error("ERROR: Invalid/Unknown Encap: %s" % e)
        sys.exit(1)
        return None

def GetEncapTypeString(e):
    if e == types_pb2.ENCAP_TYPE_VXLAN:
        return "vxlan"
    elif e == types_pb2.ENCAP_TYPE_MPLSoUDP:
        return "mplsoudp"
    else:
        logger.error("ERROR: Invalid/Unknown Encap: %s" % e)
        sys.exit(1)
        return None

def isDefaultRoute(ippfx):
    if ippfx == IPV4_DEFAULT_ROUTE or ippfx == IPV6_DEFAULT_ROUTE:
        return True
    return False

def isDefaultAddrRange(addrLow, addrHigh):
    if addrLow == IPV4_MINADDR and addrHigh == IPV4_MAXADDR:
        return True
    if addrLow == IPV6_MINADDR and addrHigh == IPV6_MAXADDR:
        return True
    return False

def isTagWithDefaultRoute(tag):
    #TODO: Tag support
    return False

def GetRpcIPAddrFamily(ipaddrfamily):
    if ipaddrfamily == 'IPV6':
        return types_pb2.IP_AF_INET6
    elif ipaddrfamily == 'IPV4':
        return types_pb2.IP_AF_INET
    else:
        return types_pb2.IP_AF_NONE

def GetRpcIPPrefix(srcpfx, dstpfx):
    dstpfx.Len = srcpfx.prefixlen
    if srcpfx.version == IP_VERSION_6:
        dstpfx.Addr.Af = types_pb2.IP_AF_INET6
        dstpfx.Addr.V6Addr = srcpfx.network_address.packed
    else:
        dstpfx.Addr.Af = types_pb2.IP_AF_INET
        dstpfx.Addr.V4Addr = int(srcpfx.network_address)

def GetRpcIPAddr(srcaddr, dstaddr):
    if srcaddr.version == IP_VERSION_6:
        dstaddr.Af = types_pb2.IP_AF_INET6
        dstaddr.V6Addr = srcaddr.packed
    else:
        dstaddr.Af = types_pb2.IP_AF_INET
        dstaddr.V4Addr = int(srcaddr)

def GetRpcIPRange(addrLow, addrHigh, addrRange):
    if addrLow.version != addrHigh.version:
        logger.error("ERROR: addrRange version mismatch: Low %s High %s" %(addrLow, addrHigh))
        sys.exit(1)
    if addrLow.version == IP_VERSION_6:
        GetRpcIPAddr(addrLow, addrRange.IPv6Range.Low)
        GetRpcIPAddr(addrHigh, addrRange.IPv6Range.High)
    else:
        GetRpcIPAddr(addrLow, addrRange.IPv4Range.Low)
        GetRpcIPAddr(addrHigh, addrRange.IPv4Range.High)
    return

def GetRpcEncap(mplsslot, vxlanid, encap):
    encap.type = Store.GetDeviceEncapType()
    if Store.IsDeviceEncapTypeMPLS():
         encap.value.MPLSTag  = mplsslot
    else:
         encap.value.Vnid  = vxlanid

def GetPortIDfromInterface(interfaceid):
    return INTF2PORT_TBL.get(interfaceid, PortTypes.NONE)

def IsPipelineArtemis():
    if GlobalOptions.pipeline == 'artemis':
        return True
    return False
