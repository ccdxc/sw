#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from apollo.config.store import Store

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.base as base
import apollo.config.objects.policy as policy
import apollo.config.objects.route as route
import apollo.config.objects.subnet as subnet
from apollo.config.objects.nexthop import client as NhClient
from apollo.config.objects.nexthop_group import client as NhGroupClient
import apollo.config.objects.nexthop_group as nexthop_group
import apollo.config.objects.tag as tag
import apollo.config.objects.meter as meter
from apollo.config.objects.vnic import client as VnicClient
import artemis.config.objects.cfgjson as cfgjson
import apollo.config.utils as utils

import vpc_pb2 as vpc_pb2

class VpcObject(base.ConfigObjectBase):
    def __init__(self, spec, index, maxcount):
        super().__init__()
        self.SetBaseClassAttr()

        ################# PUBLIC ATTRIBUTES OF VPC OBJECT #####################
        self.VPCId = next(resmgr.VpcIdAllocator)
        self.GID('Vpc%d'%self.VPCId)
        self.IPPrefix = {}
        self.Nat46_pfx = None
        if spec.type == 'underlay':
            self.Type = vpc_pb2.VPC_TYPE_UNDERLAY
            self.IPPrefix[0] = resmgr.ProviderIpV6Network
            self.IPPrefix[1] = resmgr.ProviderIpV4Network
            # Reserve one SVC port
            # Right now it does not support multiple backends for a frontend
            self.SvcPort = resmgr.TransportSvcPort
            self.__max_svc_mapping_shared_count = 1
            self.__svc_mapping_shared_count = 0
            self.SvcMappingIPAddr  = {}
        else:
            self.Type = vpc_pb2.VPC_TYPE_TENANT
            self.IPPrefix[0] = resmgr.GetVpcIPv6Prefix(self.VPCId)
            self.IPPrefix[1] = resmgr.GetVpcIPv4Prefix(self.VPCId)
        if (hasattr(spec, 'nat46')) and spec.nat46 is True:
            self.Nat46_pfx = resmgr.Nat46Address
        self.Stack = spec.stack
        # As currently vpc can have only type IPV4 or IPV6, we will alternate
        # the configuration
        if self.Stack == 'dual':
            self.PfxSel = index % 2
        elif self.Stack == 'ipv4':
            self.PfxSel = 1
        else:
            self.PfxSel = 0
        self.Vnid = next(resmgr.VpcVxlanIdAllocator)
        self.VirtualRouterMACAddr = resmgr.VirtualRouterMacAllocator.get()
        ################# PRIVATE ATTRIBUTES OF VPC OBJECT #####################
        self.__ip_subnet_prefix_pool = {}
        self.__ip_subnet_prefix_pool[0] = {}
        self.__ip_subnet_prefix_pool[1] = {}

        self.Show()

        ############### CHILDREN OBJECT GENERATION

        if not utils.IsPipelineArtemis() and self.Type == vpc_pb2.VPC_TYPE_UNDERLAY:
            # Nothing to be done for underlay vpc
            return

        # Generate NextHop configuration
        NhClient.GenerateObjects(self, spec)

        # Generate NextHop configuration
        NhGroupClient.GenerateObjects(self, spec)

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

        # Generate Meter configuration
        meter.client.GenerateObjects(self, spec)

        # Generate Subnet configuration post policy & route
        if getattr(spec, 'subnet', None) != None:
            subnet.client.GenerateObjects(self, spec)

        return

    def SetBaseClassAttr(self):
        self.ObjType = api.ObjectTypes.VPC
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
        assert self.Type == vpc_pb2.VPC_TYPE_UNDERLAY
        if self.Stack == 'dual':
            paf = utils.IP_VERSION_6 if count % 2 == 0 else utils.IP_VERSION_4
        else:
            paf = utils.IP_VERSION_6 if stack == 'ipv6' else utils.IP_VERSION_4
        if paf == utils.IP_VERSION_6:
            return next(resmgr.ProviderIpV6AddressAllocator), 'IPV6'
        else:
            return next(resmgr.ProviderIpV4AddressAllocator), 'IPV4'

    def GetSvcMapping(self, ipversion):
        assert self.Type == vpc_pb2.VPC_TYPE_UNDERLAY

        def __alloc():
            self.SvcMappingIPAddr[0] = next(resmgr.SvcMappingPublicIpV6AddressAllocator)
            self.SvcMappingIPAddr[1] = next(resmgr.SvcMappingPublicIpV4AddressAllocator)

        def __get():
            if ipversion ==  utils.IP_VERSION_6:
                return self.SvcMappingIPAddr[0],self.SvcPort
            else:
                return self.SvcMappingIPAddr[1],self.SvcPort

        if self.__svc_mapping_shared_count == 0:
            __alloc()
            self.__svc_mapping_shared_count = (self.__svc_mapping_shared_count + 1) % self.__max_svc_mapping_shared_count
        return __get()

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.VPCId)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.VPCId
        spec.Type = self.Type
        spec.VirtualRouterMac = self.VirtualRouterMACAddr.getnum()
        utils.GetRpcEncap(self.Vnid, self.Vnid, spec.FabricEncap)
        if self.Nat46_pfx is not None:
            utils.GetRpcIPv6Prefix(self.Nat46_pfx, spec.Nat46Prefix)
        return

    def Show(self):
        logger.info("VPC Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Prefix:%s" %self.IPPrefix)
        return

    def IsUnderlayVPC(self):
        if self.Type == vpc_pb2.VPC_TYPE_UNDERLAY:
            return True
        return False

class VpcObjectClient:
    def __init__(self):
        self.__objs = dict()
        self.__underlay_vpcid = -1
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
        nh = NhClient.GetNumNextHopPerVPC()
        mtr = meter.client.GetNumMeterPerVPC()
        cfgjson.CfgJsonHelper.SetNumNexthopPerVPC(nh)
        cfgjson.CfgJsonHelper.SetNumMeterPerVPC(mtr[0], mtr[1])
        cfgjson.CfgJsonHelper.SetVPCCount(vpc_count)
        cfgjson.CfgJsonHelper.WriteConfig()

    def GenerateObjects(self, topospec):
        vpc_count = 0
        for p in topospec.vpc:
            vpc_count += p.count
            for c in range(p.count):
                if hasattr(p, "nat46"):
                    if p.nat46 is True and not utils.IsPipelineArtemis():
                        continue
                obj = VpcObject(p, c, p.count)
                self.__objs.update({obj.VPCId: obj})
                if obj.IsUnderlayVPC():
                    Store.SetUnderlayVPC(obj)
        # Write the flow and nexthop config to agent hook file
        if utils.IsFlowInstallationNeeded():
            self.__write_cfg(vpc_count)
        if utils.IsPipelineApulu():
            # Associate Nexthop objects
            NhGroupClient.CreateAllocator()
            NhClient.AssociateObjects()
            VnicClient.AssociateObjects()
        return

    def CreateObjects(self):
        cookie = utils.GetBatchCookie()
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__objs.values()))
        api.client.Create(api.ObjectTypes.VPC, msgs)

        # Create Nexthop object
        NhClient.CreateObjects()

        # Create Tag object.
        tag.client.CreateObjects()

        # Create Policy object.
        policy.client.CreateObjects()

        # Create Route object.
        route.client.CreateObjects()

        # Create Meter Objects
        meter.client.CreateObjects()

        # Create Subnet Objects after policy & route
        subnet.client.CreateObjects()
        return

    def GetGrpcReadAllMessage(self):
        grpcmsg = vpc_pb2.VPCGetRequest()
        return grpcmsg

    def ReadObjects(self):
        msg = self.GetGrpcReadAllMessage()
        api.client.Get(api.ObjectTypes.VPC, [msg])
        return

client = VpcObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()

