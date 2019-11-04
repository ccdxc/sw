#! /usr/bin/python3
import pdb
import ipaddress

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

#TODO: move these protos
PROTO_TCP = 6
PROTO_UDP = 17
protos = {PROTO_TCP, PROTO_UDP}

class SubnetObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, poolid):
        super().__init__()
        self.SetBaseClassAttr()
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
        self.V4RouteTableId = route.client.GetRouteV4TableId(parent.VPCId)
        self.V6RouteTableId = route.client.GetRouteV6TableId(parent.VPCId)
        self.IngV4SecurityPolicyId = policy.client.GetIngV4SecurityPolicyId(parent.VPCId)
        self.IngV6SecurityPolicyId = policy.client.GetIngV6SecurityPolicyId(parent.VPCId)
        self.EgV4SecurityPolicyId = policy.client.GetEgV4SecurityPolicyId(parent.VPCId)
        self.EgV6SecurityPolicyId = policy.client.GetEgV6SecurityPolicyId(parent.VPCId)
        self.V4RouteTable = route.client.GetRouteV4Table(parent.VPCId, self.V4RouteTableId)
        self.V6RouteTable = route.client.GetRouteV6Table(parent.VPCId, self.V6RouteTableId)
        self.Vnid = next(resmgr.VxlanIdAllocator)
        self.HostIf = InterfaceClient.GetHostInterface()
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
        logger.info("- Prefix %s" % self.IPPrefix)
        logger.info("- VirtualRouter IP:%s" % (self.VirtualRouterIPAddr))
        logger.info("- TableIds V4:%d|V6:%d" % (self.V4RouteTableId, self.V6RouteTableId))
        logger.info("- SecurityPolicyIDs IngV4:%d|IngV6:%d|EgV4:%d|EgV6:%d" %\
                    (self.IngV4SecurityPolicyId, self.IngV6SecurityPolicyId, self.EgV4SecurityPolicyId, self.EgV6SecurityPolicyId))
        if self.HostIf:
            logger.info("- HostInterface:", self.HostIf.Ifname)
        return

    def __fill_default_rules_in_policy(self):
        ids = [self.IngV4SecurityPolicyId, self.EgV4SecurityPolicyId]
        ids += [self.IngV6SecurityPolicyId, self.EgV6SecurityPolicyId]
        for policyid in ids:
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
        srcPfx = pfx
        dstPfx = pfx
        l4match = policy.L4MatchObject(True)
        for proto in protos:
            l3match = policy.L3MatchObject(True, proto, srcpfx=srcPfx, dstpfx=dstPfx)
            rule = policy.RuleObject(l3match, l4match)
            rules.append(rule)
        policyobj.rules = rules
        policyobj.Show()

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

    def SetBaseClassAttr(self):
        self.ObjType = api.ObjectTypes.SUBNET
        return

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
        spec.IngV4SecurityPolicyId = self.IngV4SecurityPolicyId
        spec.IngV6SecurityPolicyId = self.IngV6SecurityPolicyId
        spec.EgV4SecurityPolicyId = self.EgV4SecurityPolicyId
        spec.EgV6SecurityPolicyId = self.EgV6SecurityPolicyId
        utils.GetRpcEncap(0, self.Vnid, spec.FabricEncap)
        if self.HostIf:
            spec.HostIfIndex = utils.LifId2LifIfIndex(self.HostIf.lif.id)
        return


class SubnetObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def IsValidConfig(self):
        count = len(self.__objs)
        if  count > resmgr.MAX_SUBNET:
            return False, "Subnet count %d exceeds allowed limit of %d" %\
                          (count, resmgr.MAX_SUBNET)
        return True, ""

    def GenerateObjects(self, parent, vpc_spec_obj):
        poolid = 0
        for subnet_spec_obj in vpc_spec_obj.subnet:
            parent.InitSubnetPefixPools(poolid, subnet_spec_obj.v6prefixlen, subnet_spec_obj.v4prefixlen)
            for c in range(subnet_spec_obj.count):
                obj = SubnetObject(parent, subnet_spec_obj, poolid)
                self.__objs.append(obj)
            poolid = poolid + 1
        return

    def CreateObjects(self):
        cookie = utils.GetBatchCookie()
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__objs))
        api.client.Create(api.ObjectTypes.SUBNET, msgs)
        # Create VNIC and Remote Mapping Objects
        vnic.client.CreateObjects()
        rmapping.client.CreateObjects()
        return

    def GetGrpcReadAllMessage(self):
        grpcmsg = subnet_pb2.SubnetGetRequest()
        return grpcmsg

    def ReadObjects(self):
        msg = self.GetGrpcReadAllMessage()
        api.client.Get(api.ObjectTypes.SUBNET, [msg])
        return

client = SubnetObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
