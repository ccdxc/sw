#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.subnet as subnet

import pcn_pb2 as pcn_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class PcnObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF PCN OBJECT #####################
        self.PCNId = next(resmgr.PcnIdAllocator)
        self.VlanId = next(resmgr.PcnVlanIdAllocator)
        self.GID('Pcn%d'%self.PCNId)
        self.Type = pcn_pb2.PCN_TYPE_TENANT
        self.Prefix = resmgr.GetPcnIpv4Prefix(self.PCNId)
        
        ################# PRIVATE ATTRIBUTES OF PCN OBJECT #####################
        # All subnets are of /24 Prefix length
        self.__subnet_prefix_pool = resmgr.CreateIPv4SubnetPool(self.Prefix, 24)

        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate Subnet configuration
        subnet.client.GenerateObjects(self, spec)
        return

    def AllocSubnetPrefix(self):
        return next(self.__subnet_prefix_pool)

    def __repr__(self):
        return "PcnID:%d/Type:%d/Prefix:%s" %\
               (self.PCNId, self.Type, str(self.Prefix))

    def GetGrpcCreateMessage(self):
        grpcmsg = pcn_pb2.PCNRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.PCNId
        spec.Type = self.Type

        spec.Prefix.Len = self.Prefix.prefixlen
        spec.Prefix.Addr.Af = types_pb2.IP_AF_INET
        spec.Prefix.Addr.V4Addr = int(self.Prefix.network_address)
        return grpcmsg

    def Show(self):
        logger.info("PCN object:", self)
        logger.info("- %s" % repr(self))
        return

class PcnObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, topospec):
        for p in topospec.pcn:
            for c in range(p.count):
                obj = PcnObject(p)
                self.__objs.append(obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.PCN, msgs)

        # Create Subnet Objects
        subnet.client.CreateObjects()
        return

client = PcnObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()

