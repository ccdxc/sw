#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
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
        self.V4RouteTableId = next(resmgr.RouteTableIdAllocator)
        self.V6RouteTableId = next(resmgr.RouteTableIdAllocator)
        self.IngV4SecurityPolicyId = next(resmgr.SecurityPolicyIdAllocator)
        self.IngV6SecurityPolicyId = next(resmgr.SecurityPolicyIdAllocator)
        self.EgV4SecurityPolicyId = next(resmgr.SecurityPolicyIdAllocator)
        self.EgV6SecurityPolicyId = next(resmgr.SecurityPolicyIdAllocator)
        
        ################# PRIVATE ATTRIBUTES OF SUBNET OBJECT #####################
        self.__ipaddress_pool = resmgr.CreateIpv4AddrPool(self.Prefix)

        self.__set_vrouter_attributes()

        self.Show()
        return

    def __set_vrouter_attributes(self):
        # 1st IP address of the subnet becomes the vrouter.
        self.VirtualRouterIP = next(self.__ipaddress_pool)
        self.VirtualRouterMac = resmgr.VirtualRouterMacAllocator.get()
        return

    def __repr__(self):
        return "SubnetID:%d/PCNId:%d/Prefix:%s" %\
               (self.SubnetId, self.PCN.PCNId, str(self.Prefix))

    def GetGrpcCreateMessage(self):
        grpcmsg = subnet_pb2.SubnetSpec()
        grpcmsg.Id = self.SubnetId
        grpcmsg.PCNId = self.PCN.PCNId
        grpcmsg.Prefix.Len = self.Prefix.prefixlen
        grpcmsg.Prefix.Addr.Af = types_pb2.IP_AF_INET
        grpcmsg.Prefix.Addr.V4Addr = int(self.Prefix.network_address)
        grpcmsg.VirtualRouterIP.Af = types_pb2.IP_AF_INET
        grpcmsg.VirtualRouterIP.V4Addr = int(self.VirtualRouterIP)
        grpcmsg.VirtualRouterMac = self.VirtualRouterMac.getnum()
        grpcmsg.V4RouteTableId = self.V4RouteTableId
        grpcmsg.V6RouteTableId = self.V6RouteTableId
        grpcmsg.IngV4SecurityPolicyId = self.IngV4SecurityPolicyId
        grpcmsg.IngV6SecurityPolicyId = self.IngV6SecurityPolicyId
        grpcmsg.EgV4SecurityPolicyId = self.EgV4SecurityPolicyId
        grpcmsg.EgV6SecurityPolicyId = self.EgV6SecurityPolicyId
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
        obj.root = self
        obj.pcn = self.PCN
        # TODO: This is temp code, until the endpoint objects are created.
        obj.sip = str(next(self.__ipaddress_pool))
        obj.dip = str(next(self.__ipaddress_pool))
        # TODO: Link these two below with port objects
        obj.hostport = 1
        obj.switchport = 2
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
        return

client = SubnetObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
