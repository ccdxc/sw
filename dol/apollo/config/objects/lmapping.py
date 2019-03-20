#! /usr/bin/python3
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.utils as utils
import mapping_pb2 as mapping_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class LocalMappingObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, ipversion):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF MAPPING OBJECT #####################
        self.MappingId = next(resmgr.LocalMappingIdAllocator)
        self.GID('LocalMapping%d'%self.MappingId)
        self.VNIC = parent
        if ipversion == utils.IP_VERSION_6:
            self.IPAddr = parent.SUBNET.AllocIPv6Address();
            self.AddrFamily = 'IPV6'
        else:
            self.IPAddr = parent.SUBNET.AllocIPv4Address();
            self.AddrFamily = 'IPV4'
        self.Label = 'NETWORKING'
        self.FlType = "MAPPING"
        self.IP = str(self.IPAddr) # For testspec
        ################# PRIVATE ATTRIBUTES OF MAPPING OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "LocalMappingID:%d|VnicId:%d|SubnetId:%d|PCNId:%d" %\
               (self.MappingId, self.VNIC.VnicId, self.VNIC.SUBNET.SubnetId, self.VNIC.SUBNET.PCN.PCNId)

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.flow.filters)

    def GetGrpcCreateMessage(self):
        grpcmsg = mapping_pb2.MappingRequest()
        spec = grpcmsg.Request.add()
        spec.Id.PCNId = self.VNIC.SUBNET.PCN.PCNId
        utils.GetRpcIPAddr(self.IPAddr, spec.Id.IPAddr)
        spec.SubnetId = self.VNIC.SUBNET.SubnetId
        spec.VnicId = self.VNIC.VnicId
        spec.TunnelId = int(Store.GetDevice().IPAddr)
        spec.MACAddr = self.VNIC.MACAddr.getnum()
        spec.Encap.type = self.VNIC.Encap
        spec.Encap.value.MPLSTag  = self.VNIC.MplsSlot
        spec.PublicIP.Af = types_pb2.IP_AF_NONE # TODO Public IP support
        return grpcmsg

    def Show(self):
        logger.info("LocalMapping Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- IPAddr:%s" % str(self.IPAddr))
        return

    def SetupTestcaseConfig(self, obj):
        return

class LocalMappingObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, parent, vnic_spec_obj):
        stack = parent.SUBNET.PCN.Stack
        for c in range (0, vnic_spec_obj.ipcount):
            if stack == "dual" or stack == 'ipv6':
                obj = LocalMappingObject(parent, vnic_spec_obj, utils.IP_VERSION_6)
                self.__objs.append(obj)
            if stack == "dual" or stack == 'ipv4':
                obj = LocalMappingObject(parent, vnic_spec_obj, utils.IP_VERSION_4)
                self.__objs.append(obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.MAPPING, msgs)
        return

client = LocalMappingObjectClient()

def GetMatchingObjects(selectors):
    objs = []
    for obj in client.Objects():
        if obj.IsFilterMatch(selectors):
            objs.append(obj)
    return objs
