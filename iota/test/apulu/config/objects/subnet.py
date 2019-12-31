#! /usr/bin/python3
import pdb
import ipaddress
import itertools

from infra.common.logging import logger
from iota.test.apulu.config.store import Store

import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.agent.api as api
import iota.test.apulu.config.objects.base as base
from iota.test.apulu.config.objects.interface import client as InterfaceClient
import iota.test.apulu.config.objects.vnic as vnic
import iota.test.apulu.config.objects.rmapping as rmapping
import iota.test.apulu.config.objects.policy as policy
import iota.test.apulu.config.objects.route as route
import iota.test.apulu.config.utils as utils

import subnet_pb2 as subnet_pb2

class SubnetStatus(base.StatusObjectBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.SUBNET)
        self.HwId = None
        return

    def Update(self, status):
        self.HwId = status.HwId
        return

class SubnetObject(base.ConfigObjectBase):
    def __init__(self, node, parent, spec, poolid):
        super().__init__(api.ObjectTypes.SUBNET)
        ################# PUBLIC ATTRIBUTES OF SUBNET OBJECT #####################
        if (hasattr(spec, 'id')):
            self.SubnetId = spec.id
        else:
            self.SubnetId = next(resmgr.SubnetIdAllocator)
        self.GID('Subnet%d'%self.SubnetId)
        self.VPC = parent
        self.PfxSel = parent.PfxSel
        self.IPPrefix = {}
        if getattr(spec, 'v4prefix', None) != None:
            self.IPPrefix[1] = ipaddress.ip_network(spec.v4prefix.replace('\\', '/'))
        else:
            self.IPPrefix[1] = parent.AllocIPv4SubnetPrefix(poolid)
        self.IPPrefix[0] = parent.AllocIPv6SubnetPrefix(poolid)
        self.VirtualRouterIPAddr = {}
        self.VirtualRouterMacAddr = None
        self.V4RouteTableId = route.client.GetRouteV4TableId(node, parent.VPCId)
        self.V6RouteTableId = route.client.GetRouteV6TableId(node, parent.VPCId)
        self.IngV4SecurityPolicyIds = [policy.client.GetIngV4SecurityPolicyId(node, parent.VPCId)]
        self.IngV6SecurityPolicyIds = [policy.client.GetIngV6SecurityPolicyId(node, parent.VPCId)]
        self.EgV4SecurityPolicyIds = [policy.client.GetEgV4SecurityPolicyId(node, parent.VPCId)]
        self.EgV6SecurityPolicyIds = [policy.client.GetEgV6SecurityPolicyId(node, parent.VPCId)]
        self.V4RouteTable = route.client.GetRouteV4Table(node, parent.VPCId, self.V4RouteTableId)
        self.V6RouteTable = route.client.GetRouteV6Table(node, parent.VPCId, self.V6RouteTableId)
        if getattr(spec, 'fabricencap', None) != None:
            self.FabricEncap = utils.GetEncapType(spec.fabricencap)
        if getattr(spec, 'fabricencapvalue', None) != None:
            self.Vnid = spec.fabricencapvalue
        else:
            self.Vnid = next(resmgr.VxlanIdAllocator)
        if getattr(spec, 'hostifidx', None) != None:
            self.HostIfIdx = spec.hostifidx
        self.HostIf = InterfaceClient.GetHostInterface(node)
        self.Status = SubnetStatus()
        ################# PRIVATE ATTRIBUTES OF SUBNET OBJECT #####################
        self.__ip_address_pool = {}
        self.__ip_address_pool[0] = resmgr.CreateIpv6AddrPool(self.IPPrefix[0])
        self.__ip_address_pool[1] = resmgr.CreateIpv4AddrPool(self.IPPrefix[1])

        self.__set_vrouter_attributes()
        self.__fill_default_rules_in_policy(node)

        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate VNIC and Remote Mapping configuration
        vnic.client.GenerateObjects(node, self, spec)
        rmapping.client.GenerateObjects(node, self, spec)

        return

    def __repr__(self):
        return "SubnetID:%d|VPCId:%d|PfxSel:%d|MAC:%s" %\
               (self.SubnetId, self.VPC.VPCId, self.PfxSel, self.VirtualRouterMACAddr.get())

    def Show(self):
        logger.info("SUBNET object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Prefix %s Encap %s VNI %d" % (self.IPPrefix, utils.GetEncapTypeString(self.FabricEncap), self.Vnid))
        logger.info("- VirtualRouter IP:%s" % (self.VirtualRouterIPAddr))
        logger.info("- TableIds V4:%d|V6:%d" % (self.V4RouteTableId, self.V6RouteTableId))
        logger.info("- NaclIDs IngV4:%d|IngV6:%d|EgV4:%d|EgV6:%d" %\
                    (self.IngV4SecurityPolicyIds[0], self.IngV6SecurityPolicyIds[0], self.EgV4SecurityPolicyIds[0], self.EgV6SecurityPolicyIds[0]))
        if self.HostIf:
            logger.info("- HostInterface:", self.HostIf.Ifname)
        if self.HostIfIdx:
            logger.info("- HostInterfaceId:", self.HostIfIdx)
        return

    def __fill_default_rules_in_policy(self, node):
        ids = itertools.chain(self.IngV4SecurityPolicyIds, self.EgV4SecurityPolicyIds, self.IngV6SecurityPolicyIds, self.EgV6SecurityPolicyIds)
        for policyid in ids:
            if policyid is 0:
                continue
            policyobj = policy.client.GetPolicyObject(node, policyid)
            if policyobj.PolicyType == 'default':
                #TODO: move this to policy.py
                self.__fill_default_rules(policyobj)
            else:
                policy.client.ModifyPolicyRules(policyid, self)
        return

    def __fill_default_rules(self, policyobj):
        rules = []
        pfx = None
        srcPfx = None
        dstPfx = None
        if policyobj.AddrFamily == 'IPV4':
            if policyobj.PolicyType == 'default':
                pfx = utils.IPV4_DEFAULT_ROUTE
            elif policyobj.PolicyType is 'subnet':
                pfx = ipaddress.ip_network(self.IPPrefix[1])
        else:
            if policyobj.PolicyType == 'default':
                pfx = utils.IPV6_DEFAULT_ROUTE
            elif policyobj.PolicyType is 'subnet':
                pfx = ipaddress.ip_network(self.IPPrefix[0])
        policyobj.rules = policy.client.Generate_Allow_All_Rules(srcPfx, dstPfx)

    def __set_vrouter_attributes(self):
        # 1st IP address of the subnet becomes the vrouter.
        self.VirtualRouterIPAddr[0] = next(self.__ip_address_pool[0])
        self.VirtualRouterIPAddr[1] = next(self.__ip_address_pool[1])
        self.VirtualRouterMACAddr = resmgr.VirtualRouterMacAllocator.get()
        return

    def AllocIPv6Address(self):
        return next(self.__ip_address_pool[0])

    def AllocIPv4Address(self):
        return next(self.__ip_address_pool[1])

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.SubnetId)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.SubnetId
        spec.VPCId = self.VPC.VPCId
        utils.GetRpcIPv4Prefix(self.IPPrefix[1], spec.V4Prefix)
        utils.GetRpcIPv6Prefix(self.IPPrefix[0], spec.V6Prefix)
        spec.IPv4VirtualRouterIP = int(self.VirtualRouterIPAddr[1])
        spec.IPv6VirtualRouterIP = self.VirtualRouterIPAddr[0].packed
        spec.VirtualRouterMac = self.VirtualRouterMACAddr.getnum()
        spec.V4RouteTableId = self.V4RouteTableId
        spec.V6RouteTableId = self.V6RouteTableId
        for policyid in self.IngV4SecurityPolicyIds:
            spec.IngV4SecurityPolicyId.append(policyid)
        for policyid in self.IngV6SecurityPolicyIds:
            spec.IngV6SecurityPolicyId.append(policyid)
        for policyid in self.EgV4SecurityPolicyIds:
            spec.EgV4SecurityPolicyId.append(policyid)
        for policyid in self.EgV6SecurityPolicyIds:
            spec.EgV6SecurityPolicyId.append(policyid)
        utils.GetRpcEncap(self.Vnid, self.Vnid, spec.FabricEncap)
        if self.HostIfIdx:
            spec.HostIfIndex = self.HostIfIdx
        if self.HostIf:
            spec.HostIfIndex = utils.LifId2LifIfIndex(self.HostIf.lif.id)
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.SubnetId:
            return False
        if spec.VPCId != self.VPC.VPCId:
            return False
        if spec.VirtualRouterMac != self.VirtualRouterMACAddr.getnum():
            return False
        if spec.V4RouteTableId != self.V4RouteTableId:
            return False
        if spec.V6RouteTableId != self.V6RouteTableId:
            return False
        if spec.IngV4SecurityPolicyId[0] != self.IngV4SecurityPolicyIds[0]:
            return False
        if spec.IngV6SecurityPolicyId[0] != self.IngV6SecurityPolicyIds[0]:
            return False
        if spec.EgV4SecurityPolicyId[0] !=  self.EgV4SecurityPolicyIds[0]:
            return False
        if spec.EgV6SecurityPolicyId[0] != self.EgV6SecurityPolicyIds[0]:
            return False
        if utils.ValidateTunnelEncap(self.Vnid, spec.FabricEncap) is False:
            return False
        if utils.IsPipelineApulu():
            if self.HostIf:
                if spec.HostIfIndex != utils.LifId2LifIfIndex(self.HostIf.lif.id):
                    return False
        return True

    def ValidateYamlSpec(self, spec):
        if spec['id'] != self.SubnetId:
            return False
        return True

    def GetNaclId(self, direction, af=utils.IP_VERSION_4):
        if af == utils.IP_VERSION_4:
            if direction == 'ingress':
                return self.IngV4SecurityPolicyIds[0]
            else:
                return self.EgV4SecurityPolicyIds[0]
        elif af == utils.IP_VERSION_6:
            if direction == 'ingress':
                return self.IngV6SecurityPolicyIds[0]
            else:
                return self.EgV6SecurityPolicyIds[0]
        return None

class SubnetObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.SUBNET, resmgr.MAX_SUBNET)
        return

    def GetSubnetObject(self, node, subnetid):
        return self.GetObjectByKey(node, subnetid)

    def GenerateObjects(self, node, parent, vpc_spec_obj):
        poolid = 0
        for subnet_spec_obj in vpc_spec_obj.subnet:
            parent.InitSubnetPefixPools(poolid, subnet_spec_obj.v6prefixlen, subnet_spec_obj.v4prefixlen)
            for c in range(subnet_spec_obj.count):
                obj = SubnetObject(node, parent, subnet_spec_obj, poolid)
                self.Objs[node].update({obj.SubnetId: obj})
            poolid = poolid + 1
        return

    def CreateObjects(self, node):
        logger.info("Creating Subnet Objects in agent")
        cookie = utils.GetBatchCookie(node)
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(node, cookie), self.Objects(node)))
        api.client[node].Create(api.ObjectTypes.SUBNET, msgs)
        # Create VNIC and Remote Mapping Objects
        vnic.client.CreateObjects(node)
        rmapping.client.CreateObjects(node)
        return

client = SubnetObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
