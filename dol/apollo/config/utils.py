#! /usr/bin/python3
import sys
import types_pb2 as types_pb2
import tunnel_pb2 as tunnel_pb2
from infra.common.logging import logger

IP_VERSION_6 = 6
IP_VERSION_4 = 4

class rrobiniter:
    def __init__(self, objs):
        assert len(objs) != 0
        self.objs = objs
        self.iterator = iter(objs)
    def rrnext(self):
        while True:
            try:
                return next(self.iterator)
            except:
                self.iterator = iter(self.objs)
                continue

def GetTunnelEncapType(e):
    if e == 'vxlan':
        return tunnel_pb2.TUNNEL_ENCAP_VXLAN
    elif e == 'mplsudp_tags1':
        return tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_1
    elif e == 'mplsudp_tags2':
        return tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_2
    else:
        logger.error("ERROR: Invalid/Unknown Tunnel Encap: %s" % e)
        sys.exit(1)
        return None

def GetTunnelEncapString(e):
    if e == tunnel_pb2.TUNNEL_ENCAP_VXLAN:
        return "vxlan"
    elif e == tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_1:
        return "mplsoudp-tag1"
    elif e == tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_2:
        return "mplsoudp-tag2"
    else:
        logger.error("ERROR: Invalid/Unknown Tunnel Encap: %s" % e)
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
