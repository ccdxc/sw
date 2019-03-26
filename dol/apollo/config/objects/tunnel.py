#! /usr/bin/python3
import enum
import pdb
import ipaddress
import sys

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.utils as utils
import apollo.config.agent.api as api
import tunnel_pb2 as tunnel_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class TunnelObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, local):
        super().__init__()
        self.__spec = spec
        self.Id = next(resmgr.TunnelIdAllocator)
        self.GID("Tunnel%d"%self.Id)

        ################# PUBLIC ATTRIBUTES OF TUNNEL OBJECT #####################
        self.LocalIPAddr = parent.IPAddr
        if local == True:
            self.RemoteIPAddr = self.LocalIPAddr
            self.Encap = tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_2
        else:
            self.RemoteIPAddr = next(resmgr.TepIpAddressAllocator)
            self.Encap = utils.GetTunnelEncapType(spec.encap)
            if self.Encap == tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_2:
               self.RemoteVnicMplsSlotIdAllocator = resmgr.CreateRemoteVnicMplsSlotAllocator()

        ################# PRIVATE ATTRIBUTES OF TUNNEL OBJECT #####################

        self.Show()
        return

    def __repr__(self):
        return "Tunnel%d|Encap:%s|LocalIPAddr:%s|RemoteIPAddr:%s" %\
               (self.Id, utils.GetTunnelEncapString(self.Encap),
               self.LocalIPAddr, self.RemoteIPAddr)

    def GetGrpcCreateMessage(self):
        grpcmsg = tunnel_pb2.TunnelRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.Id
        spec.VPCId = 0 # TODO: Create Substrate VPC
        spec.Encap = self.Encap
        spec.LocalIP.Af = types_pb2.IP_AF_INET
        spec.LocalIP.V4Addr = int(self.LocalIPAddr)
        spec.RemoteIP.Af = types_pb2.IP_AF_INET
        spec.RemoteIP.V4Addr = int(self.RemoteIPAddr)
        return grpcmsg

    def IsMplsOverUdp2(self):
        if self.Encap == tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_2:
            return True
        return False

    def IsMplsOverUdp1(self):
        if self.Encap == tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_1:
            return True
        return False

    def Show(self):
        logger.info("Tunnel Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

class TunnelObjectClient:
    def __init__(self):
        self.__objs = []
        self.__lobjs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, parent, tunnelspec):
        # Generate Local Tunnel object
        self.__lobjs.append(TunnelObject(parent, None, True))
        # Generate Remote Tunnel object
        for t in tunnelspec:
            for c in range(t.count):
                obj = TunnelObject(parent, t, False)
                self.__objs.append(obj)
        Store.SetTunnels(self.__objs)
        resmgr.CreateMplsInternetTunnels()
        resmgr.CreateMplsVnicTunnels()
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__lobjs))
        api.client.Create(api.ObjectTypes.TUNNEL, msgs)
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.TUNNEL, msgs)
        return

client = TunnelObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
