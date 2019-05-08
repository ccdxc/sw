#! /usr/bin/python3
import pdb
import ipaddress
import random
import socket

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import types_pb2 as types_pb2
import policy_pb2 as policy_pb2
import apollo.config.objects.lmapping as lmapping
import apollo.config.utils as utils

from infra.common.logging import logger
from apollo.config.store import Store

class RuleObject:
    def __init__(self, stateful, l3match, proto, prefix, l4match, sportlow, sporthigh, dportlow, dporthigh):
        self.Stateful = stateful
        self.L3Match = l3match
        self.Proto = proto
        self.Prefix = prefix
        self.L4Match = l4match
        self.L4SportLow = sportlow
        self.L4SportHigh = sporthigh
        self.L4DportLow = dportlow
        self.L4DportHigh = dporthigh

    def Show(self):
        if self.L3Match:
            logger.info("- Prefix:%s Proto:%d" %(self.Prefix, self.Proto))
        else:
            logger.info("- No L3Match")
        if self.L4Match:
            logger.info("- SrcPortRange:%d - %d DstPortRange:%d - %d" %(self.L4SportLow, self.L4SportHigh, self.L4DportLow, self.L4DportHigh))
        else:
            logger.info("- No L4Match")

class PolicyObject(base.ConfigObjectBase):
    def __init__(self, parent, af, direction, rules, policytype, overlaptype):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF POLICY OBJECT #####################
        self.VPCId = parent.VPCId
        self.Direction = direction
        if af == utils.IP_VERSION_6:
            self.PolicyId = next(resmgr.V6SecurityPolicyIdAllocator)
            self.AddrFamily = 'IPV6'
        else:
            self.PolicyId = next(resmgr.V4SecurityPolicyIdAllocator)
            self.AddrFamily = 'IPV4'
        self.GID('Policy%d'%self.PolicyId)
        ################# PRIVATE ATTRIBUTES OF POLICY OBJECT #####################
        self.PolicyType = policytype
        self.OverlapType = overlaptype
        self.rules = rules
        self.Show()
        return

    def __repr__(self):
        return "PolicyID:%d" % (self.PolicyId)

    def FillRuleSpec(self, spec, rule):
        specrule = spec.Rules.add()
        specrule.Stateful = rule.Stateful
        if rule.L4Match:
            specrule.Match.L4Match.Ports.SrcPortRange.PortLow = rule.L4SportLow
            specrule.Match.L4Match.Ports.SrcPortRange.PortHigh = rule.L4SportHigh
            specrule.Match.L4Match.Ports.DstPortRange.PortLow = rule.L4DportLow
            specrule.Match.L4Match.Ports.DstPortRange.PortHigh = rule.L4DportHigh
        if rule.L3Match:
            specrule.Match.L3Match.Protocol = rule.Proto
            if rule.Prefix is not None:
                utils.GetRpcIPPrefix(rule.Prefix, specrule.Match.L3Match.Prefix)

    def GetGrpcCreateMessage(self):
        grpcmsg = policy_pb2.SecurityPolicyRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.PolicyId
        spec.Direction = types_pb2.RULE_DIR_INGRESS if self.Direction == "ingress" else types_pb2.RULE_DIR_EGRESS
        spec.AddrFamily = types_pb2.IP_AF_INET6 if self.AddrFamily == 'IPV6' else types_pb2.IP_AF_INET
        for rule in self.rules:
            self.FillRuleSpec(spec, rule)
        return grpcmsg

    def Show(self):
        logger.info("Policy Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Direction:%s" % self.Direction)
        logger.info("- PolicyType:%s" % self.PolicyType)
        logger.info("- OverlapType:%s" % self.OverlapType)
        logger.info("- Number of rules:%d" % len(self.rules))
        for rule in self.rules:
            rule.Show()
        return

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.policy.filters)

    def __get_non_default_random_rule(self):
        """
            returns a random rule without default prefix
        """
        rules = self.rules
        numrules = len(rules)
        if numrules == 0:
            return None
        elif numrules == 1:
            rule = None
            if not utils.isDefaultRoute(rules[0].Prefix):
                rule = rules[0]
            return rule
        while True:
            rule = random.choice(rules)
            if not utils.isDefaultRoute(rule.Prefix):
                break
        return rule

    def SetupTestcaseConfig(self, obj):
        obj.localmapping = self.l_obj
        obj.policy = self
        obj.route = self.l_obj.VNIC.SUBNET.V6RouteTable if self.AddrFamily == 'IPV6' else self.l_obj.VNIC.SUBNET.V4RouteTable
        obj.hostport = utils.PortTypes.HOST
        obj.switchport = utils.PortTypes.SWITCH
        obj.devicecfg = Store.GetDevice()
        # select a random rule for this testcase
        obj.tc_rule = self.__get_non_default_random_rule()
        return

class PolicyObjectClient:
    def __init__(self):
        self.__objs = []
        self.__v4ingressobjs = {}
        self.__v6ingressobjs = {}
        self.__v4egressobjs = {}
        self.__v6egressobjs = {}
        self.__v4ipolicyiter = {}
        self.__v6ipolicyiter = {}
        self.__v4epolicyiter = {}
        self.__v6epolicyiter = {}
        return

    def Objects(self):
        return self.__objs

    def GetIngV4SecurityPolicyId(self, vpcid):
        if len(self.__v4ingressobjs[vpcid]) == 0:
            return 0
        return self.__v4ipolicyiter[vpcid].rrnext().PolicyId

    def GetIngV6SecurityPolicyId(self, vpcid):
        if len(self.__v6ingressobjs[vpcid]) == 0:
            return 0
        return self.__v6ipolicyiter[vpcid].rrnext().PolicyId

    def GetEgV4SecurityPolicyId(self, vpcid):
        if len(self.__v4egressobjs[vpcid]) == 0:
            return 0
        return self.__v4epolicyiter[vpcid].rrnext().PolicyId

    def GetEgV6SecurityPolicyId(self, vpcid):
        if len(self.__v6egressobjs[vpcid]) == 0:
            return 0
        return self.__v6epolicyiter[vpcid].rrnext().PolicyId

    def GenerateObjects(self, parent, vpc_spec_obj):
        vpcid = parent.VPCId
        stack = parent.Stack
        self.__objs = []
        self.__v4ingressobjs[vpcid] = []
        self.__v6ingressobjs[vpcid] = []
        self.__v4egressobjs[vpcid] = []
        self.__v6egressobjs[vpcid] = []
        self.__v4ipolicyiter[vpcid] = None
        self.__v6ipolicyiter[vpcid] = None
        self.__v4epolicyiter[vpcid] = None
        self.__v6epolicyiter[vpcid] = None

        def __is_v4stack():
            if stack == "dual" or stack == 'ipv4':
                return True
            return False

        def __is_v6stack():
            return False #v6 policy not supported in HAL yet
            if stack == "dual" or stack == 'ipv6':
                return True
            return False

        def __get_l4_rule(af, rulespec):
            sportlow = rulespec.sportlow if hasattr(rulespec, 'sportlow') else 0
            dportlow = rulespec.dportlow if hasattr(rulespec, 'dportlow') else 0
            sporthigh = rulespec.sporthigh if hasattr(rulespec, 'sporthigh') else 65535
            dporthigh = rulespec.dporthigh if hasattr(rulespec, 'dporthigh') else 65535
            if hasattr(rulespec, 'sportlow') or hasattr(rulespec, 'dportlow'):
                # set l4match if topo has any of l4 port info
                l4match = True
            else:
                l4match = False
            return l4match, sportlow, sporthigh, dportlow, dporthigh

        def __get_l3_proto_from_rule(rulespec):
            proto = 0
            if hasattr(rulespec, 'protocol'):
                protocol = rulespec.protocol
                proto = socket.getprotobyname(protocol)
            return proto

        def __get_l3_pfx_from_rule(af, rulespec):
            prefix = None
            if af == utils.IP_VERSION_4:
                if hasattr(rulespec, 'v4pfx'):
                    prefix = rulespec.v4pfx
            else:
                if hasattr(rulespec, 'v6pfx'):
                    prefix = rulespec.v6pfx
            if prefix is not None:
                prefix = ipaddress.ip_network(prefix.replace('\\', '/'))
            return prefix

        def __get_l3_rule(af, rulespec):
            proto = __get_l3_proto_from_rule(rulespec)
            prefix = __get_l3_pfx_from_rule(af, rulespec)
            l3match = False if prefix is None else True
            return l3match, proto, prefix

        def __get_rules(af, policyspec):
            rules = []
            if not hasattr(policyspec, 'rule'):
                return rules
            for rulespec in policyspec.rule:
                stateful = rulespec.stateful
                l4match, sportlow, sporthigh, dportlow, dporthigh = __get_l4_rule(af, rulespec)
                l3match, proto, prefix = __get_l3_rule(af, rulespec)
                rule = RuleObject(stateful, l3match, proto, prefix, l4match, sportlow, sporthigh, dportlow, dporthigh)
                rules.append(rule)
            return rules

        def __add_v4policy(direction, v4rules, policytype, overlaptype):
            obj = PolicyObject(parent, utils.IP_VERSION_4, direction, v4rules, policytype, overlaptype)
            if direction == 'ingress':
                self.__v4ingressobjs[vpcid].append(obj)
            else:
                self.__v4egressobjs[vpcid].append(obj)
            self.__objs.append(obj)

        def __add_v6policy(direction, v6rules, policytype, overlaptype):
            obj = PolicyObject(parent, utils.IP_VERSION_6, direction, v6rules, policytype, overlaptype)
            if direction == 'ingress':
                self.__v6ingressobjs[vpcid].append(obj)
            else:
                self.__v6egressobjs[vpcid].append(obj)
            self.__objs.append(obj)

        def __add_user_specified_policy(policyspec, policytype, overlaptype):
            direction = policyspec.direction
            if __is_v4stack():
                v4rules = __get_rules(utils.IP_VERSION_4, policyspec)
                if direction == 'bidir':
                    #For bidirectional, add policy in both directions
                    policyobj = __add_v4policy('ingress', v4rules, policytype, overlaptype)
                    policyobj = __add_v4policy('egress', v4rules, policytype, overlaptype)
                else:
                    policyobj = __add_v4policy(direction, v4rules, policytype, overlaptype)

            if __is_v6stack():
                v6rules = __get_rules(utils.IP_VERSION_6, policyspec)
                if direction == 'bidir':
                    #For bidirectional, add policy in both directions
                    policyobj = __add_v6policy('ingress', v6rules, policytype, overlaptype)
                    policyobj = __add_v6policy('egress', v6rules, policytype, overlaptype)
                else:
                    policyobj = __add_v6policy(direction, v6rules, policytype, overlaptype)

        def __get_num_subnets(vpc_spec_obj):
            count = 0
            for subnet_obj in vpc_spec_obj.subnet:
                count += subnet_obj.count
            return count

        def __add_default_policies(vpc_spec_obj, policyspec):
            num_subnets = __get_num_subnets(vpc_spec_obj)
            for i in range(num_subnets):
                __add_user_specified_policy(policyspec, policyspec.policytype, None)

        for policy_spec_obj in vpc_spec_obj.policy:
            policy_spec_type = policy_spec_obj.type
            if policy_spec_type == "specific":
                policytype = policy_spec_obj.policytype
                if policytype == 'default':
                    __add_default_policies(vpc_spec_obj, policy_spec_obj)
                else:
                    overlaptype = policy_spec_obj.overlaptype if \
                        hasattr(policy_spec_obj, 'overlaptype') else None
                    __add_user_specified_policy(policy_spec_obj, \
                                                policytype, overlaptype)

        if len(self.__v4ingressobjs[vpcid]) != 0:
            self.__v4ipolicyiter[vpcid] = utils.rrobiniter(self.__v4ingressobjs[vpcid])

        if len(self.__v6ingressobjs[vpcid]) != 0:
            self.__v6ipolicyiter[vpcid] = utils.rrobiniter(self.__v6ingressobjs[vpcid])

        if len(self.__v4egressobjs[vpcid]) != 0:
            self.__v4epolicyiter[vpcid] = utils.rrobiniter(self.__v4egressobjs[vpcid])

        if len(self.__v6egressobjs[vpcid]) != 0:
            self.__v6epolicyiter[vpcid] = utils.rrobiniter(self.__v6egressobjs[vpcid])

        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.POLICY, msgs)
        return

client = PolicyObjectClient()

class PolicyObjectHelper:
    def __init__(self):
        return

    def __get_policyID_from_subnet(self, subnet, af, direction):
        if af == 'IPV6':
            return subnet.IngV6SecurityPolicyId if direction == 'ingress' else subnet.EgV6SecurityPolicyId
        else:
            return subnet.IngV4SecurityPolicyId if direction == 'ingress' else subnet.EgV4SecurityPolicyId

    def __is_lmapping_match(self, policyobj, lobj):
        if lobj.VNIC.SUBNET.VPC.VPCId != policyobj.VPCId:
            return False
        if lobj.AddrFamily == policyobj.AddrFamily:
            return (policyobj.PolicyId == self.__get_policyID_from_subnet(lobj.VNIC.SUBNET, policyobj.AddrFamily, policyobj.Direction))
        return False

    def GetMatchingConfigObjects(self, selectors):
        objs = []
        policyobjs = filter(lambda x: x.IsFilterMatch(selectors), client.Objects())
        for policyObj in policyobjs:
            for lobj in lmapping.client.Objects():
                if self.__is_lmapping_match(policyObj, lobj):
                    policyObj.l_obj = lobj
                    objs.append(policyObj)
                    break
        return utils.GetFilteredObjects(objs, selectors.maxlimits)

PolicyHelper = PolicyObjectHelper()

def GetMatchingObjects(selectors):
    return PolicyHelper.GetMatchingConfigObjects(selectors)
