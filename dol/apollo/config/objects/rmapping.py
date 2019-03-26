#! /usr/bin/python3
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.utils as utils
import apollo.config.agent.api as api
import mapping_pb2 as mapping_pb2
import types_pb2 as types_pb2
import tunnel_pb2 as tunnel_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class RemoteMappingObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, tunobj, ipversion):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF MAPPING OBJECT #####################
        self.MappingId = next(resmgr.RemoteMappingIdAllocator)
        self.GID('RemoteMapping%d'%self.MappingId)
        self.SUBNET = parent
        self.MACAddr = resmgr.RemoteMappingMacAllocator.get()
        self.Encap = utils.GetEncapType(spec.encap)
        self.TunIPAddr = tunobj.RemoteIPAddr
        self.MplsSlot =  next(tunobj.RemoteVnicMplsSlotIdAllocator)
        if ipversion == utils.IP_VERSION_6:
            self.IPAddr = parent.AllocIPv6Address();
            self.AddrFamily = 'IPV6'
        else:
            self.IPAddr = parent.AllocIPv4Address();
            self.AddrFamily = 'IPV4'
        self.Label = 'NETWORKING'
        self.FlType = "MAPPING"
        self.IP = str(self.IPAddr) # For testspec
        self.TunIP = str(self.TunIPAddr) # For testspec

        ################# PRIVATE ATTRIBUTES OF MAPPING OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "RemoteMappingID:%d|SubnetId:%d|VPCId:%d" %\
               (self.MappingId, self.SUBNET.SubnetId, self.SUBNET.VPC.VPCId)

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.flow.filters)

    def GetGrpcCreateMessage(self):
        grpcmsg = mapping_pb2.MappingRequest()
        spec = grpcmsg.Request.add()
        spec.Id.VPCId = self.SUBNET.VPC.VPCId
        utils.GetRpcIPAddr(self.IPAddr, spec.Id.IPAddr)
        spec.SubnetId = self.SUBNET.SubnetId
        spec.TunnelId = int(self.TunIPAddr)
        spec.MACAddr = self.MACAddr.getnum()
        spec.Encap.type = self.Encap
        spec.Encap.value.MPLSTag  = self.MplsSlot
        return grpcmsg

    def Show(self):
        logger.info("RemoteMapping object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- IPAddr:%s|TunIPAddr:%s|MAC:%s" %\
                (str(self.IPAddr), str(self.TunIPAddr), self.MACAddr))
        return

    def SetupTestcaseConfig(self, obj):
        return

class RemoteMappingObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, parent, subnet_spec_obj):
        if getattr(subnet_spec_obj, 'rmap', None) == None:
            return
            
        stack = parent.VPC.Stack

        for rmap_spec_obj in subnet_spec_obj.rmap:
            c = 0
            while c < rmap_spec_obj.count:
                tunobj = resmgr.RemoteMplsVnicTunAllocator.rrnext()
                if stack == "dual" or stack == 'ipv6':
                    obj = RemoteMappingObject(parent, rmap_spec_obj, tunobj, utils.IP_VERSION_6)
                    self.__objs.append(obj)
                    c = c + 1
                if c < rmap_spec_obj.count and (stack == "dual" or stack == 'ipv4'):
                    obj = RemoteMappingObject(parent, rmap_spec_obj, tunobj, utils.IP_VERSION_4)
                    self.__objs.append(obj)
                    c = c + 1
        return

    def CreateObjects(self):
        if len(self.__objs) == 0:
            return
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.MAPPING, msgs)
        return

client = RemoteMappingObjectClient()

def GetMatchingObjects(selectors):
    objs = []
    for obj in client.Objects():
        if obj.IsFilterMatch(selectors) == True:
            objs.append(obj)
    return objs
