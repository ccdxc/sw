#! /usr/bin/python3
import sys
import enum
import ipaddress
import socket
from random import sample
from scapy.layers.l2 import Dot1Q
import time

import types_pb2 as types_pb2
import tunnel_pb2 as tunnel_pb2
from infra.common.logging import logger
from apollo.config.store import Store
import apollo.config.agent.api as api
from infra.common.glopts import GlobalOptions

UINT32_MIN = 0
UINT32_MAX = 4294967295

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

class InterfaceTypes(enum.IntEnum):
    NONE = 0
    ETH = 1
    ETH_PC = 2
    TUNNEL = 3
    MGMT = 4
    UPLINK = 5
    UPLINKPC = 6
    L3 = 7
    LIF = 8

class PortTypes(enum.IntEnum):
    NONE = 0
    HOST = 1
    SWITCH = 2

"""
    # Eth1/1 0x11010001 ==> 1 Hostport
    # Eth2/1 0x11020001 ==> 2 Switchport
"""
INTF2PORT_TBL = { 0x11010001: PortTypes.HOST, 0x11020001: PortTypes.SWITCH }
MODE2INTF_TBL = { 'host' : InterfaceTypes.ETH, 'switch': InterfaceTypes.UPLINK,
        'uplink': InterfaceTypes.UPLINK, 'l3': InterfaceTypes.L3 }


IF_TYPE_SHIFT = 28
LIF_IF_LIF_ID_MASK = 0xFFFFFF

def LifId2LifIfIndex(lifid):
    return ((InterfaceTypes.LIF << IF_TYPE_SHIFT) | (lifid))

def LifIfindex2LifId(lififindex):
    return (lififindex & LIF_IF_LIF_ID_MASK)

class NhType(enum.IntEnum):
    NONE = 0
    OVERLAY = 2
    UNDERLAY = 4
    UNDERLAY_ECMP = 5
    IP = 8

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

class CachedObjs:

    def __init__(self):
        self.select_objs = False
        self.use_selected_objs = False
        self.objs = []

    def add(self, obj):
        self.objs.extend(obj)

    def reset(self):
        self.__init__()

    def setMaxLimits(self, maxlimits):
        self.maxlimits = maxlimits

    def getMaxLimits(self):
        return self.maxlimits

CachedObjs = CachedObjs()

def GetFilteredObjects(objs, maxlimits, random=False):
    if maxlimits is None or maxlimits is 0 or maxlimits >= len(objs):
        num = len(objs)
    else:
        num = maxlimits
    if random:
        return sample(objs, k=num)
    return objs[0:num]

def IsDryRun():
    return GlobalOptions.dryrun

def Sleep(timeout=1):
    if IsDryRun():
        return
    time.sleep(timeout)
    return

def ValidateGrpcResponse(resp, expApiStatus=types_pb2.API_STATUS_OK):
    return expApiStatus == resp.ApiStatus

def ValidateObject(obj, resp):
    return obj.ValidateSpec(resp.Spec) and\
           obj.ValidateStats(resp.Stats) and\
           obj.ValidateStatus(resp.Status)

def ValidateCreate(obj, resps):
    if IsDryRun(): return
    for resp in resps:
        if ValidateGrpcResponse(resp):
            obj.MarkDeleted(False)
        else:
            logger.error("Creation/Restoration failed for %s" %(obj.__repr__()))
            obj.Show()
    return

def ValidateRead(obj, resps, expApiStatus=types_pb2.API_STATUS_OK):
    if IsDryRun(): return
    for resp in resps:
        if ValidateGrpcResponse(resp, expApiStatus):
            if ValidateGrpcResponse(resp):
                for response in resp.Response:
                    if not ValidateObject(obj, response):
                        return False
        else:
            return False
    return True

def ValidateDelete(obj, resps):
    if IsDryRun(): return
    for resp in resps:
        for status in resp.ApiStatus:
            if status == types_pb2.API_STATUS_OK:
                obj.MarkDeleted()
            else:
                logger.error("Deletion failed for %s" %(obj.__repr__()))
                obj.Show()
    return

def GetBatchCookie():
    batchClient = Store.GetBatchClient()
    obj = batchClient.Objects()
    return obj.GetBatchCookie()

def CreateObject(obj, objType):
    if not obj.IsDeleted():
        logger.info("Already restored %s" %(obj.__repr__()))
        return
    batchClient = Store.GetBatchClient()
    batchClient.Start()
    cookie = GetBatchCookie()
    msg = obj.GetGrpcCreateMessage(cookie)
    resps = api.client.Create(objType, [msg])
    ValidateCreate(obj, resps)
    batchClient.Commit()

def ReadObject(obj, objType, expRetCode):
    msg = obj.GetGrpcReadMessage()
    resps = api.client.Get(objType, [msg])
    return ValidateRead(obj, resps, expRetCode)

def DeleteObject(obj, objType):
    if obj.IsDeleted():
        logger.info("Already deleted %s" %(obj.__repr__()))
        return
    batchClient = Store.GetBatchClient()
    batchClient.Start()
    cookie = GetBatchCookie()
    msg = obj.GetGrpcDeleteMessage(cookie)
    resps = api.client.Delete(objType, [msg])
    ValidateDelete(obj, resps)
    batchClient.Commit()

def GetIPProtoByName(protoname):
    """
        returns IP Protocol number for the given protocol name
    """
    return socket.getprotobyname(protoname)

def GetIPProtoName(proto):
    """
        returns IP Protocol name for the given protocol number
    """
    return IPPROTO_TO_NAME_TBL[proto]

def IsICMPProtocol(proto):
    """
        returns True if given proto is icmp/icmpv6
    """
    return 'ICMP' in GetIPProtoName(proto)

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
    tagpfxlist = tag.Prefixes if tag else None
    if tagpfxlist is None:
        return False
    for tagpfx in tagpfxlist:
        if isDefaultRoute(tagpfx):
            return True
    return False

def GetIPVersion(ipaf):
    if ipaf == 'IPV6':
        return IP_VERSION_6
    return IP_VERSION_4

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

def GetRpcIPv4Prefix(srcpfx, dstpfx):
    dstpfx.Len = srcpfx.prefixlen
    dstpfx.Addr = int(srcpfx.network_address)

def GetRpcIPv6Prefix(srcpfx, dstpfx):
    dstpfx.Len = srcpfx.prefixlen
    dstpfx.Addr = srcpfx.network_address.packed

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

def GetRpcDirection(direction):
    if direction == "ingress":
         return types_pb2.RULE_DIR_INGRESS
    elif direction == "egress":
         return types_pb2.RULE_DIR_EGRESS
    return types_pb2.RULE_DIR_NONE

def GetPortIDfromInterface(interfaceid):
    return INTF2PORT_TBL.get(interfaceid, PortTypes.NONE)

def IsPipelineArtemis():
    if GlobalOptions.pipeline == 'artemis':
        return True
    return False

def IsPipelineApulu():
    if GlobalOptions.pipeline == 'apulu':
        return True
    return False

def IsPipelineApollo():
    if GlobalOptions.pipeline == 'apollo':
        return True
    return False

def GetPipelineName():
    return GlobalOptions.pipeline

def IsHostLifSupported():
    if IsPipelineArtemis():
        return False
    return True

def IsInterfaceSupported():
    if IsPipelineArtemis():
        return False
    return True

def IsL3InterfaceSupported():
    if IsPipelineApulu():
        return True
    return False

def IsIGWTunnelSupported():
    if IsPipelineApulu():
        return False
    return True

def IsWorkloadTunnelSupported():
    if IsPipelineApulu():
        return False
    return True

def IsServiceTunnelSupported():
    if IsPipelineArtemis():
        return True
    return False

def IsUnderlayTunnelSupported():
    if IsPipelineApulu():
        return True
    return False

def IsRouteTableSupported():
    if IsPipelineApulu():
        return False
    return True

def IsFlowInstallationNeeded():
    if IsPipelineArtemis():
        return True
    return False

def IsV4Stack(stack):
    return True if ((stack == "dual") or (stack == 'ipv4')) else False

def IsV6Stack(stack):
    return True if ((stack == "dual") or (stack == 'ipv6')) else False

def GetVlanHeaderSize(packet):
    pkt = packet.GetScapyPacket()
    if Dot1Q in pkt:
        return DOT1Q_HDR_LEN
    return 0

def MergeFilteredObjects(objs, selected_objs):
    if CachedObjs.select_objs is True:
        CachedObjs.add(objs)
    elif CachedObjs.use_selected_objs is True:
        objs.extend(selected_objs)

