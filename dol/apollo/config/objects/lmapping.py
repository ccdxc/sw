#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from apollo.config.store import Store

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.base as base
import apollo.config.utils as utils

import service_pb2 as service_pb2
import types_pb2 as types_pb2

class LocalMappingObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, ipversion, count):
        super().__init__()
        self.SetBaseClassAttr()

        ################# PUBLIC ATTRIBUTES OF MAPPING OBJECT #####################
        self.MappingId = next(resmgr.LocalMappingIdAllocator)
        self.GID('LocalMapping%d'%self.MappingId)
        self.VNIC = parent
        self.PublicIPAddr = None
        self.SourceGuard = parent.SourceGuard
        self.HasDefaultRoute = False
        if ipversion == utils.IP_VERSION_6:
            self.AddrFamily = 'IPV6'
            self.IPAddr = parent.SUBNET.AllocIPv6Address();
            if (hasattr(spec, 'public')):
                self.PublicIPAddr = next(resmgr.PublicIpv6AddressAllocator)
            if parent.SUBNET.V6RouteTable:
                self.HasDefaultRoute = parent.SUBNET.V6RouteTable.HasDefaultRoute
            self.SvcIPAddr, self.SvcPort = Store.GetSvcMapping(utils.IP_VERSION_6)
        else:
            self.AddrFamily = 'IPV4'
            self.IPAddr = parent.SUBNET.AllocIPv4Address();
            if (hasattr(spec, 'public')):
                self.PublicIPAddr = next(resmgr.PublicIpAddressAllocator)
            if parent.SUBNET.V4RouteTable:
                self.HasDefaultRoute = parent.SUBNET.V4RouteTable.HasDefaultRoute
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

    def Show(self):
        logger.info("LocalMapping Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- IPAddr:%s|PublicIP:%s|PIP:%s|VIP:%s" \
            %(str(self.IPAddr), str(self.PublicIPAddr), str(self.ProviderIPAddr), str(self.SvcIPAddr)))
        return

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.flow.filters)

    def SetBaseClassAttr(self):
        self.ObjType = api.ObjectTypes.MAPPING
        return

    def PopulateKey(self, grpcmsg):
        key = grpcmsg.Id.add()
        key.VPCId = self.VNIC.SUBNET.VPC.VPCId
        utils.GetRpcIPAddr(self.IPAddr, key.IPAddr)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id.IPKey.VPCId = self.VNIC.SUBNET.VPC.VPCId
        utils.GetRpcIPAddr(self.IPAddr, spec.Id.IPKey.IPAddr)
        spec.SubnetId = self.VNIC.SUBNET.SubnetId
        spec.VnicId = self.VNIC.VnicId
        spec.MACAddr = self.VNIC.MACAddr.getnum()
        utils.GetRpcEncap(self.VNIC.MplsSlot, self.VNIC.Vnid, spec.Encap)
        spec.PublicIP.Af = types_pb2.IP_AF_NONE
        if self.PublicIPAddr is not None:
            utils.GetRpcIPAddr(self.PublicIPAddr, spec.PublicIP)
        if utils.IsPipelineArtemis():
            utils.GetRpcIPAddr(self.ProviderIPAddr, spec.ProviderIp)
        return

    def GetGrpcSvcMappingCreateMessage(self, cookie):
        grpcmsg = service_pb2.SvcMappingRequest()
        grpcmsg.BatchCtxt.BatchCookie = cookie
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


class LocalMappingObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, parent, vnic_spec_obj):
        isV4Stack = utils.IsV4Stack(parent.SUBNET.VPC.Stack)
        isV6Stack = utils.IsV6Stack(parent.SUBNET.VPC.Stack)
        c = 0
        v6c = 0
        v4c = 0
        while c < vnic_spec_obj.ipcount:
            if isV6Stack:
                obj = LocalMappingObject(parent, vnic_spec_obj, utils.IP_VERSION_6, v6c)
                self.__objs.append(obj)
                c = c + 1
                v6c = v6c + 1
            if c < vnic_spec_obj.ipcount and isV4Stack:
                obj = LocalMappingObject(parent, vnic_spec_obj, utils.IP_VERSION_4, v4c)
                self.__objs.append(obj)
                c = c + 1
                v4c = v4c + 1
        return

    def CreateObjects(self):
        cookie = utils.GetBatchCookie()
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__objs))
        api.client.Create(api.ObjectTypes.MAPPING, msgs)

        if utils.IsPipelineArtemis():
            msgs = list(map(lambda x: x.GetGrpcSvcMappingCreateMessage(cookie), self.__objs))
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
