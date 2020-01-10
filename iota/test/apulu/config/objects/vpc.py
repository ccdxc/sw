#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from iota.test.apulu.config.store import Store

import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.agent.api as api
import iota.test.apulu.config.objects.base as base
import iota.test.apulu.config.objects.policy as policy
import iota.test.apulu.config.objects.route as route
import iota.test.apulu.config.objects.subnet as subnet
import iota.test.apulu.config.objects.tunnel as tunnel
from iota.test.apulu.config.objects.nexthop import client as NhClient
from iota.test.apulu.config.objects.nexthop_group import client as NhGroupClient
from iota.test.apulu.config.objects.tunnel import client as TunnelClient
from iota.test.apulu.config.objects.interface import client as InterfaceClient
from iota.test.apulu.config.objects.port import client as PortClient
import iota.test.apulu.config.objects.nexthop_group as nexthop_group
import iota.test.apulu.config.objects.tag as tag
import iota.test.apulu.config.objects.meter as meter
from iota.test.apulu.config.objects.vnic import client as VnicClient
#import artemis.config.objects.cfgjson as cfgjson
import iota.test.apulu.config.utils as utils

import vpc_pb2 as vpc_pb2

class VpcStatus(base.StatusObjectBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.VPC)
        self.HwId = None
        return

    def Update(self, status):
        self.HwId = status.HwId
        return

class VpcObject(base.ConfigObjectBase):
    def __init__(self, node, spec, index, maxcount):
        super().__init__(api.ObjectTypes.VPC)
        self.Node = node
        ################# PUBLIC ATTRIBUTES OF VPC OBJECT #####################
        if (hasattr(spec, 'id')):
            self.VPCId = spec.id
        else:
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
        self.Status = VpcStatus()
        ################# PRIVATE ATTRIBUTES OF VPC OBJECT #####################
        self.__ip_subnet_prefix_pool = {}
        self.__ip_subnet_prefix_pool[0] = {}
        self.__ip_subnet_prefix_pool[1] = {}

        self.Show()

        ############### CHILDREN OBJECT GENERATION

        # Apulu pipeline also supports underlay vpc, hence following check is disabled
        #if not utils.IsPipelineArtemis() and self.Type == vpc_pb2.VPC_TYPE_UNDERLAY:
            # Nothing to be done for underlay vpc
            # return

        # Generate Port Configuration
        PortClient.GenerateObjects(node, self, spec)

        # Generate Interface Configuration
        InterfaceClient.GenerateObjects(node, self, spec)

        # Generate NextHop configuration
        NhClient.GenerateObjects(node, self, spec)

        # Generate NextHop configuration
        NhGroupClient.GenerateObjects(node, self, spec)

        # Generate NextHop configuration
        if hasattr(spec, "tunnel"):
            TunnelClient.GenerateObjects(node, self, spec.tunnel)

        # Generate Tag configuration.
        if getattr(spec, 'tagtbl', None) != None:
            tag.client.GenerateObjects(node, self, spec)

        # Generate Policy configuration.
        if getattr(spec, 'policy', None) != None:
            policy.client.GenerateObjects(node, self, spec)

        # Generate Route configuration.
        if getattr(spec, 'routetbl', None) != None:
            # find peer vpcid
            if (index + 1) == maxcount:
                vpc_peerid = self.VPCId - maxcount + 1
            else:
                vpc_peerid = self.VPCId + 1
            route.client.GenerateObjects(node, self, spec, vpc_peerid)

        # Generate Meter configuration
        meter.client.GenerateObjects(node, self, spec)

        # Generate Subnet configuration post policy & route
        if getattr(spec, 'subnet', None) != None:
            subnet.client.GenerateObjects(node, self, spec)

        return

    def __repr__(self):
        return "VpcID:%d|Type:%d|PfxSel:%d" %\
               (self.VPCId, self.Type, self.PfxSel)

    def Show(self):
        logger.info("VPC Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Prefix:%s" %self.IPPrefix)
        return

    def InitSubnetPefixPools(self, poolid, v6pfxlen, v4pfxlen):
        self.__ip_subnet_prefix_pool[0][poolid] =  resmgr.CreateIPv6SubnetPool(self.IPPrefix[0], v6pfxlen, poolid)
        self.__ip_subnet_prefix_pool[1][poolid] =  resmgr.CreateIPv4SubnetPool(self.IPPrefix[1], v4pfxlen, poolid)

    def AllocIPv6SubnetPrefix(self, poolid):
        return next(self.__ip_subnet_prefix_pool[0][poolid])

    def AllocIPv4SubnetPrefix(self, poolid):
        return next(self.__ip_subnet_prefix_pool[1][poolid])

    def GetProviderIPAddr(self, count):
        assert self.Type == vpc_pb2.VPC_TYPE_UNDERLAY
        if self.Stack == 'dual':
            paf = utils.IP_VERSION_6 if count % 2 == 0 else utils.IP_VERSION_4
        else:
            paf = utils.IP_VERSION_6 if self.Stack == 'ipv6' else utils.IP_VERSION_4
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
        grpcmsg.Id.append(str.encode(str(self.VPCId)))
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = str.encode(str(self.VPCId))
        spec.Type = self.Type
        spec.VirtualRouterMac = self.VirtualRouterMACAddr.getnum()
        utils.GetRpcEncap(self.Vnid, self.Vnid, spec.FabricEncap)
        if self.Nat46_pfx is not None:
            utils.GetRpcIPv6Prefix(self.Nat46_pfx, spec.Nat46Prefix)
        return

    def ValidateSpec(self, spec):
        if int(spec.Id) != self.VPCId:
            return False
        if spec.Type != self.Type:
            return False
        if utils.ValidateTunnelEncap(self.Vnid, spec.FabricEncap) is False:
            return False
        if utils.IsPipelineApulu():
            if spec.VirtualRouterMac != self.VirtualRouterMACAddr.getnum():
                return False
        return True

    def ValidateYamlSpec(self, spec):
        if utils.GetYamlSpecAttr(spec, 'id') != self.VPCId:
            return False
        if spec['type'] != self.Type:
            return False
        return True

    def IsUnderlayVPC(self):
        if self.Type == vpc_pb2.VPC_TYPE_UNDERLAY:
            return True
        return False

    def IsV6Stack(self):
        return utils.IsV6Stack(self.Stack)

class VpcObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.VPC, resmgr.MAX_VPC)
        return

    # TODO: move to GetObjectByKey
    def GetVpcObject(self, vpcid):
        return self.GetObjectByKey(vpcid)

    def GetKeyfromSpec(self, spec, yaml=False):
        if yaml:
            return utils.GetYamlSpecAttr(spec, 'id')
        return int(spec.Id)

    def __write_cfg(self, vpc_count):
        nh = NhClient.GetNumNextHopPerVPC()
        mtr = meter.client.GetNumMeterPerVPC()
        cfgjson.CfgJsonHelper.SetNumNexthopPerVPC(nh)
        cfgjson.CfgJsonHelper.SetNumMeterPerVPC(mtr[0], mtr[1])
        cfgjson.CfgJsonHelper.SetVPCCount(vpc_count)
        cfgjson.CfgJsonHelper.WriteConfig()

    def GenerateObjects(self, node, topospec):
        vpc_count = 0
        for p in topospec.vpc:
            vpc_count += p.count
            for c in range(p.count):
                if hasattr(p, "nat46"):
                    if p.nat46 is True and not utils.IsPipelineArtemis():
                        continue
                obj = VpcObject(node, p, c, p.count)
                self.Objs[node].update({obj.VPCId: obj})
                if obj.IsUnderlayVPC():
                    Store.SetUnderlayVPC(obj)
        # Write the flow and nexthop config to agent hook file
        if utils.IsFlowInstallationNeeded():
            self.__write_cfg(vpc_count)
        if utils.IsPipelineApulu():
            # Associate Nexthop objects
            NhGroupClient.CreateAllocator(node)
            NhClient.AssociateObjects(node)
            NhGroupClient.AssociateObjects(node)
            tunnel.client.FillUnderlayNhGroups(node)
            route.client.FillNhGroups(node)
            VnicClient.AssociateObjects(node)
        return

    def CreateObjects(self, node):
        super().CreateObjects(node)

        # Create Interface Objects
        InterfaceClient.CreateObjects(node)

        # Create Nexthop object
        NhClient.CreateObjects(node)

        # Create NexthopGroup object
        NhGroupClient.CreateObjects(node)

        # Create Tunnel object
        TunnelClient.CreateObjects(node)

        # Create Tag object.
        tag.client.CreateObjects(node)

        # Create Policy object.
        policy.client.CreateObjects(node)

        # Create Route object.
        route.client.CreateObjects(node)

        # Create Meter Objects
        meter.client.CreateObjects(node)

        # Create Subnet Objects after policy & route
        subnet.client.CreateObjects(node)
        return

client = VpcObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
