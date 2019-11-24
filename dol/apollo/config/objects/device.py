#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from apollo.config.store import Store
from apollo.config.objects.nexthop_group import client as NhGroupClient

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.base as base
import apollo.config.objects.tunnel as tunnel
import apollo.config.utils as utils

import device_pb2 as device_pb2
import types_pb2 as types_pb2

class DeviceObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__(api.ObjectTypes.DEVICE)
        self.GID("Device1")

        self.stack = getattr(spec, 'stack', 'ipv4')
        ################# PUBLIC ATTRIBUTES OF DEVICE OBJECT #####################
        self.Mode = getattr(spec, 'mode', 'auto')
        if self.Mode == 'auto':
            self.Mode = utils.GetDefaultDeviceMode()
        self.BridgingEnabled = getattr(spec, 'bridging', False)
        self.LearningEnabled = getattr(spec, 'learning', False)
        #TODO: based on stack, get ip & gw addr
        self.IPAddr = next(resmgr.TepIpAddressAllocator)
        self.GatewayAddr = next(resmgr.TepIpAddressAllocator)
        self.MACAddr = resmgr.DeviceMacAllocator.get()
        self.IP = str(self.IPAddr) # For testspec
        self.EncapType = utils.GetEncapType(spec.encap)

        ################# PRIVATE ATTRIBUTES OF DEVICE OBJECT #####################
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

    def ValidateSpec(self, spec):
        if utils.ValidateRpcIPAddr(self.IPAddr, spec.IPAddr) is False:
            return False
        if utils.ValidateRpcIPAddr(self.GatewayAddr, spec.GatewayIP) is False:
            return False
        # TODO: fix this
        if utils.IsPipelineArtemis() is False:
            if spec.MACAddr != self.MACAddr.getnum():
                return False
        if utils.IsPipelineApulu() is True:
            if self.Mode == "bitw":
                if spec.DevOperMode != device_pb2.DEVICE_OPER_MODE_BITW:
                    return False
            elif self.Mode == "host":
                if spec.DevOperMode != device_pb2.DEVICE_OPER_MODE_HOST:
                    return False
        return True

    def ValidateYamlSpec(self, spec):
        if utils.IsPipelineApulu() is True:
            if self.Mode == "bitw":
                if spec['devopermode'] != device_pb2.DEVICE_OPER_MODE_BITW:
                    return False
            elif self.Mode == "host":
                if spec['devopermode'] != device_pb2.DEVICE_OPER_MODE_HOST:
                    return False
        return True

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
        resp = api.client.Get(api.ObjectTypes.DEVICE, [ grpcmsg ])
        if resp != None:
            return resp[0]
        return None

class DeviceObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.DEVICE, resmgr.MAX_DEVICE)
        return

    def GenerateObjects(self, topospec):
        obj = DeviceObject(topospec.device)
        self.Objs.update({0: obj})
        Store.SetDevice(obj)
        return

    def CreateObjects(self):
        super().CreateObjects()

        # Create Nexthop group object before tunnel as tep impl looks up nhgroup
        NhGroupClient.CreateObjects()
        tunnel.client.CreateObjects()
        return

    def GetGrpcReadAllMessage(self):
        grpcmsg = types_pb2.Empty()
        return grpcmsg

    def ValidateGrpcRead(self, getResp):
        if utils.IsDryRun(): return True
        for obj in getResp:
            if not utils.ValidateGrpcResponse(obj):
                logger.error("GRPC get request failed for ", obj)
                return False
            resp = obj.Response
            device = self.GetObjectByKey(0)
            if not utils.ValidateObject(device, resp):
                logger.error("GRPC read validation failed for  ", obj)
                device.Show()
                return False
        return True

    def ValidatePdsctlRead(self, ret, stdout):
        if utils.IsDryRun(): return True
        if not ret:
            logger.error("pdsctl show cmd failed for ", self.ObjType)
            return False
        # split output per object
        cmdop = stdout.split("---")
        for op in cmdop:
            yamlOp = utils.LoadYaml(op)
            if not yamlOp:
                continue
            device = self.GetObjectByKey(0)
            resp = yamlOp['response']
            if not utils.ValidateObject(device, resp, yaml=True):
                logger.error("pdsctl read validation failed for ", op)
                device.Show()
                return False
        return True

client = DeviceObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
