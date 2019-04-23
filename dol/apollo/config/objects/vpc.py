#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.policy as policy
import apollo.config.objects.route as route
import apollo.config.objects.subnet as subnet
import apollo.config.utils as utils

import vpc_pb2 as vpc_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class VpcObject(base.ConfigObjectBase):
    def __init__(self, spec, index):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF VPC OBJECT #####################
        self.VPCId = next(resmgr.VpcIdAllocator)
        self.GID('Vpc%d'%self.VPCId)
        self.Type = vpc_pb2.VPC_TYPE_TENANT
        self.IPPrefix = {}
        self.IPPrefix[0] = resmgr.GetVpcIPv6Prefix(self.VPCId)
        self.IPPrefix[1] = resmgr.GetVpcIPv4Prefix(self.VPCId)
        self.Stack = spec.stack
        # As currently vpc can have only type IPV4 or IPV6, we will alternate
        # the configuration
        if self.Stack == 'dual':
            self.PfxSel = index % 2
        elif self.Stack == 'ipv4':
            self.PfxSel = 1
        else:
            self.PfxSel = 0

        ################# PRIVATE ATTRIBUTES OF VPC OBJECT #####################
        self.__ip_subnet_prefix_pool = {}
        self.__ip_subnet_prefix_pool[0] = {}
        self.__ip_subnet_prefix_pool[1] = {}

        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate Policy configuration.
        policy.client.GenerateObjects(self, spec)

        # Generate Route configuration.
        route.client.GenerateObjects(self, spec)

        # Generate Subnet configuration post policy & route
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
        return "VpcID:%d|Type:%d|PfxSel:%d" %\
               (self.VPCId, self.Type, self.PfxSel)

    def GetGrpcCreateMessage(self):
        grpcmsg = vpc_pb2.VPCRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.VPCId
        spec.Type = self.Type
        utils.GetRpcIPPrefix(self.IPPrefix[1], spec.V4Prefix)
        utils.GetRpcIPPrefix(self.IPPrefix[0], spec.V6Prefix)
        return grpcmsg

    def Show(self):
        logger.info("VPC Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Prefix:%s" %self.IPPrefix)
        return

class VpcObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, topospec):
        for p in topospec.vpc:
            for c in range(p.count):
                obj = VpcObject(p, c)
                self.__objs.append(obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.VPC, msgs)

        # Create Policy object.
        policy.client.CreateObjects()

        # Create Route object.
        route.client.CreateObjects()

        # Create Subnet Objects after policy & route
        subnet.client.CreateObjects()
        return

client = VpcObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()

