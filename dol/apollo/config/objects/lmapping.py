#! /usr/bin/python3
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import mapping_pb2 as mapping_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class LocalMappingObject(base.ConfigObjectBase):
    def __init__(self, parent, spec):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF MAPPING OBJECT #####################
        self.MappingId = next(resmgr.LocalMappingIdAllocator)
        self.GID('LocalMapping%d'%self.MappingId)
        self.VNIC = parent
        self.IPAddr = parent.SUBNET.AllocIPAddress();
        if self.IPAddr.version == 4:
            self.af = 'IPV4'
        else:
            self.af = 'IPV6'
        self.label = 'NETWORKING'
        ################# PRIVATE ATTRIBUTES OF MAPPING OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "LocalMappingID:%d/VnicId:%d/SubnetId:%d" %\
               (self.MappingId, self.VNIC.VnicId, self.VNIC.SUBNET.SubnetId)

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.mapping.filters)

    def GetGrpcCreateMessage(self):
        grpcmsg = mapping_pb2.MappingRequest()
        spec = grpcmsg.Request.add()
        spec.Id.PCNId = self.VNIC.SUBNET.PCN.PCNId
        if self.IPAddr.version == 4:
            spec.Id.IPAddr.V4Addr = int(self.IPAddr)
            spec.Id.IPAddr.Af = types_pb2.IP_AF_INET
        else:
            spec.Id.IPAddr.V6Addr = self.IPAddr.packed
            spec.Id.IPAddr.Af = types_pb2.IP_AF_INET6
        spec.SubnetId = self.VNIC.SUBNET.SubnetId
        spec.VnicId = self.VNIC.VnicId
        spec.TunnelId = int(Store.GetSwitch().GetLocalIP())
        spec.MACAddr = self.VNIC.MacAddr.getnum()
        spec.Encap.type = self.VNIC.Encap
        spec.Encap.value.MPLSTag  = self.VNIC.MplsSlot
        spec.PublicIP.Af = types_pb2.IP_AF_NONE # TODO Public IP support
        return grpcmsg

    def Show(self):
        logger.info("LocalMapping object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- iP address %s" % str(self.IPAddr))
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
        for c in range (0, vnic_spec_obj.ipcount):
            obj = LocalMappingObject(parent, vnic_spec_obj)
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
