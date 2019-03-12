#! /usr/bin/python3
import enum
import pdb
import ipaddress
import sys

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import tunnel_pb2 as tunnel_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class TunnelObject(base.ConfigObjectBase):
    def __init__(self, parent, spec):
        super().__init__()
        self.Id = next(resmgr.TunnelIdAllocator)
        self.GID("Tunnel%d"%self.Id)
        self.__spec = spec

        ################# PUBLIC ATTRIBUTES OF TUNNEL OBJECT #####################
        self.LocalIP = parent.LocalIP
        self.RemoteIP = next(resmgr.TepIpAddressAllocator)
        self.Encap = self.__get_tunnel_encap(spec.encap)

        ################# PRIVATE ATTRIBUTES OF TUNNEL OBJECT #####################

        self.Show()
        return

    def __get_tunnel_encap(self, e):
        if e == 'vxlan':
            return tunnel_pb2.TUNNEL_ENCAP_NONE
        elif e == 'mplsudp_tags1':
            return tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_1
        elif e == 'mplsudp_tags2':
            return tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_2
        else:
            logger.error("ERROR: Invalid/Unknown Tunnel Encap: %s" % e)
            sys.exit(1)
        return None

    def __repr__(self):
        return "Tunnel%d/Encap:%s/LocalIP:%s/RemoteIP:%s" %\
               (self.Id, self.Encap, self.LocalIP, self.RemoteIP)

    def GetGrpcCreateMessage(self):
        grpcmsg = tunnel_pb2.TunnelSpec()
        grpcmsg.Id = self.Id
        grpcmsg.PCNId = 0 # TODO: Create Substrate PCN
        grpcmsg.Encap = self.Encap
        grpcmsg.LocalIP.Af = types_pb2.IP_AF_INET
        grpcmsg.LocalIP.V4Addr = int(self.LocalIP)
        grpcmsg.RemoteIP.Af = types_pb2.IP_AF_INET
        grpcmsg.RemoteIP.V4Addr = int(self.RemoteIP)
        return grpcmsg
   
    def Show(self):
        logger.info("Tunnel Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

class TunnelObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, parent, tunnelspec):
        for t in tunnelspec:
            for c in range(t.count):
                obj = TunnelObject(parent, t)
                self.__objs.append(obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.TUNNEL, msgs)
        return

client = TunnelObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
