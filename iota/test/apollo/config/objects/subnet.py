#! /usr/bin/python3
import enum
import pdb
import ipaddress
import sys

#Following come from dol/infra
import infra.config.base as base
import infra.common.defs as defs
import infra.common.parser as parser

import iota.test.apollo.config.resmgr as resmgr
import iota.test.apollo.config.utils as utils
import iota.test.apollo.config.agent.api as agent_api
import iota.test.apollo.config.objects.route as route
import iota.test.apollo.config.objects.policy as policy
import iota.test.apollo.config.objects.vnic as vnic


#import apollo.config.objects.vnic as vnic
#import apollo.config.objects.rmapping as rmapping
#import apollo.config.objects.policy as policy
#import apollo.config.objects.route as route

import subnet_pb2 as subnet_pb2
import types_pb2 as types_pb2


from iota.harness.infra.utils.logger import Logger as logger
from iota.test.apollo.config.store import Store


PROTO_TCP = 6
PROTO_UDP = 17
protos = {PROTO_TCP, PROTO_UDP}

class SubnetObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, poolid):
        super().__init__()
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
        self.EgV4SecurityPolicyId =  policy.client.GetEgV4SecurityPolicyId(parent.VPCId)
        self.EgV6SecurityPolicyId =  policy.client.GetEgV6SecurityPolicyId(parent.VPCId)
        self.V4RouteTable = route.client.GetRouteV4Table(parent.VPCId, self.V4RouteTableId)
        self.V6RouteTable = route.client.GetRouteV6Table(parent.VPCId, self.V6RouteTableId)
        ################# PRIVATE ATTRIBUTES OF SUBNET OBJECT #####################
        self.__ip_address_pool = {}
        self.__ip_address_pool[0] = resmgr.CreateIpv6AddrPool(self.IPPrefix[0])
        self.__ip_address_pool[1] = resmgr.CreateIpv4AddrPool(self.IPPrefix[1])

        #self.policyClient = parent.policyClient
        self.__set_vrouter_attributes()
        self.__fill_default_rules_in_policy()

        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate VNIC and Remote Mapping configuration
        vnic.client.GenerateObjects(self, spec)
        #rmapping.client.GenerateObjects(self, spec)

        return

    def __fill_default_rules_in_policy(self):
        ids = {self.IngV4SecurityPolicyId, self.EgV4SecurityPolicyId}
        for id in ids:
            policyobjs = policy.client.Objects()
            for policyobj in policyobjs:
                if policyobj.PolicyType == 'default':
                    if id == policyobj.PolicyId:
                        self.__fill_default_rules(policyobj)
        return

    def __fill_default_rules(self, policyobj):
        rules = []
        pfx = None
        if policyobj.AddrFamily == 'IPV4':
            if policyobj.PolicyType == 'default':
                pfx = ipaddress.ip_network('0.0.0.0/0')
            elif policyobj.PolicyType is 'subnet':
                pfx = ipaddress.ip_network(self.IPPrefix[1])
        else:
            if policyobj.PolicyType == 'default':
                pfx = ipaddress.ip_network('::/0')
            elif policyobj.PolicyType is 'subnet':
                pfx = ipaddress.ip_network(self.IPPrefix[0])
        for proto in protos:
            rule = policy.RuleObject(False, True, proto, pfx, True, 0, 65535, \
                                     0, 65535)
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

    def __repr__(self):
        return "SubnetID:%d|VPCId:%d|PfxSel:%d|MAC:%s" %\
               (self.SubnetId, self.VPC.VPCId, self.PfxSel, self.VirtualRouterMACAddr.get())

    def GetGrpcCreateMessage(self):
        grpcmsg = subnet_pb2.SubnetRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.SubnetId
        spec.VPCId = self.VPC.VPCId
        utils.GetRpcIPPrefix(self.IPPrefix[1], spec.V4Prefix)
        utils.GetRpcIPPrefix(self.IPPrefix[0], spec.V6Prefix)
        spec.IPv4VirtualRouterIP = int(self.VirtualRouterIPAddr[1])
        spec.IPv6VirtualRouterIP = self.VirtualRouterIPAddr[0].packed
        spec.VirtualRouterMac = self.VirtualRouterMACAddr.getnum()
        spec.V4RouteTableId = self.V4RouteTableId
        spec.V6RouteTableId = self.V6RouteTableId
        spec.IngV4SecurityPolicyId = self.IngV4SecurityPolicyId
        spec.IngV6SecurityPolicyId = self.IngV6SecurityPolicyId
        spec.EgV4SecurityPolicyId = self.EgV4SecurityPolicyId
        spec.EgV6SecurityPolicyId = self.EgV6SecurityPolicyId
        return grpcmsg

    def Show(self):
        logger.info("SUBNET object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Prefix %s" % self.IPPrefix)
        logger.info("- VirtualRouter IP:%s" % (self.VirtualRouterIPAddr))
        logger.info("- TableIds V4:%d|V6:%d" % (self.V4RouteTableId, self.V6RouteTableId))
        logger.info("- SecurityPolicyIDs IngV4:%d|IngV6:%d|EgV4:%d|EgV6:%d" %\
                    (self.IngV4SecurityPolicyId, self.IngV6SecurityPolicyId, self.EgV4SecurityPolicyId, self.EgV6SecurityPolicyId))
        return

    def SetupTestcaseConfig(self, obj):
        return

class SubnetObjectClient:
    def __init__(self, devices):
        self.__objs = []
        self.devices = devices
        return

    def Objects(self):
        return self.__objs

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
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        for device in self.devices:
            logger.info("Creating subnet objects for device :", device)
            device.client.Create(agent_api.ObjectTypes.SUBNET, msgs)
        # Create VNIC and Remote Mapping Objects
        #vnic.client.CreateObjects()
        #rmapping.client.CreateObjects()
        return


client = None
def NewSubnetObjectClient(devices):
    global client
    client = SubnetObjectClient(devices)

def GetMatchingObjects(selectors):
    return client.Objects()
