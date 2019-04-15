#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.subnet as subnet
import types_pb2 as types_pb2
import policy_pb2 as policy_pb2
import apollo.config.utils as utils
import ipaddress as ipaddress

from infra.common.logging import logger
from apollo.config.store import Store

PROTO_TCP = 6
PROTO_UDP = 17

protos = {PROTO_TCP, PROTO_UDP}

class rule_obj:
    def __init__(self, af, stateful, l3match, prefix, proto, l4match):
        self.af = af
        self.Stateful = stateful
        self.L3Match = l3match
        self.Prefix = prefix
        self.Proto = proto
        self.L4Match = l4match
        self.L4SportLow = 0
        self.L4SportHigh = 65535
        self.L4DportLow = 0
        self.L4DportHigh = 65535

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

    def GetPrefix(self, subnet_obj, af, wildcard):
        if af is 'IPV4':
            if wildcard is True:
                return ipaddress.ip_network('0.0.0.0/0')
            else:
                return subnet_obj.IPPrefix[1]
        elif af is 'IPV6':
            if wildcard is True:
                return ipaddress.ip_network('0::0/0')
            else:
                return subnet_obj.IPPrefix[0]
        else:
            return None

    def GetRule(self, subnet_obj, proto, af, wildcard):
        prefix = self.GetPrefix(subnet_obj, af, wildcard)
        rule = rule_obj(af, False, True, prefix, proto, True)
        return rule

    def GetRules(self, subnet_obj, af, wildcard):
        rules = []
        for proto in protos:
            rules.append(self.GetRule(subnet_obj, proto, af, wildcard))
        return rules

    def GetWildcardPolicyId(self, direction, af):
        if af is 'IPV4':
            if direction is types_pb2.RULE_DIR_INGRESS:
                return IngV4SecurityPolicyId
            elif direction is types_pb2.RULE_DIR_EGRESS:
                return EgV4SecurityPolicyId
            else:
                return None
        elif af is 'IPV6':
            if direction is types_pb2.RULE_DIR_INGRESS:
                return IngV6SecurityPolicyId
            elif direction is types_pb2.RULE_DIR_EGRESS:
                return EgV6SecurityPolicyId
            else:
                return None
        else:
            return None

    def GetGeneralPolicyId(self, sobj, direction, af):
        if af is 'IPV4':
            if direction == types_pb2.RULE_DIR_INGRESS:
                return sobj.IngV4SecurityPolicyId
            elif direction == types_pb2.RULE_DIR_EGRESS:
                return sobj.EgV4SecurityPolicyId
            else:
                return None
        elif af == 'IPV6':
            if direction == types_pb2.RULE_DIR_INGRESS:
                return sobj.IngV6SecurityPolicyId
            elif direction == types_pb2.RULE_DIR_EGRESS:
                return sobj.EgV6SecurityPolicyId
            else:
                return None
        else:
            return None

    def GetPolicyId(self, sobj, direction, af, wildcard):
        if wildcard is True:
            return self.GetWildcardPolicyId(direction, af)
        else:
            return self.GetGeneralPolicyId(sobj, direction, af)

    def ConfigRulesAndIds(self, sobj, direction, af, wildcard):
        self.__subnetobj = sobj
        self.PolicyId = self.GetPolicyId(sobj, direction, af, False)
        self.af = af
        self.rules = self.GetRules(sobj, af, wildcard)
        self.num_rules = len(self.rules)
        self.Direction = direction
        #TODO: add support for multiple rules per policy
        self.Stateful = False
        self.GID('Policy%d'%self.PolicyId)
        self.Show()

    def FillRule(self, spec, rule):
        specrule = spec.Rules.add()
        specrule.Stateful = rule.Stateful
        if rule.L4Match:
            specrule.Match.L4Match.Ports.SrcPortRange.PortLow = rule.L4SportLow
            specrule.Match.L4Match.Ports.SrcPortRange.PortHigh = rule.L4SportHigh
            specrule.Match.L4Match.Ports.DstPortRange.PortLow = rule.L4DportLow
            specrule.Match.L4Match.Ports.DstPortRange.PortHigh = rule.L4DportHigh
        if rule.L3Match:
            specrule.Match.L3Match.Protocol = rule.Proto
            utils.GetRpcIPPrefix(rule.Prefix, specrule.Match.L3Match.Prefix)

    def GetGrpcCreateMessage(self):
        grpcmsg = policy_pb2.SecurityPolicyRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.PolicyId
        if self.af == 'IPV6':
            spec.AddrFamily = types_pb2.IP_AF_INET6
        else:
            spec.AddrFamily = types_pb2.IP_AF_INET
        for rule in self.rules:
            self.FillRule(spec, rule)
        spec.Direction = self.Direction
        return grpcmsg

    def Show(self):
        logger.info("Policy Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Dir:%d" % self.Direction)
        logger.info("- Number of rules:%d" % len(self.rules))
        for rule in self.rules:
            if rule.L3Match:
                logger.info("- Prefix:%s Proto:%d" %(rule.Prefix, rule.Proto))
            if rule.L4Match:
                logger.info("- SrcPortRange:%d - %d DstPortRange:%d - %d" %(rule.L4SportLow, rule.L4SportHigh, rule.L4DportLow, rule.L4DportHigh))
        return

    def SetupTestcaseConfig(self, obj):
        obj.subnet = self.__subnetobj
        obj.hostport = 1
        obj.switchport = 2
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
            obj = PolicyObject(topospec)
            obj.ConfigRulesAndIds(sobj, types_pb2.RULE_DIR_INGRESS, 'IPV4', False)
            self.__objs.append(obj)
            #obj = PolicyObject(topospec)
            #obj.ConfigRulesAndIds(sobj, types_pb2.RULE_DIR_INGRESS, 'IPV6', False)
            #self.__objs.append(obj)
            obj = PolicyObject(topospec)
            obj.ConfigRulesAndIds(sobj, types_pb2.RULE_DIR_EGRESS, 'IPV4', False)
            self.__objs.append(obj)
            #obj = PolicyObject(topospec)
            #obj.ConfigRulesAndIds(sobj, types_pb2.RULE_DIR_EGRESS, 'IPV6', False)
            #self.__objs.append(obj)
        return

    def GenerateWildcardObjects(self, topospec):
        sobjs = subnet.client.Objects()
        for sobj in sobjs:
            obj = PolicyObject(topospec)
            obj.ConfigRulesAndIds(sobj, types_pb2.RULE_DIR_INGRESS, 'IPV4', True)
            self.__objs.append(obj)
            #obj = PolicyObject(topospec)
            #obj.ConfigRulesAndIds(sobj, types_pb2.RULE_DIR_INGRESS, 'IPV6', True)
            #self.__objs.append(obj)
            obj = PolicyObject(topospec)
            obj.ConfigRulesAndIds(sobj, types_pb2.RULE_DIR_EGRESS, 'IPV4', True)
            self.__objs.append(obj)
            #obj = PolicyObject(topospec)
            #obj.ConfigRulesAndIds(sobj, types_pb2.RULE_DIR_EGRESS, 'IPV6', True)
            #self.__objs.append(obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.POLICY, msgs)
        return

client = PolicyObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
