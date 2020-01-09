#! /usr/bin/python3
import pdb
import copy
import enum
import ipaddress
import random

from infra.common.logging import logger

from apollo.config.store import EzAccessStore

import apollo.config.agent.api as api
import apollo.config.resmgr as resmgr
import apollo.config.objects.base as base
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.tag as tag
import apollo.config.utils as utils
import apollo.config.topo as topo

import policy_pb2 as policy_pb2
import types_pb2 as types_pb2

class RulePriority(enum.IntEnum):
    MIN = 0
    MAX = 1023

class SupportedIPProtos(enum.IntEnum):
    TCP = 6
    UDP = 17

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
                 srctype=topo.L3MatchType.PFX, dsttype=topo.L3MatchType.PFX):
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
    def __init__(self, l3match, l4match, priority=0, action=policy_pb2.SECURITY_RULE_ACTION_ALLOW, stateful=False):
        self.Stateful = stateful
        self.L3Match = copy.deepcopy(l3match)
        self.L4Match = copy.deepcopy(l4match)
        self.Priority = priority
        self.Action = action

    def Show(self):
        def __get_action_str(action):
            if action == policy_pb2.SECURITY_RULE_ACTION_ALLOW:
                return "allow"
            return "deny"

        logger.info(" -- Stateful:%s Priority:%d Action:%s"\
                    %(self.Stateful, self.Priority, __get_action_str(self.Action)))
        self.L3Match.Show()
        self.L4Match.Show()

class PolicyObject(base.ConfigObjectBase):
    def __init__(self, vpcid, af, direction, rules, policytype, overlaptype, level='subnet'):
        super().__init__(api.ObjectTypes.POLICY)
        ################# PUBLIC ATTRIBUTES OF POLICY OBJECT #####################
        self.VPCId = vpcid
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
        self.Level = level
        self.OverlapType = overlaptype
        self.rules = copy.deepcopy(rules)
        self.DeriveOperInfo()
        self.Show()
        return

    def __repr__(self):
        return "PolicyID:%d" % (self.PolicyId)

    def Show(self):
        logger.info("Policy Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Level:%s" % self.Level)
        logger.info("- Vpc%d" % self.VPCId)
        logger.info("- Direction:%s|AF:%s" % (self.Direction, self.AddrFamily))
        logger.info("- PolicyType:%s" % self.PolicyType)
        logger.info("- OverlapType:%s" % self.OverlapType)
        logger.info("- Number of rules:%d" % len(self.rules))
        for rule in self.rules:
            rule.Show()
        return

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.policy.filters)

    def CopyObject(self):
        clone = copy.copy(self)
        clone.rules = self.rules
        self.rules = copy.deepcopy(clone.rules)
        return clone

    def UpdateAttributes(self):
        for rule in self.rules:
            if rule.Action == policy_pb2.SECURITY_RULE_ACTION_ALLOW:
                rule.Action = policy_pb2.SECURITY_RULE_ACTION_DENY
            else:
                rule.Action = policy_pb2.SECURITY_RULE_ACTION_ALLOW
        return

    def RollbackAttributes(self):
        self.rules = self.GetPrecedent().rules
        return

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

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.PolicyId)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.PolicyId
        spec.Direction = utils.GetRpcDirection(self.Direction)
        spec.AddrFamily = utils.GetRpcIPAddrFamily(self.AddrFamily)
        for rule in self.rules:
            self.FillRuleSpec(spec, rule)
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.PolicyId:
            return False
        if spec.Direction != utils.GetRpcDirection(self.Direction):
            return False
        if spec.AddrFamily != utils.GetRpcIPAddrFamily(self.AddrFamily):
            return False
        return True

    def __get_random_rule(self):
        rules = self.rules
        numrules = len(rules)
        if numrules == 0:
            return None
        # TODO: Disabling randomness for debug - remove once rfc job is stable
        # return random.choice(rules)
        return rules[0]

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

    def IsIngressPolicy(self):
        return self.Direction == 'ingress'

    def IsEgressPolicy(self):
        return self.Direction == 'egress'

    def SetupTestcaseConfig(self, obj):
        obj.localmapping = self.l_obj
        obj.policy = self
        obj.route = self.l_obj.VNIC.SUBNET.V6RouteTable if self.AddrFamily == 'IPV6' else self.l_obj.VNIC.SUBNET.V4RouteTable
        obj.tunnel = obj.route.TUNNEL
        obj.hostport = EzAccessStore.GetHostPort()
        obj.switchport = EzAccessStore.GetSwitchPort()
        obj.devicecfg = EzAccessStore.GetDevice()
        # select a random rule for this testcase
        if utils.IsPipelineApollo():
            # TODO: move apollo also to random rule
            obj.tc_rule = self.__get_non_default_random_rule()
        else:
            obj.tc_rule = self.__get_random_rule()
        utils.DumpTestcaseConfig(obj)
        return

class PolicyObjectClient(base.ConfigClientBase):
    def __init__(self):
        def __isObjSupported():
            return True
        super().__init__(api.ObjectTypes.POLICY, resmgr.MAX_POLICY)
        self.__v4ingressobjs = {}
        self.__v6ingressobjs = {}
        self.__v4egressobjs = {}
        self.__v6egressobjs = {}
        self.__v4ipolicyiter = {}
        self.__v6ipolicyiter = {}
        self.__v4epolicyiter = {}
        self.__v6epolicyiter = {}
        self.__supported = __isObjSupported()
        return

    def PdsctlRead(self):
        # pdsctl show not supported for policy
        return

    def GetPolicyObject(self, policyid):
        return self.GetObjectByKey(policyid)

    def ModifyPolicyRules(self, policyid, subnetobj):
        if utils.IsPipelineApollo():
            # apollo does not support both sip & dip match in rules
            return

        def __is_default_l3_attr(matchtype=topo.L3MatchType.PFX, pfx=None,\
                                 iplow=None, iphigh=None, tag=None):
            if matchtype == topo.L3MatchType.PFX:
                return utils.isDefaultRoute(pfx)
            elif matchtype == topo.L3MatchType.PFXRANGE:
                return utils.isDefaultAddrRange(iplow, iphigh)
            elif matchtype == topo.L3MatchType.TAG:
                return utils.isTagWithDefaultRoute(tag)
            return False

        def __get_l3_attr(l3matchtype, newpfx, newtag):
            pfx = None
            startaddr = None
            endaddr = None
            tag = None
            if l3matchtype == topo.L3MatchType.PFX:
                pfx = newpfx
            elif l3matchtype == topo.L3MatchType.PFXRANGE:
                startaddr = newpfx.network_address
                endaddr = startaddr + newpfx.num_addresses - 1
            elif l3matchtype == topo.L3MatchType.TAG:
                tag = newtag
            return pfx, startaddr, endaddr, tag

        def __modify_l3_match(direction, l3matchobj, subnetpfx, subnettag):
            if not l3matchobj.valid:
                # nothing to do in case of wildcard
                return
            if (direction == 'egress'):
                if __is_default_l3_attr(l3matchobj.SrcType, l3matchobj.SrcPrefix, l3matchobj.SrcIPLow, l3matchobj.SrcIPHigh, l3matchobj.SrcTag):
                    # no need of modification if it is already a default route
                    return
                l3matchobj.SrcPrefix, l3matchobj.SrcIPLow, l3matchobj.SrcIPHigh, l3matchobj.SrcTag = __get_l3_attr(l3matchobj.SrcType, subnetpfx, subnettag)
            else:
                if __is_default_l3_attr(l3matchobj.DstType, l3matchobj.DstPrefix, l3matchobj.DstIPLow, l3matchobj.DstIPHigh, l3matchobj.DstTag):
                    return
                l3matchobj.DstPrefix, l3matchobj.DstIPLow, l3matchobj.DstIPHigh, l3matchobj.DstTag = __get_l3_attr(l3matchobj.DstType, subnetpfx, subnettag)
            return

        policy = self.GetPolicyObject(policyid)
        direction = policy.Direction
        af = utils.GetIPVersion(policy.AddrFamily)
        subnetpfx = subnetobj.IPPrefix[1] if af == utils.IP_VERSION_4 else subnetobj.IPPrefix[0]
        subnettag = None
        if utils.IsTagSupported():
            subnettag = tag.client.GetCreateTag(policy.VPCId, af, subnetpfx)
        for rule in policy.rules:
            __modify_l3_match(direction, rule.L3Match, subnetpfx, subnettag)
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

    def Add_V4Policy(self, vpcid, direction, v4rules, policytype, overlaptype, level='subnet'):
        obj = PolicyObject(vpcid, utils.IP_VERSION_4, direction, v4rules, policytype, overlaptype, level)
        if direction == 'ingress':
            self.__v4ingressobjs[vpcid].append(obj)
        else:
            self.__v4egressobjs[vpcid].append(obj)
        self.Objs.update({obj.PolicyId: obj})
        return obj.PolicyId

    def Add_V6Policy(self, vpcid, direction, v6rules, policytype, overlaptype, level='subnet'):
        obj = PolicyObject(vpcid, utils.IP_VERSION_6, direction, v6rules, policytype, overlaptype, level)
        if direction == 'ingress':
            self.__v6ingressobjs[vpcid].append(obj)
        else:
            self.__v6egressobjs[vpcid].append(obj)
        self.Objs.update({obj.PolicyId: obj})
        return obj.PolicyId

    def Generate_Allow_All_Rules(self, spfx, dpfx):
        rules = []
        l4match = L4MatchObject(True)
        for proto in SupportedIPProtos:
            l3match = L3MatchObject(True, proto, srcpfx=spfx, dstpfx=dpfx)
            rule = RuleObject(l3match, l4match)
            rules.append(rule)
        return rules

    def Generate_random_rules_from_nacl(self, naclobj, subnetpfx, af):
        # TODO: make it random. Add allow all with default pfx for now
        if af == utils.IP_VERSION_6:
            pfx = utils.IPV6_DEFAULT_ROUTE
        else:
            pfx = utils.IPV4_DEFAULT_ROUTE
        return self.Generate_Allow_All_Rules(pfx, pfx)

    def GenerateVnicPolicies(self, numPolicy, subnet, direction, is_v6=False):
        if not self.__supported:
            return

        vpcid = subnet.VPC.VPCId
        if is_v6:
            af = utils.IP_VERSION_6
            add_policy = self.Add_V6Policy
        else:
            af = utils.IP_VERSION_4
            add_policy = self.Add_V4Policy
        subnetpfx = subnet.IPPrefix[1] if af == utils.IP_VERSION_4 else subnet.IPPrefix[0]
        naclid = subnet.GetNaclId(direction, af)
        naclobj = self.GetPolicyObject(naclid)

        vnic_policies = []
        for i in range(numPolicy):
            overlaptype = naclobj.OverlapType
            policytype = naclobj.PolicyType
            rules = self.Generate_random_rules_from_nacl(naclobj, subnetpfx, af)
            policyid = add_policy(vpcid, direction, rules, policytype, overlaptype, 'vnic')
            vnic_policies.append(policyid)
        return vnic_policies

    def GenerateObjects(self, parent, vpc_spec_obj):
        if not self.__supported:
            return

        vpcid = parent.VPCId
        isV4Stack = utils.IsV4Stack(parent.Stack)
        isV6Stack = utils.IsV6Stack(parent.Stack)
        self.__v4ingressobjs[vpcid] = []
        self.__v6ingressobjs[vpcid] = []
        self.__v4egressobjs[vpcid] = []
        self.__v6egressobjs[vpcid] = []
        self.__v4ipolicyiter[vpcid] = None
        self.__v6ipolicyiter[vpcid] = None
        self.__v4epolicyiter[vpcid] = None
        self.__v6epolicyiter[vpcid] = None

        def __get_l4_rule(af, rulespec):
            sportlow = getattr(rulespec, 'sportlow', utils.L4PORT_MIN)
            dportlow = getattr(rulespec, 'dportlow', utils.L4PORT_MIN)
            sporthigh = getattr(rulespec, 'sporthigh', utils.L4PORT_MAX)
            dporthigh = getattr(rulespec, 'dporthigh', utils.L4PORT_MAX)
            icmptype = getattr(rulespec, 'icmptype', utils.ICMPTYPE_MIN)
            icmpcode = getattr(rulespec, 'icmpcode', utils.ICMPCODE_MIN)
            l4match = any([sportlow, sporthigh, dportlow, dporthigh, icmptype, icmpcode])
            obj = L4MatchObject(l4match, sportlow, sporthigh, dportlow, dporthigh, icmptype, icmpcode)
            return obj

        def __get_l3_proto_from_rule(af, rulespec):
            proto = getattr(rulespec, 'protocol', utils.L3PROTO_MIN)
            if proto:
                if proto == "icmp" and af == utils.IP_VERSION_6:
                    proto = "ipv6-" + proto
                proto = utils.GetIPProtoByName(proto)
            return proto

        def __get_l3_match_type(rulespec, attr):
            matchtype = topo.L3MatchType.PFX
            if hasattr(rulespec, attr):
                matchval = getattr(rulespec, attr)
                if matchval == "pfxrange":
                    matchtype = topo.L3MatchType.PFXRANGE
                elif matchval == "tag":
                    matchtype = topo.L3MatchType.TAG
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

        def __get_l3_tag_from_rule(af, rulespec, srctype, dsttype):
            srctag = None
            dsttag = None
            if srctype != topo.L3MatchType.TAG and dsttype != topo.L3MatchType.TAG:
                # no need to create tag if none of src,dst is of tag type
                return srctag, dsttag
            #get pfx from rule and configure tag on the fly to tagtable of af in this vpc
            pfx = __get_pfx_from_rule(af, rulespec, 'pfx')
            tagObj = tag.client.GetCreateTag(vpcid, af, pfx)
            if srctype == topo.L3MatchType.TAG:
                srctag = tagObj
            if dsttype == topo.L3MatchType.TAG:
                dsttag = tagObj
            return srctag, dsttag

        def __get_l3_rule(af, rulespec):
            def __is_l3_matchtype_supported(matchtype):
                if matchtype == topo.L3MatchType.TAG:
                    return utils.IsTagSupported()
                elif matchtype == topo.L3MatchType.PFXRANGE:
                    return utils.IsPfxRangeSupported()
                elif matchtype == topo.L3MatchType.PFX:
                    return True
                return False

            def __is_proto_supported(proto):
                if utils.IsICMPProtocol(proto):
                    if utils.IsPipelineApollo():
                        # Apollo does NOT support icmp proto
                        return False
                return True

            def __convert_tag2pfx(matchtype):
                return topo.L3MatchType.PFX if matchtype is topo.L3MatchType.TAG else matchtype

            proto = __get_l3_proto_from_rule(af, rulespec)
            if not __is_proto_supported(proto):
                logger.error("policy rule do not support", proto)
                return None
            srctype, dsttype = __get_l3_match_type_from_rule(rulespec)
            if not utils.IsTagSupported():
                srctype = __convert_tag2pfx(srctype)
                dsttype = __convert_tag2pfx(dsttype)
            if (not __is_l3_matchtype_supported(srctype)) or (not __is_l3_matchtype_supported(dsttype)):
                logger.error("Unsupported l3match type ", srctype, dsttype)
                return None
            srcpfx, dstpfx = __get_l3_pfx_from_rule(af, rulespec)
            srciplow, srciphigh, dstiplow, dstiphigh = __get_l3_pfx_range_from_rule(af, rulespec)
            srctag, dsttag = __get_l3_tag_from_rule(af, rulespec, srctype, dsttype)
            l3match = any([proto, srcpfx, dstpfx, srciplow, srciphigh, dstiplow, dstiphigh, srctag, dsttag])
            obj = L3MatchObject(l3match, proto, srcpfx, dstpfx, srciplow, srciphigh, dstiplow, dstiphigh, srctag, dsttag, srctype, dsttype)
            return obj

        def __get_rule_action(rulespec):
            actionVal = getattr(rulespec, 'action', None)
            if actionVal == "deny":
                action = policy_pb2.SECURITY_RULE_ACTION_DENY
            elif actionVal == "random":
                action = random.choice([policy_pb2.SECURITY_RULE_ACTION_DENY, policy_pb2.SECURITY_RULE_ACTION_ALLOW])
            else:
                action = policy_pb2.SECURITY_RULE_ACTION_ALLOW
            return action

        def __get_valid_priority(prio):
            if prio < RulePriority.MIN:
                return RulePriority.MIN
            elif prio > RulePriority.MAX:
                return RulePriority.MAX
            else:
                return prio

        def __get_rule_priority(rulespec, basePrio=0):
            prio = getattr(rulespec, 'priority', -3)
            if prio == -1:
                # increasing priority
                basePrio = basePrio + 1
                priority = basePrio
            elif prio == -2:
                # decreasing priority
                basePrio = basePrio - 1
                priority = basePrio
            elif prio == -3:
                # random priority
                priority = random.randint(RulePriority.MIN, RulePriority.MAX)
            else:
                # configured priority
                priority = prio
            return __get_valid_priority(priority), basePrio

        def __get_l3_rules_from_rule_base(af, rulespec, overlaptype):

            def __get_first_subnet(ippfx, pfxlen):
                for ip in ippfx.subnets(new_prefix=pfxlen):
                    return (ip)
                return

            def __get_user_specified_pfxs(pfxspec):
                pfxlist = []
                if pfxspec:
                    for pfx in pfxspec:
                        pfxlist.append(ipaddress.ip_network(pfx.replace('\\', '/')))
                return pfxlist

            def __get_adjacent_pfxs(basepfx, count):
                pfxlist = []
                c = 1
                pfxlist.append(ipaddress.ip_network(basepfx))
                while c < count:
                    pfxlist.append(utils.GetNextSubnet(pfxlist[c-1]))
                    c += 1
                return pfxlist

            def __get_overlap_pfxs(basepfx, base, count):
                # for overlap, add user specified base prefix with original prefixlen
                pfxlist = [ basepfx ]
                pfxlist.extend(__get_adjacent_pfxs(base, count))
                return pfxlist

            if af == utils.IP_VERSION_4:
                pfxcount = getattr(rulespec, 'nv4prefixes', 0)
                pfxlen = getattr(rulespec, 'v4prefixlen', 24)
                basepfx = ipaddress.ip_network(rulespec.v4base.replace('\\', '/'))
                user_specified_routes = __get_user_specified_pfxs(getattr(rulespec, 'v4prefixes', None))
            else:
                pfxcount = getattr(rulespec, 'nv6prefixes', 0)
                pfxlen = getattr(rulespec, 'v6prefixlen', 120)
                basepfx = ipaddress.ip_network(rulespec.v6base.replace('\\', '/'))
                user_specified_routes = __get_user_specified_pfxs(getattr(rulespec, 'v6prefixes', None))

            newbase = __get_first_subnet(basepfx, pfxlen)
            pfxlist = user_specified_routes
            if 'adjacent' in overlaptype:
                pfxlist +=  __get_adjacent_pfxs(newbase, pfxcount)
            elif 'overlap' in overlaptype:
                pfxlist += __get_overlap_pfxs(basepfx, newbase, pfxcount-1)

            l3rules = []
            proto = __get_l3_proto_from_rule(af, rulespec)
            for pfx in pfxlist:
                obj = L3MatchObject(True, proto, pfx, pfx)
                l3rules.append(obj)
            return l3rules

        def __get_rules(af, policyspec, overlaptype):
            rules = []
            if not hasattr(policyspec, 'rule'):
                return rules
            policy_spec_type = policyspec.type
            for rulespec in policyspec.rule:
                stateful = getattr(rulespec, 'stateful', False)
                l4match = __get_l4_rule(af, rulespec)
                if policy_spec_type == 'base':
                    prioritybase = getattr(rulespec, 'prioritybase', 0)
                    objs = __get_l3_rules_from_rule_base(af, rulespec, overlaptype)
                    if len(objs) == 0:
                        return None
                    for l3match in objs:
                        priority, prioritybase = __get_rule_priority(rulespec, prioritybase)
                        action = __get_rule_action(rulespec)
                        rule = RuleObject(l3match, l4match, priority, action, stateful)
                        rules.append(rule)
                else:
                    l3match = __get_l3_rule(af, rulespec)
                    if l3match is None:
                        # L3 match is mandatory for a rule
                        return None
                    priority, prioritybase = __get_rule_priority(rulespec)
                    action = __get_rule_action(rulespec)
                    rule = RuleObject(l3match, l4match, priority, action, stateful)
                    rules.append(rule)
            return rules

        def __add_v4policy(direction, v4rules, policytype, overlaptype):
            obj = PolicyObject(vpcid, utils.IP_VERSION_4, direction, v4rules, policytype, overlaptype)
            if direction == 'ingress':
                self.__v4ingressobjs[vpcid].append(obj)
            else:
                self.__v4egressobjs[vpcid].append(obj)
            self.Objs.update({obj.PolicyId: obj})

        def __add_v6policy(direction, v6rules, policytype, overlaptype):
            obj = PolicyObject(vpcid, utils.IP_VERSION_6, direction, v6rules, policytype, overlaptype)
            if direction == 'ingress':
                self.__v6ingressobjs[vpcid].append(obj)
            else:
                self.__v6egressobjs[vpcid].append(obj)
            self.Objs.update({obj.PolicyId: obj})

        def __add_user_specified_policy(policyspec, policytype, overlaptype):
            direction = policyspec.direction
            if isV4Stack:
                v4rules = __get_rules(utils.IP_VERSION_4, policyspec, overlaptype)
                if v4rules is None:
                    return
                if direction == 'bidir':
                    #For bidirectional, add policy in both directions
                    policyobj = __add_v4policy('ingress', v4rules, policytype, overlaptype)
                    policyobj = __add_v4policy('egress', v4rules, policytype, overlaptype)
                else:
                    policyobj = __add_v4policy(direction, v4rules, policytype, overlaptype)

            if isV6Stack:
                v6rules = __get_rules(utils.IP_VERSION_6, policyspec, overlaptype)
                if v6rules is None:
                    return
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
            policytype = policy_spec_obj.policytype
            if policy_spec_type == "specific":
                if policytype == 'default':
                    __add_default_policies(vpc_spec_obj, policy_spec_obj)
                else:
                    overlaptype = getattr(policy_spec_obj, 'overlaptype', None)
                    __add_user_specified_policy(policy_spec_obj, \
                                                policytype, overlaptype)
            elif policy_spec_type == "base":
                overlaptype = getattr(policy_spec_obj, 'overlaptype', 'none')
                __add_user_specified_policy(policy_spec_obj, policytype, overlaptype)

        if len(self.__v4ingressobjs[vpcid]) != 0:
            self.__v4ipolicyiter[vpcid] = utils.rrobiniter(self.__v4ingressobjs[vpcid])

        if len(self.__v6ingressobjs[vpcid]) != 0:
            self.__v6ipolicyiter[vpcid] = utils.rrobiniter(self.__v6ingressobjs[vpcid])

        if len(self.__v4egressobjs[vpcid]) != 0:
            self.__v4epolicyiter[vpcid] = utils.rrobiniter(self.__v4egressobjs[vpcid])

        if len(self.__v6egressobjs[vpcid]) != 0:
            self.__v6epolicyiter[vpcid] = utils.rrobiniter(self.__v6egressobjs[vpcid])

        return

client = PolicyObjectClient()

class PolicyObjectHelper:
    def __init__(self):
        return

    def __get_policyID_from_subnet(self, subnet, af, direction):
        if af == 'IPV6':
            return subnet.IngV6SecurityPolicyIds[0] if direction == 'ingress' else subnet.EgV6SecurityPolicyIds[0]
        else:
            return subnet.IngV4SecurityPolicyIds[0] if direction == 'ingress' else subnet.EgV4SecurityPolicyIds[0]

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
