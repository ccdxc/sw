#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from apollo.config.store import Store

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.base as base
import apollo.config.objects.tunnel as tunnel
import apollo.config.utils as utils

import device_pb2 as device_pb2
import types_pb2 as types_pb2

class DeviceObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__()
        self.SetBaseClassAttr()
        self.GID("Device1")

        self.stack = getattr(spec, 'stack', 'ipv4')
        ################# PUBLIC ATTRIBUTES OF SWITCH OBJECT #####################
        self.Mode = getattr(spec, 'mode', 'bitw')
        self.BridgingEnabled = getattr(spec, 'bridging', False)
        self.LearningEnabled = getattr(spec, 'learning', False)
        #TODO: based on stack, get ip & gw addr
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

    def Show(self):
        logger.info("Device Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def SetBaseClassAttr(self):
        self.ObjType = api.ObjectTypes.SWITCH
        return

    def PopulateKey(self, grpcmsg):
        return

    def PopulateSpec(self, grpcmsg):
        grpcmsg.Request.IPAddr.Af = types_pb2.IP_AF_INET
        grpcmsg.Request.IPAddr.V4Addr = int(self.IPAddr)
        grpcmsg.Request.GatewayIP.Af = types_pb2.IP_AF_INET
        grpcmsg.Request.GatewayIP.V4Addr = int(self.GatewayAddr)
        grpcmsg.Request.MACAddr = self.MACAddr.getnum()
        if self.Mode == "bitw":
            grpcmsg.Request.DevOperMode = device_pb2.DEVICE_OPER_MODE_BITW
        elif self.Mode == "host":
            grpcmsg.Request.DevOperMode = device_pb2.DEVICE_OPER_MODE_HOST
        grpcmsg.Request.BridgingEn = self.BridgingEnabled
        grpcmsg.Request.LearningEn = self.LearningEnabled
        return

    def GetGrpcReadMessage(self):
        grpcmsg = types_pb2.Empty()
        return grpcmsg

    def IsBitwMode(self):
        if self.Mode == "bitw":
            return True
        return False

    def IsHostMode(self):
        if self.Mode == "host":
            return True
        return False

    def IsEncapTypeMPLS(self):
        if self.EncapType == types_pb2.ENCAP_TYPE_MPLSoUDP:
            return True
        return False

    def IsEncapTypeVXLAN(self):
        if self.EncapType == types_pb2.ENCAP_TYPE_VXLAN:
            return True
        return False

    def GetDropStats(self):
        grpcmsg = types_pb2.Empty()
        resp = api.client.Get(api.ObjectTypes.SWITCH, [ grpcmsg ])
        if resp != None:
            return resp[0]
        return None

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
