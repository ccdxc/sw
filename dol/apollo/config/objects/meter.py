#! /usr/bin/python3
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.route as route
import types_pb2 as types_pb2
import meter_pb2 as meter_pb2

from infra.common.logging import logger

class MeterRuleObject(base.ConfigObjectBase):
    def __init__(self, metertype, priority, prefixes,
            bursttype = None, persecburst = 0, numburst = 0):
        self.MeterType = metertype
        self.BurstType = bursttype    # <packet> or <byte> burst
        self.PersecBurst = persecburst
        self.NumBurst = numburst
        self.Priority = priority
        self.Prefixes = prefixes

    def __repr__(self):
        return "RuleType:%s|RulePriority:%d" % (self.MeterType, self.Priority)

    def Show(self):
        res = ""
        for p in self.Prefixes:
            res += str(p) + ', '
        logger.info("- %s" % repr(self))
        logger.info("- %s" % res)

class MeterObject(base.ConfigObjectBase):
    def __init__(self, parent, af, rules):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF METER OBJECT #####################
        self.VPCId = parent.VPCId
        if af == utils.IP_VERSION_6:
            self.MeterId = next(resmgr.V6MeterIdAllocator)
            self.Af = 'IPV6'
        else:
            self.MeterId = next(resmgr.V4MeterIdAllocator)
            self.Af = 'IPV4'
        self.GID('Meter%d'%self.MeterId)
        self.Rules = rules
        self.Show()
        return

    def __repr__(self):
        return "MeterID:%d|VPCId:%d" % (self.MeterId, self.VPCId)

    def FillMeterRulePrefixes(self, rulespec, rule):
        for pfx in rule.Prefixes:
            pfxobj = rulespec.Prefix.add()
            utils.GetRpcIPPrefix(pfx, pfxobj)
        return

    def FillMeterRuleSpec(self, spec, rule):
        ruleobj = spec.rules.add()
        self.FillMeterRulePrefixes(ruleobj, rule)
        #ruleobj.PPSPolicer = 0
        #ruleobj.BPSPolicer = 0
        ruleobj.Priority = rule.Priority
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = meter_pb2.MeterRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.MeterId
        spec.Af = utils.GetRpcIPAddrFamily(self.Af)
        #spec.VPCId = self.VPCId
        for rule in self.Rules:
            self.FillMeterRuleSpec(spec, rule)
        return grpcmsg

    def Show(self):
        logger.info("Meter object:", self)
        logger.info("- %s" % repr(self))
        for rule in self.Rules:
            rule.Show()
        return

    def SetupTestcaseConfig(self, obj):
        return

class MeterObjectClient:
    def __init__(self):
        self.__objs = []
        self.__v4objs = {}
        self.__v6objs = {}
        self.__v4iter = {}
        self.__v6iter = {}
        return

    def Objects(self):
        return self.__objs

    def GetV4MeterId(self, vpcid):
        if len(self.__objs):
            assert(len(self.__v4objs[vpcid]) != 0)
            return self.__v4iter[vpcid].rrnext().MeterId
        else:
            return 0

    def GetV6MeterId(self, vpcid):
        if len(self.__objs):
            assert(len(self.__v6objs[vpcid]) != 0)
            return self.__v6iter[vpcid].rrnext().MeterId
        else:
            return 0

    def GetNumMeterPerVPC(self):
        return self.__num_meter_per_vpc

    def GenerateObjects(self, parent, vpcspecobj):
        vpcid = parent.VPCId
        stack = parent.Stack
        self.__v4objs[vpcid] = []
        self.__v6objs[vpcid] = []
        self.__v4iter[vpcid] = None
        self.__v6iter[vpcid] = None
        self.__num_meter_per_vpc = 0

        if getattr(vpcspecobj, 'meter', None) == None:
            return

        if utils.IsPipelineArtemis() == False:
            return

        def __is_v4stack():
            if stack == "dual" or stack == 'ipv4':
                return True
            return False

        def __is_v6stack():
            if stack == "dual" or stack == 'ipv6':
                return True
            return False

        def __add_specific_meter_prefixes(rulespec, af):
            prefixes = []
            if af == utils.IP_VERSION_4:
                for r in rulespec.v4prefixes:
                    base = ipaddress.ip_network(r.replace('\\', '/'))
                    prefix = ipaddress.ip_network(base)
                    prefixes.append(prefix)
            else:
                for r in rulespec.v6prefixes:
                    base = ipaddress.ip_network(r.replace('\\', '/'))
                    prefix = ipaddress.ip_network(base)
                    prefixes.append(prefix)
            return prefixes

        def __add_meter_rules(rule_spec, af, metercount):
            rules = []
            for rulespec in rule_spec:
                prefixes = []
                if af == utils.IP_VERSION_4:
                    pfx = ipaddress.ip_network(rulespec.v4base.replace('\\', '/'))
                else:
                    pfx = ipaddress.ip_network(rulespec.v6base.replace('\\', '/'))
                totalhosts = ipaddress.ip_network(pfx).num_addresses * (metercount * rulespec.num_prefixes)
                new_pfx = str(pfx.network_address + totalhosts) + '/' + str(pfx.prefixlen)
                prefix = ipaddress.ip_network(new_pfx)
                prefixes.append(prefix)
                c = 1
                while c < rulespec.num_prefixes:
                    pfx = utils.GetNextSubnet(prefix)
                    prefix = ipaddress.ip_network(pfx)
                    prefixes.append(prefix)
                    c += 1
                prefixes.extend(__add_specific_meter_prefixes(rulespec, af))
                obj = MeterRuleObject(rulespec.type, rulespec.priority, prefixes)
                rules.append(obj)
            return rules

        def __add_meter_v4rules_from_routetable(meterspec):
            base_priority = meterspec.base_priority
            rule_type = meterspec.rule_type
            total_rt = route.client.GetRouteV4Tables(vpcid)
            rules = []

            if total_rt != None:
                for rt_id, rt_obj in total_rt.items():
                    if rt_obj.RouteType != 'overlap':
                        # one rule for all routes in one route table
                        prefixes = list(rt_obj.routes)
                        ruleobj = MeterRuleObject(rule_type, base_priority, prefixes)
                        base_priority += 1
                        rules.append(ruleobj)
            return rules

        def __add_meter_v6rules_from_routetable(meterspec):
            total_rt = route.client.GetRouteV6Tables(vpcid)
            base_priority = meterspec.base_priority
            rule_type = meterspec.rule_type
            rules = []

            if total_rt != None:
                for rt_id, rt_obj in total_rt.items():
                    if rt_obj.RouteType != 'overlap':
                        # one rule for all routes in one route table
                        prefixes = list(rt_obj.routes)
                        ruleobj = MeterRuleObject(rule_type, base_priority, prefixes)
                        base_priority += 1
                        rules.append(ruleobj)
            return rules

        for meter in vpcspecobj.meter:
            c = 0
            if meter.auto_fill:
                if __is_v4stack():
                    rules = __add_meter_v4rules_from_routetable(meter)
                    obj = MeterObject(parent, utils.IP_VERSION_4, rules)
                    self.__v4objs[vpcid].append(obj)
                    self.__objs.append(obj)
                if __is_v6stack():
                    rules = __add_meter_v6rules_from_routetable(meter)
                    obj = MeterObject(parent, utils.IP_VERSION_6, rules)
                    self.__v6objs[vpcid].append(obj)
                    self.__objs.append(obj)
            else:
                while c < meter.count:
                    if __is_v4stack():
                        rules = __add_meter_rules(meter.rule, utils.IP_VERSION_4, c)
                        obj = MeterObject(parent, utils.IP_VERSION_4, rules)
                        self.__v4objs[vpcid].append(obj)
                        self.__objs.append(obj)
                    if __is_v6stack():
                        rules = __add_meter_rules(meter.rule, utils.IP_VERSION_6, c)
                        obj = MeterObject(parent, utils.IP_VERSION_6, rules)
                        self.__v6objs[vpcid].append(obj)
                        self.__objs.append(obj)
                    c += 1

        if len(self.__v4objs[vpcid]):
            self.__v4iter[vpcid] = utils.rrobiniter(self.__v4objs[vpcid])
        if len(self.__v6objs[vpcid]):
            self.__v6iter[vpcid] = utils.rrobiniter(self.__v6objs[vpcid])
        self.__num_meter_per_vpc = vpcspecobj.meter.count
        return

    def CreateObjects(self):
        if utils.IsPipelineArtemis():
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
            api.client.Create(api.ObjectTypes.METER, msgs)
        return

client = MeterObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
