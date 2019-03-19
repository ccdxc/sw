#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.vnic as vnic
import apollo.config.objects.rmapping as rmapping
import apollo.config.objects.route as route
import subnet_pb2 as subnet_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class SubnetObject(base.ConfigObjectBase):
    def __init__(self, parent, spec):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF SUBNET OBJECT #####################
        self.SubnetId = next(resmgr.SubnetIdAllocator)
        self.GID('Subnet%d'%self.SubnetId)
        self.PCN = parent
        self.Prefix = parent.AllocSubnetPrefix()
        self.VirtualRouterIP = None
        self.VirtualRouterMac = None
        self.V4RouteTableId = route.client.GetRouteV4TableId(parent.PCNId)
        self.V6RouteTableId = route.client.GetRouteV6TableId(parent.PCNId)
        self.IngV4SecurityPolicyId = next(resmgr.SecurityPolicyIdAllocator)
        self.IngV6SecurityPolicyId = next(resmgr.SecurityPolicyIdAllocator)
        self.EgV4SecurityPolicyId = next(resmgr.SecurityPolicyIdAllocator)
        self.EgV6SecurityPolicyId = next(resmgr.SecurityPolicyIdAllocator)
        
        ################# PRIVATE ATTRIBUTES OF SUBNET OBJECT #####################
        if self.Prefix.version == 6:
            self.__ipaddress_pool = resmgr.CreateIpv6AddrPool(self.Prefix)
        else:
            self.__ipaddress_pool = resmgr.CreateIpv4AddrPool(self.Prefix)

        self.__set_vrouter_attributes()

        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate VNIC and Remote Mapping configuration
        vnic.client.GenerateObjects(self, spec)
        rmapping.client.GenerateObjects(self, spec)

        return

    def __set_vrouter_attributes(self):
        # 1st IP address of the subnet becomes the vrouter.
        self.VirtualRouterIP = next(self.__ipaddress_pool)
        self.VirtualRouterMac = resmgr.VirtualRouterMacAllocator.get()
        return

    def AllocIPAddress(self):
        return next(self.__ipaddress_pool)

    def __repr__(self):
        return "SubnetID:%d/PCNId:%d/Prefix:%s" %\
               (self.SubnetId, self.PCN.PCNId, str(self.Prefix))

    def GetGrpcCreateMessage(self):
        grpcmsg = subnet_pb2.SubnetRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.SubnetId
        spec.PCNId = self.PCN.PCNId
        spec.Prefix.Len = self.Prefix.prefixlen
        if self.Prefix.version == 6:
            spec.Prefix.Addr.Af = types_pb2.IP_AF_INET6
            spec.Prefix.Addr.V6Addr = self.Prefix.network_address.packed
            spec.VirtualRouterIP.Af = types_pb2.IP_AF_INET6
            spec.VirtualRouterIP.V6Addr = self.VirtualRouterIP.packed
        else:
            spec.Prefix.Addr.Af = types_pb2.IP_AF_INET
            spec.Prefix.Addr.V4Addr = int(self.Prefix.network_address)
            spec.VirtualRouterIP.Af = types_pb2.IP_AF_INET
            spec.VirtualRouterIP.V4Addr = int(self.VirtualRouterIP)
        spec.VirtualRouterMac = self.VirtualRouterMac.getnum()
        spec.V4RouteTableId = self.V4RouteTableId
        spec.V6RouteTableId = self.V6RouteTableId
        spec.IngV4SecurityPolicyId = self.IngV4SecurityPolicyId
        spec.IngV6SecurityPolicyId = self.IngV6SecurityPolicyId
        spec.EgV4SecurityPolicyId = self.EgV4SecurityPolicyId
        spec.EgV6SecurityPolicyId = self.EgV6SecurityPolicyId
        return grpcmsg

    def Show(self):
        logger.info("SUBNET object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- VirtualRouter IP:%s/Mac:%s" % (self.VirtualRouterIP, self.VirtualRouterMac.get()))
        logger.info("- TableIds  V4:%d/V6:%d" % (self.V4RouteTableId, self.V6RouteTableId))
        logger.info("- SecurityPolicyIDs IngV4:%d/IngV6:%d/EgV4:%d/EgV6:%d" %\
                    (self.IngV4SecurityPolicyId, self.IngV6SecurityPolicyId, self.EgV4SecurityPolicyId, self.EgV6SecurityPolicyId))
        return

    def SetupTestcaseConfig(self, obj):
        return

class SubnetObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, parent, pcn_spec_obj):
        for subnet_spec_obj in pcn_spec_obj.subnet:
            for c in range(subnet_spec_obj.count):
                obj = SubnetObject(parent, subnet_spec_obj)
                self.__objs.append(obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.SUBNET, msgs)
        # Create VNIC and Remote Mapping Objects
        vnic.client.CreateObjects()
        rmapping.client.CreateObjects()
        return

client = SubnetObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
