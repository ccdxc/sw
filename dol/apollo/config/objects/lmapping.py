#! /usr/bin/python3
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import mapping_pb2 as mapping_pb2
import service_pb2 as service_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class LocalMappingObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, ipversion, count, stack):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF MAPPING OBJECT #####################
        self.MappingId = next(resmgr.LocalMappingIdAllocator)
        self.GID('LocalMapping%d'%self.MappingId)
        self.VNIC = parent
        self.PublicIPAddr = None
        self.SourceGuard = parent.SourceGuard
        if ipversion == utils.IP_VERSION_6:
            self.AddrFamily = 'IPV6'
            self.IPAddr = parent.SUBNET.AllocIPv6Address();
            if (hasattr(spec, 'public')):
                self.PublicIPAddr = next(resmgr.PublicIpv6AddressAllocator)
            self.HasDefaultRoute = parent.SUBNET.V6RouteTable.HasDefaultRoute # For testspec
            self.SvcIPAddr, self.SvcPort = Store.GetSvcMapping(utils.IP_VERSION_6)
        else:
            self.AddrFamily = 'IPV4'
            self.IPAddr = parent.SUBNET.AllocIPv4Address();
            if (hasattr(spec, 'public')):
                self.PublicIPAddr = next(resmgr.PublicIpAddressAllocator)
            self.HasDefaultRoute = parent.SUBNET.V4RouteTable.HasDefaultRoute # For testspec
            self.SvcIPAddr, self.SvcPort = Store.GetSvcMapping(utils.IP_VERSION_4)
        self.Label = 'NETWORKING'
        self.FlType = "MAPPING"
        self.IP = str(self.IPAddr) # for testspec
        # Provider IP can be v4 or v6
        self.ProviderIPAddr, self.TunFamily = Store.GetProviderIPAddr(count)
        self.ProviderIP = str(self.ProviderIPAddr) # for testspec
        if self.PublicIPAddr is not None:
            self.PublicIP = str(self.PublicIPAddr) # for testspec
        self.SvcIP = str(self.SvcIPAddr) # for testspec
        # We will differentiate the traffic by port (vnet vs internet) and
        # different rules will be applied
        self.AppPort = resmgr.TransportSrcPort
        self.LBPort = resmgr.TransportSrcLBPort
        self.SubstrateVPCId = Store.GetSubstrateVPCId()

        ################# PRIVATE ATTRIBUTES OF MAPPING OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "LocalMappingID:%d|VnicId:%d|SubnetId:%d|VPCId:%d" %\
               (self.MappingId, self.VNIC.VnicId, self.VNIC.SUBNET.SubnetId, self.VNIC.SUBNET.VPC.VPCId)

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.flow.filters)

    def GetGrpcCreateMessage(self):
        grpcmsg = mapping_pb2.MappingRequest()
        spec = grpcmsg.Request.add()
        spec.Id.VPCId = self.VNIC.SUBNET.VPC.VPCId
        utils.GetRpcIPAddr(self.IPAddr, spec.Id.IPAddr)
        spec.SubnetId = self.VNIC.SUBNET.SubnetId
        spec.VnicId = self.VNIC.VnicId
        spec.MACAddr = self.VNIC.MACAddr.getnum()
        utils.GetRpcEncap(self.VNIC.MplsSlot, self.VNIC.Vnid, spec.Encap)
        spec.PublicIP.Af = types_pb2.IP_AF_NONE
        if self.PublicIPAddr is not None:
            utils.GetRpcIPAddr(self.PublicIPAddr, spec.PublicIP)
        if utils.IsPipelineArtemis():
            utils.GetRpcIPAddr(self.ProviderIPAddr, spec.ProviderIp)
        return grpcmsg

    def GetGrpcReadMessage(self):
        grpcmsg = mapping_pb2.MappingGetRequest()
        key = grpcmsg.Id.add()
        key.VPCId = self.VNIC.SUBNET.VPC.VPCId
        utils.GetRpcIPAddr(self.IPAddr, key.IPAddr)
        return grpcmsg

    def GetGrpcSvcMappingCreateMessage(self):
        grpcmsg = service_pb2.SvcMappingRequest()
        spec = grpcmsg.Request.add()
        spec.Key.VPCId = self.SubstrateVPCId
        utils.GetRpcIPAddr(self.SvcIPAddr, spec.Key.IPAddr)
        spec.Key.SvcPort = self.SvcPort
        spec.VPCId = self.VNIC.SUBNET.VPC.VPCId
        utils.GetRpcIPAddr(self.IPAddr, spec.PrivateIP)
        utils.GetRpcIPAddr(self.ProviderIPAddr, spec.ProviderIP)
        spec.Port = self.LBPort
        return grpcmsg

    def GetGrpcSvcMappingReadMessage(self):
        grpcmsg = service_pb2.SvcMappingRequest()
        key = grpcmsg.Id.add()
        key.VPCId = self.VNIC.SUBNET.VPC.VPCId
        utils.GetRpcIPAddr(self.SvcIPAddr, key.IPAddr)
        return grpcmsg

    def Show(self):
        logger.info("LocalMapping Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- IPAddr:%s|PublicIP:%s|PIP:%s|VIP:%s" \
            %(str(self.IPAddr), str(self.PublicIPAddr), str(self.ProviderIPAddr), str(self.SvcIPAddr)))
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
        stack = parent.SUBNET.VPC.Stack
        c = 0
        v6c = 0
        v4c = 0
        while c < vnic_spec_obj.ipcount:
            if stack == "dual" or stack == 'ipv6':
                obj = LocalMappingObject(parent, vnic_spec_obj, utils.IP_VERSION_6, v6c, stack)
                self.__objs.append(obj)
                c = c + 1
                v6c = v6c + 1
            if c < vnic_spec_obj.ipcount and (stack == "dual" or stack == 'ipv4'):
                obj = LocalMappingObject(parent, vnic_spec_obj, utils.IP_VERSION_4, v4c, stack)
                self.__objs.append(obj)
                c = c + 1
                v4c = v4c + 1
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.MAPPING, msgs)

        if utils.IsPipelineArtemis():
            msgs = list(map(lambda x: x.GetGrpcSvcMappingCreateMessage(), self.__objs))
            api.client.Create(api.ObjectTypes.SVCMAPPING, msgs)
        return

    def ReadObjects(self):
        msgs = list(map(lambda x: x.GetGrpcReadMessage(), self.__objs))
        api.client.Get(api.ObjectTypes.MAPPING, msgs)

        if utils.IsPipelineArtemis():
            msgs = list(map(lambda x: x.GetGrpcSvcMappingReadMessage(), self.__objs))
            api.client.Get(api.ObjectTypes.SVCMAPPING, msgs)
        return

client = LocalMappingObjectClient()

def GetMatchingObjects(selectors):
    objs = []
    for obj in client.Objects():
        if obj.IsFilterMatch(selectors):
            objs.append(obj)
    return objs
