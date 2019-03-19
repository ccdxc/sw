#! /usr/bin/python3
import enum
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.tunnel as tunnel
import apollo.config.objects.utils as utils

import device_pb2 as device_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class DeviceObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__()
        self.GID("Device1")
        
        ################# PUBLIC ATTRIBUTES OF SWITCH OBJECT #####################
        self.LocalIP = next(resmgr.TepIpAddressAllocator)
        self.Gateway = next(resmgr.TepIpAddressAllocator)
        self.MACAddress = spec.macaddress

        ################# PRIVATE ATTRIBUTES OF SWITCH OBJECT #####################
        self.__spec = spec

        self.Show()
        tunnel.client.GenerateObjects(self, spec.tunnel)
        return

    def __repr__(self):
        return "Device1/LocalIP:%s/Gateway:%s/MAC:%s" %\
               (self.LocalIP, self.Gateway, self.MACAddress.get())

    def GetLocalIP(self):
        return self.LocalIP

    def GetLocalMac(self):
        return self.MACAddress.get()

    def GetGrpcCreateMessage(self):
        grpcmsg = device_pb2.DeviceRequest()
        grpcmsg.Request.IPAddr.Af = types_pb2.IP_AF_INET
        grpcmsg.Request.IPAddr.V4Addr = int(self.LocalIP)
        grpcmsg.Request.GatewayIP.Af = types_pb2.IP_AF_INET
        grpcmsg.Request.GatewayIP.V4Addr = int(self.Gateway)
        grpcmsg.Request.MACAddr = self.MACAddress.getnum()
        return grpcmsg

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

    def GenerateObjects(self, topospec):
        obj = DeviceObject(topospec.device)
        self.__objs.append(obj)
        Store.SetSwitch(obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.SWITCH, msgs)
        tunnel.client.CreateObjects()
        return

client = DeviceObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
