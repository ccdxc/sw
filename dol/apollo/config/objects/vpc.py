#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.policy as policy
import apollo.config.objects.route as route
import apollo.config.objects.subnet as subnet
import apollo.config.objects.nexthop as nexthop
import apollo.config.objects.tag as tag
import artemis.config.objects.cfgjson as cfgjson
import apollo.config.utils as utils

import vpc_pb2 as vpc_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class VpcObject(base.ConfigObjectBase):
    def __init__(self, spec, index, maxcount):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF VPC OBJECT #####################
        self.VPCId = next(resmgr.VpcIdAllocator)
        self.GID('Vpc%d'%self.VPCId)
        self.IPPrefix = {}
        if spec.type == 'substrate':
            self.SvcMappingIPAddr = {}
            self.Type = vpc_pb2.VPC_TYPE_SUBSTRATE
            self.IPPrefix[0] = resmgr.ProviderIpV6Network
            self.IPPrefix[1] = resmgr.ProviderIpV4Network
            # Reserve one SVC endpoint
            self.SvcMappingIPAddr[0] = next(resmgr.SvcMappingPublicIpV6AddressAllocator)
            self.SvcMappingIPAddr[1] = next(resmgr.SvcMappingPublicIpV4AddressAllocator)
            self.SvcPort = resmgr.TransportSvcPort
        else:
            self.Type = vpc_pb2.VPC_TYPE_TENANT
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
        self.Vnid = 0
        if utils.IsPipelineArtemis() and Store.IsDeviceEncapTypeVXLAN() :
            self.Vnid = next(resmgr.VpcVxlanIdAllocator)
        ################# PRIVATE ATTRIBUTES OF VPC OBJECT #####################
        self.__ip_subnet_prefix_pool = {}
        self.__ip_subnet_prefix_pool[0] = {}
        self.__ip_subnet_prefix_pool[1] = {}

        self.Show()

        ############### CHILDREN OBJECT GENERATION

        if not utils.IsPipelineArtemis() and self.Type == vpc_pb2.VPC_TYPE_SUBSTRATE:
            # Nothing to be done for substrate vpc
            return

        # Generate NextHop configuration
        if getattr(spec, 'nexthop', None) != None:
            nexthop.client.GenerateObjects(self, spec)

        # Generate Tag configuration.
        if getattr(spec, 'tagtbl', None) != None:
            tag.client.GenerateObjects(self, spec)

        # Generate Policy configuration.
        if getattr(spec, 'policy', None) != None:
            policy.client.GenerateObjects(self, spec)

        # Generate Route configuration.
        if getattr(spec, 'routetbl', None) != None:
            # find peer vpcid
            if (index + 1) == maxcount:
                vpc_peerid = self.VPCId - maxcount + 1
            else:
                vpc_peerid = self.VPCId + 1
            route.client.GenerateObjects(self, spec, vpc_peerid)

        # Generate Subnet configuration post policy & route
        if getattr(spec, 'subnet', None) != None:
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

    def GetProviderIPAddr(self, count):
        assert self.Type == vpc_pb2.VPC_TYPE_SUBSTRATE
        if self.Stack == 'dual':
            paf = utils.IP_VERSION_6 if count % 2 == 0 else utils.IP_VERSION_4
        else:
            paf = utils.IP_VERSION_6 if stack == 'ipv6' else utils.IP_VERSION_4
        if paf == utils.IP_VERSION_6:
            return next(resmgr.ProviderIpV6AddressAllocator), 'IPV6'
        else:
            return next(resmgr.ProviderIpV4AddressAllocator), 'IPV4'

    def GetSvcMapping(self, ipversion):
        assert self.Type == vpc_pb2.VPC_TYPE_SUBSTRATE
        if ipversion ==  utils.IP_VERSION_6:
            return self.SvcMappingIPAddr[0],self.SvcPort
        else:
            return self.SvcMappingIPAddr[1],self.SvcPort

    def GetGrpcCreateMessage(self):
        grpcmsg = vpc_pb2.VPCRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.VPCId
        spec.Type = self.Type
        utils.GetRpcIPPrefix(self.IPPrefix[1], spec.V4Prefix)
        utils.GetRpcIPPrefix(self.IPPrefix[0], spec.V6Prefix)
        if self.Vnid:
            utils.GetRpcEncap(self.Vnid, self.Vnid, spec.FabricEncap)
        return grpcmsg

    def Show(self):
        logger.info("VPC Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Prefix:%s" %self.IPPrefix)
        return

    def IsSubstrateVPC(self):
        if self.Type == vpc_pb2.VPC_TYPE_SUBSTRATE:
            return True
        return False

class VpcObjectClient:
    def __init__(self):
        self.__objs = dict()
        self.__substrate_vpcid = -1
        return

    def Objects(self):
        return self.__objs.values()

    def IsValidConfig(self):
        count = len(self.__objs.values())
        if  count > resmgr.MAX_VPC:
            return False, "VPC count %d exceeds allowed limit of %d" %\
                          (count, resmgr.MAX_VPC)
        return True, ""

    def GetVpcObject(self, vpcid):
        return self.__objs.get(vpcid, None)

    def __write_cfg(self, vpc_count):
        nh = nexthop.client.GetNumNextHopPerVPC()
        cfgjson.CfgJsonHelper.SetNumNexthopPerVPC(nh)
        cfgjson.CfgJsonHelper.SetVPCCount(vpc_count)
        cfgjson.CfgJsonHelper.WriteConfig()

    def GenerateObjects(self, topospec):
        for p in topospec.vpc:
            for c in range(p.count):
                obj = VpcObject(p, c, p.count)
                self.__objs.update({obj.VPCId: obj})
                if obj.IsSubstrateVPC():
                    Store.SetSubstrateVPC(obj)
        # Write the flow and nexthop config to agent hook file
        if utils.IsPipelineArtemis():
            self.__write_cfg(p.count)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs.values()))
        api.client.Create(api.ObjectTypes.VPC, msgs)

        # Create Nexthop object
        nexthop.client.CreateObjects()

        # Create Tag object.
        tag.client.CreateObjects()

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

