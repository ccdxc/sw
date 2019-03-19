#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.subnet as subnet
import apollo.config.objects.route as route

import pcn_pb2 as pcn_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class PcnObject(base.ConfigObjectBase):
    def __init__(self, spec, ipversion):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF PCN OBJECT #####################
        self.PCNId = next(resmgr.PcnIdAllocator)
        self.GID('Pcn%d'%self.PCNId)
        self.Type = pcn_pb2.PCN_TYPE_TENANT
        if ipversion == 6:
            self.Prefix = resmgr.GetPcnIpv6Prefix(self.PCNId)
        else:
            self.Prefix = resmgr.GetPcnIpv4Prefix(self.PCNId)

        ################# PRIVATE ATTRIBUTES OF PCN OBJECT #####################
        # All subnets are of /24 Prefix length for IPV4 and 64 for IPV6
        if ipversion == 6:
            self.__subnet_prefix_pool = resmgr.CreateIPv6SubnetPool(self.Prefix, 64)
        else:
            self.__subnet_prefix_pool = resmgr.CreateIPv4SubnetPool(self.Prefix, 24)

        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate Route configuration. This should be before subnet
        route.client.GenerateObjects(self, spec)

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
        if self.Prefix.version == 6:
            spec.Prefix.Addr.Af = types_pb2.IP_AF_INET6
            spec.Prefix.Addr.V6Addr = self.Prefix.network_address.packed
        else:
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
            if getattr(p, 'v4count', None) != None:
                for c in range(p.v4count):
                    obj = PcnObject(p, 4)
                    self.__objs.append(obj)
            if getattr(p, 'v6count', None) != None:
                for c in range(p.v6count):
                    obj = PcnObject(p, 6)
                    self.__objs.append(obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.PCN, msgs)

        # Create Route object. This should be before subnet
        route.client.CreateObjects()

        # Create Subnet Objects
        subnet.client.CreateObjects()
        return

client = PcnObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()

