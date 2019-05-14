#! /usr/bin/python3
import enum
import pdb
import ipaddress
import sys

#Following come from dol/infra
import infra.config.base as base
import infra.common.defs as defs
import infra.common.parser as parser

import iota.test.apollo.config.resmgr as resmgr
import iota.test.apollo.config.utils as utils
import iota.test.apollo.config.agent.api as agent_api

import tunnel_pb2 as tunnel_pb2
import types_pb2 as types_pb2
from iota.harness.infra.utils.logger import Logger as logger
from iota.test.apollo.config.store import Store

class TunnelObject(base.ConfigObjectBase):
    def __init__(self, device, local, remote = None, type = tunnel_pb2.TUNNEL_TYPE_WORKLOAD):
        super().__init__()
        self.Id = next(resmgr.TunnelIdAllocator)
        self.GID("Tunnel%d"%self.Id)

        ################# PUBLIC ATTRIBUTES OF TUNNEL OBJECT #####################
        self.EncapValue = 0
        self.Nat = False
        #if (hasattr(spec, 'nat')):
        #    self.Nat = spec.nat
        self.LocalIPAddr = local.IPAddr
        self.device = device
        if remote == None:
            self.RemoteIPAddr = self.LocalIPAddr
            self.Type = tunnel_pb2.TUNNEL_TYPE_NONE
        else:
            self.RemoteIPAddr = remote.IPAddr
            #TODO based on config gen
            if type == tunnel_pb2.TUNNEL_TYPE_WORKLOAD:
                self.Type = tunnel_pb2.TUNNEL_TYPE_WORKLOAD
                self.RemoteVnicMplsSlotIdAllocator = resmgr.CreateRemoteVnicMplsSlotAllocator()
                self.RemoteVnicVxlanIdAllocator = resmgr.CreateRemoteVnicVxlanIdAllocator()
            else:
            #    if parent.IsEncapTypeMPLS():
            #        self.EncapValue = next(resmgr.IGWMplsSlotIdAllocator)
            #    else:
                self.Type = type
                self.EncapValue = next(resmgr.IGWVxlanIdAllocator)

        ################# PRIVATE ATTRIBUTES OF TUNNEL OBJECT #####################

        self.Show()
        return

    def __repr__(self):
        return "Tunnel%d|LocalIPAddr:%s|RemoteIPAddr:%s|TunnelType:%s|EncapValue:%d|Nat:%s" %\
               (self.Id,self.LocalIPAddr, self.RemoteIPAddr, utils.GetTunnelTypeString(self.Type), self.EncapValue, self.Nat)

    def GetGrpcCreateMessage(self):
        grpcmsg = tunnel_pb2.TunnelRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.Id
        spec.VPCId = 0 # TODO: Create Substrate VPC
        utils.GetRpcEncap(self.device.name, self.EncapValue, self.EncapValue, spec.Encap)
        spec.Type = self.Type
        spec.LocalIP.Af = types_pb2.IP_AF_INET
        spec.LocalIP.V4Addr = int(self.LocalIPAddr)
        spec.RemoteIP.Af = types_pb2.IP_AF_INET
        spec.RemoteIP.V4Addr = int(self.RemoteIPAddr)
        spec.Nat = self.Nat
        return grpcmsg

    def IsWorkload(self):
        if self.Type == tunnel_pb2.TUNNEL_TYPE_WORKLOAD:
            return True
        return False

    def IsIgw(self):
        if self.Type == tunnel_pb2.TUNNEL_TYPE_IGW:
            return True
        return False

    def IsNat(self):
        if self.Nat is True:
            return True
        return False

    def Show(self):
        logger.info("Tunnel Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

class TunnelObjectClient:
    def __init__(self, device, client):
        self.__lobjs = []
        self.__client = client
        self.__tunnel_map = {}
        self.device = device
        return

    def Objects(self):
        return self.__lobjs


    def GenerateLocalTunnelObject(self, parent):
        self.__lobjs.append(TunnelObject(self.device, local=parent))


    def GetRemoteTunnel(self, device):
        return self.__tunnel_map.get(device.ID)

    def AddRemoteTunnel(self, local, remote):
        tun_obj = TunnelObject(self.device, local=local, remote=remote)
        self.__tunnel_map[remote.ID] = tun_obj
        self.__lobjs.append(tun_obj)

    def AddInternetTunnel(self, local, remote):
        obj = TunnelObject(self.device, local=local, remote=remote, type=tunnel_pb2.TUNNEL_TYPE_IGW)
        self.__lobjs.append(obj)
        Store.AddTunnel(obj.ID(), obj)


    def GenerateObjects(self, devices, tunnelspec):
        # Generate Local Tunnel object
        #self.__lobjs.append(TunnelObject(parent, None, True))
        resmgr.CreateInternetTunnels()
        resmgr.CreateVnicTunnels()
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__lobjs))
        self.__client.Create(agent_api.ObjectTypes.TUNNEL, msgs)
        return

#client = TunnelObjectClient()

#def GetMatchingObjects(selectors):
#    return client.Objects()
