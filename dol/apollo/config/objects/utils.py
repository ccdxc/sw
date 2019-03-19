#! /usr/bin/python3
import sys
import types_pb2 as types_pb2
import tunnel_pb2 as tunnel_pb2
from infra.common.logging import logger

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

def GetEncapType(e):
    if e == 'vxlan':
        return types_pb2.ENCAP_TYPE_VXLAN
    elif e == 'mplsoudp':
        return types_pb2.ENCAP_TYPE_MPLSoUDP
    else:
        logger.error("ERROR: Invalid/Unknown Encap: %s" % e)
        sys.exit(1)
        return None
