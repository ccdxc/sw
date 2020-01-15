#! /usr/bin/python3
import pdb
import ipaddress
import random
import socket


#Following come from dol/infra
import infra.config.base as base
import infra.common.defs as defs
import infra.common.parser as parser

import iota.test.apollo.config.resmgr as resmgr
import iota.test.apollo.config.utils as utils
import iota.test.apollo.config.agent.api as agent_api


import types_pb2 as types_pb2
import policy_pb2 as policy_pb2
#import apollo.config.objects.mapping as mapping
import iota.test.apollo.config.utils as utils

from iota.harness.infra.utils.logger import Logger as logger
from iota.test.apollo.config.store import Store

class L4MatchObject:
    def __init__(self, valid=False,\
                 sportlow=utils.L4PORT_MIN, sporthigh=utils.L4PORT_MAX,\
                 dportlow=utils.L4PORT_MIN, dporthigh=utils.L4PORT_MAX,\
                 icmptype=utils.ICMPTYPE_MIN, icmpcode=utils.ICMPCODE_MIN):
        self.valid = valid
        self.SportLow = sportlow
        self.SportHigh = sporthigh
        self.DportLow = dportlow
        self.DportHigh = dporthigh
        self.IcmpType = icmptype
        self.IcmpCode = icmpcode

    def Show(self):
        if self.valid:
            logger.info("    SrcPortRange:%d - %d"\
                        %(self.SportLow, self.SportHigh))
            logger.info("    DstPortRange:%d - %d"\
                        %(self.DportLow, self.DportHigh))
            logger.info("    Icmp Type:%d Code:%d"\
                        %(self.IcmpType, self.IcmpCode))
        else:
            logger.info("    No L4Match")

class L3MatchObject:
    def __init__(self, valid=False, proto=utils.L3PROTO_MIN,\
                 srcpfx=None, dstpfx=None,\
                 srciplow=None, srciphigh=None, dstiplow=None,\
                 dstiphigh=None, srctag=None, dsttag=None,\
                 srctype=utils.L3MatchType.PFX, dsttype=utils.L3MatchType.PFX):
        self.valid = valid
        self.Proto = proto
        self.SrcType = srctype
        self.DstType = dsttype
        self.SrcPrefix = srcpfx
        self.DstPrefix = dstpfx
        self.SrcIPLow = srciplow
        self.SrcIPHigh = srciphigh
        self.DstIPLow = dstiplow
        self.DstIPHigh = dstiphigh
        self.SrcTag = srctag
        self.DstTag = dsttag

    def Show(self):
        def __get_tag_id(tagObj):
            return tagObj.TagId if tagObj else None

        if self.valid:
            logger.info("    Proto:%s(%d)"\
                        %(utils.GetIPProtoName(self.Proto), self.Proto))
            logger.info("    SrcType:%s DstType:%s"\
                        %(self.SrcType, self.DstType))
            logger.info("    SrcPrefix:%s DstPrefix:%s"\
                        %(self.SrcPrefix, self.DstPrefix))
            logger.info("    SrcIPLow:%s SrcIPHigh:%s"\
                        %(self.SrcIPLow, self.SrcIPHigh))
            logger.info("    DstIPLow:%s DstIPHigh:%s"\
                        %(self.DstIPLow, self.DstIPHigh))
            logger.info("    SrcTag:%s DstTag:%s"\
                        %(__get_tag_id(self.SrcTag), __get_tag_id(self.DstTag)))
        else:
            logger.info("    No L3Match")


class RuleObject:
    def __init__(self, l3match, l4match, priority=0, action=types_pb2.SECURITY_RULE_ACTION_ALLOW, stateful=False):
        self.Stateful = stateful
        self.L3Match = l3match
        self.L4Match = l4match
        self.Priority = priority
        self.Action = action

    def Show(self):
        def __get_action_str(action):
            if action == types_pb2.SECURITY_RULE_ACTION_ALLOW:
                return "allow"
            return "deny"

        logger.info(" -- Stateful:%s Priority:%d Action:%s"\
                    %(self.Stateful, self.Priority, __get_action_str(self.Action)))
        self.L3Match.Show()
        self.L4Match.Show()

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
        proto = 0
        specrule = spec.Rules.add()
        specrule.Stateful = rule.Stateful
        specrule.Priority = rule.Priority
        specrule.Action = rule.Action
        l3match = rule.L3Match
        if l3match and l3match.valid:
            proto = l3match.Proto
            specrule.Match.L3Match.Protocol = proto
            if l3match.SrcIPLow and l3match.SrcIPHigh:
                utils.GetRpcIPRange(l3match.SrcIPLow, l3match.SrcIPHigh, specrule.Match.L3Match.SrcRange)
            if l3match.DstIPLow and l3match.DstIPHigh:
                utils.GetRpcIPRange(l3match.DstIPLow, l3match.DstIPHigh, specrule.Match.L3Match.DstRange)
            if l3match.SrcTag:
                specrule.Match.L3Match.SrcTag = l3match.SrcTag.TagId
            if l3match.DstTag:
                specrule.Match.L3Match.DstTag = l3match.DstTag.TagId
            if l3match.SrcPrefix is not None:
                utils.GetRpcIPPrefix(l3match.SrcPrefix, specrule.Match.L3Match.SrcPrefix)
            if l3match.DstPrefix is not None:
                utils.GetRpcIPPrefix(l3match.DstPrefix, specrule.Match.L3Match.DstPrefix)
        l4match = rule.L4Match
        if l4match and l4match.valid:
            if utils.IsICMPProtocol(proto):
                specrule.Match.L4Match.TypeCode.type = l4match.IcmpType
                specrule.Match.L4Match.TypeCode.code = l4match.IcmpCode
            else:
                specrule.Match.L4Match.Ports.SrcPortRange.PortLow = l4match.SportLow
                specrule.Match.L4Match.Ports.SrcPortRange.PortHigh = l4match.SportHigh
                specrule.Match.L4Match.Ports.DstPortRange.PortLow = l4match.DportLow
                specrule.Match.L4Match.Ports.DstPortRange.PortHigh = l4match.DportHigh

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
        obj.hostport = 1
        obj.switchport = 2
        obj.devicecfg = Store.GetDevice()
        # select a random rule for this testcase
        obj.tc_rule = self.__get_non_default_random_rule()
        return

class PolicyObjectClient:
    def __init__(self, devices):
        self.__objs = []
        self.__v4ingressobjs = {}
        self.__v6ingressobjs = {}
        self.__v4egressobjs = {}
        self.__v6egressobjs = {}
        self.__v4ipolicyiter = {}
        self.__v6ipolicyiter = {}
        self.__v4epolicyiter = {}
        self.__v6epolicyiter = {}
        self.devices = devices
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
        isV4Stack = True if ((stack == "dual") or (stack == 'ipv4')) else False
        isV6Stack = True if ((stack == "dual") or (stack == 'ipv6')) else False
        self.__v4ingressobjs[vpcid] = []
        self.__v6ingressobjs[vpcid] = []
        self.__v4egressobjs[vpcid] = []
        self.__v6egressobjs[vpcid] = []
        self.__v4ipolicyiter[vpcid] = None
        self.__v6ipolicyiter[vpcid] = None
        self.__v4epolicyiter[vpcid] = None
        self.__v6epolicyiter[vpcid] = None

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
            if isV4Stack:
                v4rules = __get_rules(utils.IP_VERSION_4, policyspec)
                if direction == 'bidir':
                    #For bidirectional, add policy in both directions
                    policyobj = __add_v4policy('ingress', v4rules, policytype, overlaptype)
                    policyobj = __add_v4policy('egress', v4rules, policytype, overlaptype)
                else:
                    policyobj = __add_v4policy(direction, v4rules, policytype, overlaptype)

            if isV6Stack:
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
        for device in self.devices:
            logger.info("Creating Policy objects for device :", device)
            device.client.Create(agent_api.ObjectTypes.POLICY, msgs)

        return



client = None
def NewPolicyObjectClient(devices):
    global client
    client = PolicyObjectClient(devices)

#client = PolicyObjectClient()

class PolicyObjectHelper:
    def __init__(self):
        return

    def __get_policyID_from_subnet(self, subnet, af, direction):
        if af == 'IPV6':
            return subnet.IngV6SecurityPolicyId if direction == 'ingress' else subnet.EgV6SecurityPolicyId
        else:
            return subnet.IngV4SecurityPolicyId if direction == 'ingress' else subnet.EgV4SecurityPolicyId

    def __is_mapping_match(self, policyobj, lobj):
        if lobj.VNIC.SUBNET.VPC.VPCId != policyobj.VPCId:
            return False
        if lobj.AddrFamily == policyobj.AddrFamily:
            return (policyobj.PolicyId == self.__get_policyID_from_subnet(lobj.VNIC.SUBNET, policyobj.AddrFamily, policyobj.Direction))
        return False

    def GetMatchingConfigObjects(self, selectors):
        objs = []
        policyobjs = filter(lambda x: x.IsFilterMatch(selectors), client.Objects())
        for policyObj in policyobjs:
            for lobj in mapping.client.Objects():
                if self.__is_mapping_match(policyObj, lobj):
                    policyObj.l_obj = lobj
                    objs.append(policyObj)
                    break
        return utils.GetFilteredObjects(objs, selectors.maxlimits)

PolicyHelper = PolicyObjectHelper()

def GetMatchingObjects(selectors):
    return PolicyHelper.GetMatchingConfigObjects(selectors)
