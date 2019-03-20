#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.subnet as subnet
import apollo.config.objects.route as route
import apollo.config.objects.utils as utils

import pcn_pb2 as pcn_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class PcnObject(base.ConfigObjectBase):
    def __init__(self, spec, index):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF PCN OBJECT #####################
        self.PCNId = next(resmgr.PcnIdAllocator)
        self.GID('Pcn%d'%self.PCNId)
        self.Type = pcn_pb2.PCN_TYPE_TENANT
        self.IPPrefix = {}
        self.IPPrefix[0] = resmgr.GetPcnIPv6Prefix(self.PCNId)
        self.IPPrefix[1] = resmgr.GetPcnIPv4Prefix(self.PCNId)
        self.Stack = spec.stack
        # As currently vcn can have only type IPV4 or IPV6, we will alternate 
        # the configuration
        if self.Stack == 'dual':
            self.PfxSel = index % 2
        elif obj.Stack == 'ipv4':
            self.PfxSel = 1
        else:
            self.PfxSel = 0

        ################# PRIVATE ATTRIBUTES OF PCN OBJECT #####################
        self.__ip_subnet_prefix_pool = {}
        self.__ip_subnet_prefix_pool[0] = {}
        self.__ip_subnet_prefix_pool[1] = {}

        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate Route configuration. This should be before subnet
        route.client.GenerateObjects(self, spec)

        # Generate Subnet configuration
        subnet.client.GenerateObjects(self, spec)
        return

    def InitSubnetPefixPools(self, poolid, v6pfxlen, v4pfxlen):
        self.__ip_subnet_prefix_pool[0][poolid] =  resmgr.CreateIPv6SubnetPool(self.IPPrefix[0], v6pfxlen, poolid)
        self.__ip_subnet_prefix_pool[1][poolid] =  resmgr.CreateIPv4SubnetPool(self.IPPrefix[1], v4pfxlen, poolid)

    def AllocIPv6SubnetPrefix(self, poolid):
        return next(self.__ip_subnet_prefix_pool[0][poolid])

    def AllocIPv4SubnetPrefix(self, poolid):
        return next(self.__ip_subnet_prefix_pool[1][poolid])

    def __repr__(self):
        return "PcnID:%d|Type:%d|PfxSel:%d" %\
               (self.PCNId, self.Type, self.PfxSel)

    def GetGrpcCreateMessage(self):
        grpcmsg = pcn_pb2.PCNRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.PCNId
        spec.Type = self.Type
        utils.GetRpcIPPrefix(self.IPPrefix[self.PfxSel], spec.Prefix)
        return grpcmsg

    def Show(self):
        logger.info("PCN Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Prefix:%s" %self.IPPrefix)
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
                obj = PcnObject(p, c)
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

