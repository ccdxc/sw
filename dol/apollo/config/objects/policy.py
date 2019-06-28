#! /usr/bin/python3
import pdb
import enum
import copy
import ipaddress
import random
import socket

from infra.common.logging import logger
import infra.config.base as base

from apollo.config.store import Store
import apollo.config.agent.api as api
import apollo.config.resmgr as resmgr
import apollo.config.objects.lmapping as lmapping
import apollo.config.utils as utils

import policy_pb2 as policy_pb2
import types_pb2 as types_pb2

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
            logger.info("- SrcPortRange:%d - %d DstPortRange:%d - %d" %(self.SportLow, self.SportHigh, self.DportLow, self.DportHigh))
        else:
            logger.info("- No L4Match")

class L3MatchObject:
    def __init__(self, valid=False, proto=0, srcpfx=None, dstpfx=None,\
                 srciplow=None, srciphigh=None, dstiplow=None,\
                 dstiphigh=None, srctag=0, dsttag=0,\
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
        if self.valid:
            logger.info("- Proto:%d SrcType:%d DstType:%d" %(self.Proto, self.SrcType, self.DstType))
            logger.info("- SrcPrefix:%s DstPrefix:%s" %(self.SrcPrefix, self.DstPrefix))
            logger.info("- SrcIPLow:%s SrcIPHigh:%s" %(self.SrcIPLow, self.SrcIPHigh))
            logger.info("- DstIPLow:%s DstIPHigh:%s" %(self.DstIPLow, self.DstIPHigh))
            logger.info("- SrcTag:%s DstTag:%s" %(self.SrcTag, self.DstTag))
        else:
            logger.info("- No L3Match")

class RuleObject:
    def __init__(self, stateful, l3match, l4match, priority=0, action=policy_pb2.SECURITY_RULE_ACTION_ALLOW):
        #TODO: introduce rule no
        self.Stateful = stateful
        self.L3Match = l3match
        self.L4Match = l4match
        self.Priority = priority
        self.Action = action

    def Show(self):
        logger.info("- Stateful:%s Priority:%d Action:%d" %(self.Stateful, self.Priority, self.Action))
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
        self.rules = copy.deepcopy(rules)
        self.Show()
        return

    def __repr__(self):
        return "PolicyID:%d" % (self.PolicyId)

    def FillRuleSpec(self, spec, rule):
        specrule = spec.Rules.add()
        specrule.Stateful = rule.Stateful
        specrule.Priority = rule.Priority
        specrule.Action = rule.Action
        l4match = rule.L4Match
        if l4match and l4match.valid:
            specrule.Match.L4Match.Ports.SrcPortRange.PortLow = l4match.SportLow
            specrule.Match.L4Match.Ports.SrcPortRange.PortHigh = l4match.SportHigh
            specrule.Match.L4Match.Ports.DstPortRange.PortLow = l4match.DportLow
            specrule.Match.L4Match.Ports.DstPortRange.PortHigh = l4match.DportHigh
        l3match = rule.L3Match
        if l3match and l3match.valid:
            specrule.Match.L3Match.Protocol = l3match.Proto
            if l3match.SrcIPLow and l3match.SrcIPHigh:
                utils.GetRpcIPRange(l3match.SrcIPLow, l3match.SrcIPHigh, specrule.Match.L3Match.SrcRange)
            if l3match.DstIPLow and l3match.DstIPHigh:
                utils.GetRpcIPRange(l3match.DstIPLow, l3match.DstIPHigh, specrule.Match.L3Match.DstRange)
            specrule.Match.L3Match.SrcTag = l3match.SrcTag
            specrule.Match.L3Match.DstTag = l3match.DstTag
            if l3match.SrcPrefix is not None:
                utils.GetRpcIPPrefix(l3match.SrcPrefix, specrule.Match.L3Match.SrcPrefix)
            if l3match.DstPrefix is not None:
                utils.GetRpcIPPrefix(l3match.DstPrefix, specrule.Match.L3Match.DstPrefix)

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

    def __get_random_rule(self):
        rules = self.rules
        numrules = len(rules)
        if numrules == 0:
            return None
        return random.choice(rules)

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
            if self.Direction == types_pb2.RULE_DIR_INGRESS:
                pfx = rules[0].L3Match.SrcPrefix
            else:
                pfx = rules[0].L3Match.DstPrefix
            if not utils.isDefaultRoute(pfx):
                rule = rules[0]
            return rule
        while True:
            rule = random.choice(rules)
            if self.Direction == types_pb2.RULE_DIR_INGRESS:
                pfx = rule.L3Match.SrcPrefix
            else:
                pfx = rule.L3Match.DstPrefix
            if not utils.isDefaultRoute(pfx):
                break
        return rule

    def SetupTestcaseConfig(self, obj):
        obj.localmapping = self.l_obj
        obj.policy = self
        obj.route = self.l_obj.VNIC.SUBNET.V6RouteTable if self.AddrFamily == 'IPV6' else self.l_obj.VNIC.SUBNET.V4RouteTable
        obj.tunnel = obj.route.TUNNEL
        obj.hostport = utils.PortTypes.HOST
        obj.switchport = utils.PortTypes.SWITCH
        obj.devicecfg = Store.GetDevice()
        # select a random rule for this testcase
        if utils.IsPipelineArtemis():
            obj.tc_rule = self.__get_random_rule()
        else:
            obj.tc_rule = self.__get_non_default_random_rule()
        return

class PolicyObjectClient:
    def __init__(self):
        self.__objs = dict()
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
        return self.__objs.values()

    def IsValidConfig(self):
        count = len(self.__objs.values())
        if  count > resmgr.MAX_POLICY:
            return False, "Policy count %d exceeds allowed limit of %d" %\
                          (count, resmgr.MAX_POLICY)
        return True, ""

    def GetPolicyObject(self, policyid):
        return self.__objs.get(policyid, None)

    def ModifyPolicyRules(self, policyid, subnetobj):
        if not utils.IsPipelineArtemis():
            return

        def __get_l3_attr(l3matchtype, newpfx):
            pfx = None
            startaddr = None
            endaddr = None
            tag = 0
            if l3matchtype == utils.L3MatchType.PFX:
                pfx = newpfx
            elif l3matchtype == utils.L3MatchType.PFXRANGE:
                startaddr = newpfx.network_address
                endaddr = startaddr + newpfx.num_addresses - 1
            elif l3matchtype == utils.L3MatchType.TAG:
                # TODO: once tag support comes
                tag = configure_tag_(newpfx)
            return pfx, startaddr, endaddr, tag

        def __modify_l3_match(direction, l3matchobj, subnetpfx):
            if not l3matchobj.valid:
                # nothing to do in case of wildcard
                return
            #TODO: return without modification for default pfx so that it can be tested
            if (direction == 'egress'):
                l3matchobj.SrcPrefix, l3matchobj.SrcIPLow, l3matchobj.SrcIPHigh, l3matchobj.SrcTag = __get_l3_attr(l3matchobj.SrcType, subnetpfx)
            else:
                l3matchobj.DstPrefix, l3matchobj.DstIPLow, l3matchobj.DstIPHigh, l3matchobj.DstTag = __get_l3_attr(l3matchobj.DstType, subnetpfx)
            return

        policy = self.GetPolicyObject(policyid)
        direction = policy.Direction
        af = policy.AddrFamily
        subnetpfx = subnetobj.IPPrefix[1] if af == 'IPV4' else subnetobj.IPPrefix[0]
        for rule in policy.rules:
            __modify_l3_match(direction, rule.L3Match, subnetpfx)
        logger.info("Modified Policy")
        policy.Show()
        return

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
        self.__objs = dict()
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
            sportlow = getattr(rulespec, 'sportlow', 0)
            dportlow = getattr(rulespec, 'dportlow', 0)
            sporthigh = getattr(rulespec, 'sporthigh', 65535)
            dporthigh = getattr(rulespec, 'dporthigh', 65535)
            icmptype = getattr(rulespec, 'icmptype', 0)
            icmpcode = getattr(rulespec, 'icmpcode', 0)
            l4match = any([sportlow, sporthigh, dportlow, dporthigh, icmptype, icmpcode])
            obj = L4MatchObject(l4match, sportlow, sporthigh, dportlow, dporthigh, icmptype, icmpcode)
            return obj

        def __get_l3_proto_from_rule(rulespec):
            proto = getattr(rulespec, 'protocol', 0)
            if proto:
                proto = socket.getprotobyname(proto)
            return proto

        def __get_l3_match_type(rulespec, attr):
            matchtype = utils.L3MatchType.PFX
            if hasattr(rulespec, attr):
                matchval = getattr(rulespec, attr)
                if matchval == "pfxrange":
                    matchtype = utils.L3MatchType.PFXRANGE
                elif matchval == "tag":
                    matchtype = utils.L3MatchType.TAG
            return matchtype

        def __get_l3_match_type_from_rule(rulespec):
            srctype = __get_l3_match_type(rulespec, 'srctype')
            dsttype = __get_l3_match_type(rulespec, 'dsttype')
            return srctype, dsttype

        def __get_pfx_from_rule(af, rulespec, attr, ispfx=True):
            prefix = None
            if af == utils.IP_VERSION_4:
                prefix = getattr(rulespec, 'v4' + attr, None)
            else:
                prefix = getattr(rulespec, 'v6' + attr, None)
            if prefix is not None:
                if ispfx:
                    prefix = ipaddress.ip_network(prefix.replace('\\', '/'))
                else:
                    prefix = ipaddress.ip_address(prefix)
            return prefix

        def __get_l3_pfx_from_rule(af, rulespec):
            pfx = __get_pfx_from_rule(af, rulespec, 'pfx')
            srcpfx = pfx
            dstpfx = pfx
            pfx = __get_pfx_from_rule(af, rulespec, 'srcpfx')
            if pfx is not None:
                srcpfx = pfx
            pfx = __get_pfx_from_rule(af, rulespec, 'dstpfx')
            if pfx is not None:
                dstpfx = pfx
            return srcpfx, dstpfx

        def __get_l3_pfx_range_from_rule(af, rulespec):
            srciplow = __get_pfx_from_rule(af, rulespec, 'srciplow', False)
            srciphigh = __get_pfx_from_rule(af, rulespec, 'srciphigh', False)
            dstiplow = __get_pfx_from_rule(af, rulespec, 'dstiplow', False)
            dstiphigh = __get_pfx_from_rule(af, rulespec, 'dstiphigh', False)
            return srciplow, srciphigh, dstiplow, dstiphigh

        def __get_l3_tag_from_rule(af, rulespec):
            srctag = getattr(rulespec, 'srctag', 0)
            dsttag = getattr(rulespec, 'dsttag', 0)
            return srctag, dsttag

        def __get_l3_rule(af, rulespec):
            proto = __get_l3_proto_from_rule(rulespec)
            srctype, dsttype = __get_l3_match_type_from_rule(rulespec)
            srcpfx, dstpfx = __get_l3_pfx_from_rule(af, rulespec)
            srciplow, srciphigh, dstiplow, dstiphigh = __get_l3_pfx_range_from_rule(af, rulespec)
            srctag, dsttag = __get_l3_tag_from_rule(af, rulespec)
            l3match = any([proto, srcpfx, dstpfx, srciplow, srciphigh, dstiplow, dstiphigh, srctag, dsttag])
            obj = L3MatchObject(l3match, proto, srcpfx, dstpfx, srciplow, srciphigh, dstiplow, dstiphigh, srctag, dsttag, srctype, dsttype)
            return obj

        def __get_rule_action(rulespec):
            actionVal = getattr(rulespec, 'action', None)
            if actionVal == "deny":
                action = policy_pb2.SECURITY_RULE_ACTION_DENY
            else:
                action = policy_pb2.SECURITY_RULE_ACTION_ALLOW
            return action

        def __get_rules(af, policyspec):
            rules = []
            if not hasattr(policyspec, 'rule'):
                return rules
            for rulespec in policyspec.rule:
                stateful = getattr(rulespec, 'stateful', False)
                priority = getattr(rulespec, 'priority', 0)
                l4match = __get_l4_rule(af, rulespec)
                l3match = __get_l3_rule(af, rulespec)
                action = __get_rule_action(rulespec)
                rule = RuleObject(stateful, l3match, l4match, priority, action)
                rules.append(rule)
            return rules

        def __add_v4policy(direction, v4rules, policytype, overlaptype):
            obj = PolicyObject(parent, utils.IP_VERSION_4, direction, v4rules, policytype, overlaptype)
            if direction == 'ingress':
                self.__v4ingressobjs[vpcid].append(obj)
            else:
                self.__v4egressobjs[vpcid].append(obj)
            self.__objs.update({obj.PolicyId: obj})

        def __add_v6policy(direction, v6rules, policytype, overlaptype):
            obj = PolicyObject(parent, utils.IP_VERSION_6, direction, v6rules, policytype, overlaptype)
            if direction == 'ingress':
                self.__v6ingressobjs[vpcid].append(obj)
            else:
                self.__v6egressobjs[vpcid].append(obj)
            self.__objs.update({obj.PolicyId: obj})

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
                    overlaptype = getattr(policy_spec_obj, 'overlaptype', None)
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
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs.values()))
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
        return utils.GetFilteredObjects(objs, selectors.maxlimits, False)

PolicyHelper = PolicyObjectHelper()

def GetMatchingObjects(selectors):
    return PolicyHelper.GetMatchingConfigObjects(selectors)
