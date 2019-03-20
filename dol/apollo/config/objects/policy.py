#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.subnet as subnet
import types_pb2 as types_pb2
import policy_pb2 as policy_pb2

from infra.common.logging import logger
from apollo.config.store import Store

PROTO_TCP = 6

class PolicyObject(base.ConfigObjectBase):
    def __init__(self, topospec):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF SUBNET OBJECT #####################
        self.PolicyId = next(resmgr.SecurityPolicyIdAllocator)
        self.GID('Policy%d'%self.PolicyId)

        ################# PRIVATE ATTRIBUTES OF SUBNET OBJECT #####################
        return

    def __repr__(self):
        return "PolicyID:%d" % (self.PolicyId)

    def AddSubnets(self, subnet_obj, direction):
        self.L3Match = True
        self.L4Match = False
        self.Prefix = subnet_obj.Prefix
        self.Direction = direction
        self.Stateful = False
        if self.Prefix.version == 6:
            if self.Direction == types_pb2.RULE_DIR_INGRESS:
                self.PolicyId = subnet_obj.IngV6SecurityPolicyId
            else:
                self.PolicyId = subnet_obj.EgV6SecurityPolicyId
        else:
            if self.Direction == types_pb2.RULE_DIR_INGRESS:
                self.PolicyId = subnet_obj.IngV4SecurityPolicyId
            else:
                self.PolicyId = subnet_obj.EgV4SecurityPolicyId
        self.GID('Policy%d'%self.PolicyId)
        self.Show()

    def GetGrpcCreateMessage(self):
        grpcmsg = policy_pb2.SecurityPolicyRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.PolicyId
        print(str(spec))
        if self.Prefix.version == 6:
            spec.AddrFamily = types_pb2.IP_AF_INET6
        else:
            spec.AddrFamily = types_pb2.IP_AF_INET
        spec.Direction = self.Direction
        rule = spec.Rules.add()
        rule.Stateful = self.Stateful
        if self.L4Match:
            rule.Match.L4Match.Ports.SrcPortRange.PortLow = self.L4SportLow
            rule.Match.L4Match.Ports.SrcPortRange.PortHigh = self.L4SportHigh
            rule.Match.L4Match.Ports.DstPortRange.PortLow = self.L4DportLow
            rule.Match.L4Match.Ports.DstPortRange.PortHigh = self.L4DportHigh

        if self.L3Match:
            rule.Match.L3Match.Protocol = PROTO_TCP
            if self.Prefix.version == 6:
                rule.Match.L3Match.Prefix.Addr.Af = types_pb2.IP_AF_INET6
                rule.Match.L3Match.Prefix.Addr.V6Addr = self.Prefix.network_address.packed
            else:
                rule.Match.L3Match.Prefix.Addr.Af = types_pb2.IP_AF_INET
                rule.Match.L3Match.Prefix.Addr.V4Addr = int(self.Prefix.network_address)
            rule.Match.L3Match.Prefix.Len = self.Prefix.prefixlen

        return grpcmsg

    def Show(self):
        logger.info("Policy Object:", self)
        logger.info("- %s" % repr(self))
        if self.L3Match:
            logger.info("- Prefix:%s Dir:%d" %(self.Prefix, self.Direction))
        return

    def SetupTestcaseConfig(self, obj):
        return

class PolicyObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, topospec):
        sobjs = subnet.client.Objects()
        for sobj in sobjs:
            iv4id = sobj.IngV4SecurityPolicyId
            ev4id = sobj.EgV4SecurityPolicyId

            obj = PolicyObject(topospec)
            obj.AddSubnets(sobj, types_pb2.RULE_DIR_INGRESS)
            self.__objs.append(obj)
            obj = PolicyObject(topospec)
            obj.AddSubnets(sobj, types_pb2.RULE_DIR_EGRESS)
            self.__objs.append(obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.POLICY, msgs)
        return

client = PolicyObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
