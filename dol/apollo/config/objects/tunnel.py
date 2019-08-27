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
        self.EncapValue = 0
        self.Nat = False
        if (hasattr(spec, 'nat')):
            self.Nat = spec.nat
        if local == True:
            self.RemoteIPAddr = self.LocalIPAddr
            self.Type = tunnel_pb2.TUNNEL_TYPE_NONE
        else:
            self.Type = utils.GetTunnelType(spec.type)
            if self.Type == tunnel_pb2.TUNNEL_TYPE_WORKLOAD:
                self.RemoteIPAddr = next(resmgr.TepIpAddressAllocator)
                self.RemoteVnicMplsSlotIdAllocator = resmgr.CreateRemoteVnicMplsSlotAllocator()
                self.RemoteVnicVxlanIdAllocator = resmgr.CreateRemoteVnicVxlanIdAllocator()
            elif self.Type == tunnel_pb2.TUNNEL_TYPE_IGW:
                self.RemoteIPAddr = next(resmgr.TepIpAddressAllocator)
                if parent.IsEncapTypeMPLS():
                    self.EncapValue = next(resmgr.IGWMplsSlotIdAllocator)
                else:
                    self.EncapValue = next(resmgr.IGWVxlanIdAllocator)
            elif self.Type == tunnel_pb2.TUNNEL_TYPE_SERVICE:
                self.RemoteIPAddr = next(resmgr.TepIpv6AddressAllocator)
                if hasattr(spec, "remote") and spec.remote is True:
                    self.Remote = True
                    self.RemoteServicePublicIP = next(resmgr.RemoteSvcTunIPv4Addr)
                    self.RemoteServiceEncap = next(resmgr.IGWVxlanIdAllocator)
                else:
                    self.Remote = False
                self.EncapValue = next(resmgr.IGWVxlanIdAllocator)
        self.RemoteIP = str(self.RemoteIPAddr) # for testspec
        self.MACAddr = resmgr.TepMacAllocator.get()
        ################# PRIVATE ATTRIBUTES OF TUNNEL OBJECT #####################

        self.Show()
        return

    def __repr__(self):
        remote = ""
        if hasattr(self, "Remote") and self.Remote is True:
            remote = " Remote:%s"% (self.Remote)
        return "Tunnel%d|LocalIPAddr:%s|RemoteIPAddr:%s|TunnelType:%s%s|" \
               "EncapValue:%d|Nat:%s|Mac:%s" % \
               (self.Id,self.LocalIPAddr, self.RemoteIPAddr,
               utils.GetTunnelTypeString(self.Type), remote, self.EncapValue,
               self.Nat, self.MACAddr)

    def GetGrpcCreateMessage(self):
        grpcmsg = tunnel_pb2.TunnelRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.Id
        spec.VPCId = 0 # TODO: Create Substrate VPC
        utils.GetRpcEncap(self.EncapValue, self.EncapValue, spec.Encap)
        spec.Type = self.Type
        utils.GetRpcIPAddr(self.LocalIPAddr, spec.LocalIP)
        utils.GetRpcIPAddr(self.RemoteIPAddr, spec.RemoteIP)
        spec.Nat = self.Nat
        if utils.IsPipelineArtemis():
            spec.MACAddress = self.MACAddr.getnum()
            if self.Type is tunnel_pb2.TUNNEL_TYPE_SERVICE and self.Remote is True:
                spec.RemoteService = self.Remote
                utils.GetRpcIPAddr(self.RemoteServicePublicIP, spec.RemoteServicePublicIP)
                utils.GetRpcEncap(self.RemoteServiceEncap, self.RemoteServiceEncap, spec.RemoteServiceEncap)
        return grpcmsg

    def GetGrpcReadMessage(self):
        grpcmsg = tunnel_pb2.TunnelGetRequest()
        grpcmsg.Id.append(self.Id)
        return grpcmsg

    def IsWorkload(self):
        if self.Type == tunnel_pb2.TUNNEL_TYPE_WORKLOAD:
            return True
        return False

    def IsIgw(self):
        if self.Type == tunnel_pb2.TUNNEL_TYPE_IGW:
            return True
        return False

    def IsSvc(self):
        if self.Type == tunnel_pb2.TUNNEL_TYPE_SERVICE:
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
    def __init__(self):
        self.__objs = []
        self.__lobjs = []
        return

    def Objects(self):
        return self.__objs

    def IsValidConfig(self):
        count = len(self.__objs)
        if  count > resmgr.MAX_TUNNEL:
            return False, "Tunnel count %d exceeds allowed limit of %d" % \
                          (count, resmgr.MAX_TUNNEL)
        return True, ""

    def GenerateObjects(self, parent, tunnelspec):
        # Generate Local Tunnel object
        self.__lobjs.append(TunnelObject(parent, None, True))
        # Generate Remote Tunnel object
        for t in tunnelspec:
            for c in range(t.count):
                if (t.type == "service") and not \
                    utils.IsPipelineArtemis():
                        continue
                else:
                    obj = TunnelObject(parent, t, False)
                    self.__objs.append(obj)
        Store.SetTunnels(self.__objs)
        resmgr.CreateInternetTunnels()
        resmgr.CreateVnicTunnels()
        resmgr.CollectSvcTunnels()
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__lobjs))
        api.client.Create(api.ObjectTypes.TUNNEL, msgs)
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.TUNNEL, msgs)
        return

    def GetGrpcReadAllMessage(self):
        grpcmsg = tunnel_pb2.TunnelGetRequest()
        return grpcmsg

    def ReadObjects(self):
        msg = self.GetGrpcReadAllMessage()
        api.client.Get(api.ObjectTypes.TUNNEL, [msg])
        return

client = TunnelObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
