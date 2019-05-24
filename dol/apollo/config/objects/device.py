#! /usr/bin/python3
import enum
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.tunnel as tunnel
import apollo.config.utils as utils

import device_pb2 as device_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class DeviceObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__()
        self.GID("Device1")

        ################# PUBLIC ATTRIBUTES OF SWITCH OBJECT #####################
        self.IPAddr = next(resmgr.TepIpAddressAllocator)
        self.GatewayAddr = next(resmgr.TepIpAddressAllocator)
        self.MACAddr = spec.macaddress
        self.IP = str(self.IPAddr) # For testspec
        self.EncapType = utils.GetEncapType(spec.encap)

        ################# PRIVATE ATTRIBUTES OF SWITCH OBJECT #####################
        self.__spec = spec

        self.Show()
        tunnel.client.GenerateObjects(self, spec.tunnel)
        return

    def __repr__(self):
        return "Device1|IPAddr:%s|GatewayAddr:%s|MAC:%s|Encap:%s" %\
               (self.IPAddr, self.GatewayAddr, self.MACAddr.get(),
               utils.GetEncapTypeString(self.EncapType))

    def GetGrpcCreateMessage(self):
        grpcmsg = device_pb2.DeviceRequest()
        grpcmsg.Request.IPAddr.Af = types_pb2.IP_AF_INET
        grpcmsg.Request.IPAddr.V4Addr = int(self.IPAddr)
        grpcmsg.Request.GatewayIP.Af = types_pb2.IP_AF_INET
        grpcmsg.Request.GatewayIP.V4Addr = int(self.GatewayAddr)
        grpcmsg.Request.MACAddr = self.MACAddr.getnum()
        return grpcmsg

    def IsEncapTypeMPLS(self):
        if self.EncapType == types_pb2.ENCAP_TYPE_MPLSoUDP:
            return True
        return False

    def IsEncapTypeVXLAN(self):
        if self.EncapType == types_pb2.ENCAP_TYPE_VXLAN:
            return True
        return False

    def Show(self):
        logger.info("Device Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

class DeviceObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def IsValidConfig(self):
        count = len(self.__objs)
        if  count > resmgr.MAX_DEVICE:
            return False, "Device count %d exceeds allowed limit of %d" %\
                          (count, resmgr.MAX_DEVICE)
        return True, ""

    def GenerateObjects(self, topospec):
        obj = DeviceObject(topospec.device)
        self.__objs.append(obj)
        Store.SetDevice(obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.SWITCH, msgs)
        tunnel.client.CreateObjects()
        return

client = DeviceObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
