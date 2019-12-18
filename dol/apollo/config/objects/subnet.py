#! /usr/bin/python3
import pdb
import ipaddress
import itertools

from infra.common.logging import logger
from apollo.config.store import Store

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.base as base
from apollo.config.objects.interface import client as InterfaceClient
import apollo.config.objects.vnic as vnic
import apollo.config.objects.rmapping as rmapping
import apollo.config.objects.policy as policy
import apollo.config.objects.route as route
import apollo.config.utils as utils

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
    def __init__(self, parent, spec, poolid):
        super().__init__(api.ObjectTypes.SUBNET)
        ################# PUBLIC ATTRIBUTES OF SUBNET OBJECT #####################
        self.SubnetId = next(resmgr.SubnetIdAllocator)
        self.GID('Subnet%d'%self.SubnetId)
        self.VPC = parent
        self.PfxSel = parent.PfxSel
        self.IPPrefix = {}
        self.IPPrefix[0] = parent.AllocIPv6SubnetPrefix(poolid)
        self.IPPrefix[1] = parent.AllocIPv4SubnetPrefix(poolid)
        self.VirtualRouterIPAddr = {}
        self.VirtualRouterMacAddr = None
        self.V4RouteTableId = route.client.GetRouteV4TableId(parent.VPCId, self, "V4RTID:" + str(self.SubnetId))
        self.V6RouteTableId = route.client.GetRouteV6TableId(parent.VPCId)
        self.IngV4SecurityPolicyIds = [policy.client.GetIngV4SecurityPolicyId(parent.VPCId)]
        self.IngV6SecurityPolicyIds = [policy.client.GetIngV6SecurityPolicyId(parent.VPCId)]
        self.EgV4SecurityPolicyIds = [policy.client.GetEgV4SecurityPolicyId(parent.VPCId)]
        self.EgV6SecurityPolicyIds = [policy.client.GetEgV6SecurityPolicyId(parent.VPCId)]
        self.V4RouteTable = route.client.GetRouteV4Table(parent.VPCId, self.V4RouteTableId)
        self.V6RouteTable = route.client.GetRouteV6Table(parent.VPCId, self.V6RouteTableId)
        self.Vnid = next(resmgr.VxlanIdAllocator)
        self.HostIf = InterfaceClient.GetHostInterface()
        self.Status = SubnetStatus()
        ################# PRIVATE ATTRIBUTES OF SUBNET OBJECT #####################
        self.__ip_address_pool = {}
        self.__ip_address_pool[0] = resmgr.CreateIpv6AddrPool(self.IPPrefix[0])
        self.__ip_address_pool[1] = resmgr.CreateIpv4AddrPool(self.IPPrefix[1])

        self.__set_vrouter_attributes()
        self.__fill_default_rules_in_policy()

        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate VNIC and Remote Mapping configuration
        vnic.client.GenerateObjects(self, spec)
        rmapping.client.GenerateObjects(self, spec)

        return

    def __repr__(self):
        return "SubnetID:%d|VPCId:%d|PfxSel:%d|MAC:%s" %\
               (self.SubnetId, self.VPC.VPCId, self.PfxSel, self.VirtualRouterMACAddr.get())

    def Show(self):
        logger.info("SUBNET object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Prefix %s VNI %d" % (self.IPPrefix, self.Vnid))
        logger.info("- VirtualRouter IP:%s" % (self.VirtualRouterIPAddr))
        logger.info("- TableIds V4:%d|V6:%d" % (self.V4RouteTableId, self.V6RouteTableId))
        logger.info("- NaclIDs IngV4:%d|IngV6:%d|EgV4:%d|EgV6:%d" %\
                    (self.IngV4SecurityPolicyIds[0], self.IngV6SecurityPolicyIds[0], self.EgV4SecurityPolicyIds[0], self.EgV6SecurityPolicyIds[0]))
        if self.HostIf:
            logger.info("- HostInterface:", self.HostIf.Ifname)
        return

    def __fill_default_rules_in_policy(self):
        ids = itertools.chain(self.IngV4SecurityPolicyIds, self.EgV4SecurityPolicyIds, self.IngV6SecurityPolicyIds, self.EgV6SecurityPolicyIds)
        for policyid in ids:
            if policyid is 0:
                continue
            policyobj = policy.client.GetPolicyObject(policyid)
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
        if utils.IsPipelineApulu():
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

    def DeleteNotify(self, ident):
        logger.info("Delete notify %s ident %s" %(__repr__()), ident)
        if not self.IsDeleted():
            logger.info("Applying object modification")
            # Need to assign new route table/policy based on ident
            # Send a modify message to agent

class SubnetObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.SUBNET, resmgr.MAX_SUBNET)
        return

    def GetSubnetObject(self, subnetid):
        return self.GetObjectByKey(subnetid)

    def GenerateObjects(self, parent, vpc_spec_obj):
        poolid = 0
        for subnet_spec_obj in vpc_spec_obj.subnet:
            parent.InitSubnetPefixPools(poolid, subnet_spec_obj.v6prefixlen, subnet_spec_obj.v4prefixlen)
            for c in range(subnet_spec_obj.count):
                obj = SubnetObject(parent, subnet_spec_obj, poolid)
                self.Objs.update({obj.SubnetId: obj})
            poolid = poolid + 1
        return

    def CreateObjects(self):
        logger.info("Creating Subnet Objects in agent")
        cookie = utils.GetBatchCookie()
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.Objects()))
        api.client.Create(api.ObjectTypes.SUBNET, msgs)
        # Create VNIC and Remote Mapping Objects
        vnic.client.CreateObjects()
        rmapping.client.CreateObjects()
        return

client = SubnetObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
