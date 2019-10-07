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

import apollo.config.objects.lif as lif

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
        #TODO: Move LIF creation to appropriate interface once that is supported
        self.lifns = getattr(spec, 'lifns', None)
        if utils.IsHostLifSupported() and self.lifns:
            # Process Host LIFs
            self.obj_helper_lif = lif.LifObjectHelper()
            self.__create_lifs()

        self.Show()
        tunnel.client.GenerateObjects(self, spec.tunnel)
        return

    def __repr__(self):
        return "Device1|IPAddr:%s|GatewayAddr:%s|MAC:%s|Encap:%s" %\
               (self.IPAddr, self.GatewayAddr, self.MACAddr.get(),
               utils.GetEncapTypeString(self.EncapType))

    def __create_lifs(self):
        lif = self.__spec.lif.Get(Store)
        self.obj_helper_lif.Generate(lif, self.lifns)
        self.obj_helper_lif.Configure()
        #TODO: Fix once interface.py is available
        self.lif = self.obj_helper_lif.GetRandomHostLif()
        logger.info(" Selecting %s for Test" % self.lif.GID())
        self.lif.Show()
        return

    def GetGrpcCreateMessage(self, cookie):
        grpcmsg = device_pb2.DeviceRequest()
        grpcmsg.BatchCtxt.BatchCookie = cookie
        grpcmsg.Request.IPAddr.Af = types_pb2.IP_AF_INET
        grpcmsg.Request.IPAddr.V4Addr = int(self.IPAddr)
        grpcmsg.Request.GatewayIP.Af = types_pb2.IP_AF_INET
        grpcmsg.Request.GatewayIP.V4Addr = int(self.GatewayAddr)
        grpcmsg.Request.MACAddr = self.MACAddr.getnum()
        return grpcmsg

    def GetGrpcReadMessage(self):
        grpcmsg = device_pb2.DeviceRequest()
        return grpcmsg

    def Update(self):
        self.old = copy.deepcopy(self)
        self.IPAddr = next(resmgr.TepIpAddressAllocator)
        self.GatewayAddr = next(resmgr.TepIpAddressAllocator)
        self.IP = str(self.IPAddr) # For testspec

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

    def GetDropStats(self):
        grpcmsg = types_pb2.Empty()
        resp = api.client.Get(api.ObjectTypes.SWITCH, [ grpcmsg ])
        if resp != None:
            return resp[0]
        return None

    def Equals(self, obj, spec):
        return True

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
        cookie = utils.GetBatchCookie()
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__objs))
        api.client.Create(api.ObjectTypes.SWITCH, msgs)
        tunnel.client.CreateObjects()
        return

    def ReadObjects(self):
        msgs = list(map(lambda x: x.GetGrpcReadMessage(), self.__objs))
        api.client.Get(api.ObjectTypes.SWITCH, msgs)
        return

client = DeviceObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
