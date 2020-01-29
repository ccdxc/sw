#! /usr/bin/python3
import pdb
import ipaddress
from collections import defaultdict

from infra.common.logging import logger

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.objects.base as base
import apollo.config.objects.route as route
from apollo.config.store import EzAccessStore

import meter_pb2 as meter_pb2

class MeterStats:
    def __init__(self):
        self.RxBytes = 0
        self.TxBytes = 0
        return

class MeterStatsHelper:
    def __init__(self, meterid = None):
        self.MeterId = meterid
        self.__sid = 1
        self.__mid =  2 * Resmgr.MAX_METER + 1
        self.PreStats = {}
        self.PostStats = {}

        for c in range(self.__sid, self.__mid):
            self.PreStats[c] = MeterStats()
            self.PostStats[c] = MeterStats()
        return

    def GetMeterStats(self):
        grpcmsg = meter_pb2.MeterGetRequest()
        if self.MeterId != None:
            grpcmsg.Id = self.MeterId
        resp = api.client[EzAccessStore.GetDUTNode()].Get(api.ObjectTypes.METER, [ grpcmsg ])
        if resp != None:
            return resp[0]
        return None

    def __parse_meter_stats_get_response(self, resp, pre = False):
        def __show(entry):
            logger.info("Meter stats for MeterId: %s, tx_bytes: %ld,\
                    rx_bytes: %ld" % (entry.Stats.MeterId,
                    entry.Stats.TxBytes, entry.Stats.RxBytes))
        if resp is None:
            return
        stats = self.PreStats if pre else self.PostStats
        for entry in resp.Response:
            meterid = utils.PdsUuid.GetIdfromUUID(entry.Spec.Id)
            assert meterid != 0
            stats[meterid].RxBytes = entry.Stats.RxBytes
            stats[meterid].TxBytes = entry.Stats.TxBytes
        return

    def IncrMeterStats(self, meterid, rxbytes, txbytes):
        self.PreStats[meterid].RxBytes += rxbytes
        self.PreStats[meterid].TxBytes += txbytes
        if utils.IsDryRun():
            self.PostStats[meterid].RxBytes += rxbytes
            self.PostStats[meterid].TxBytes += txbytes


    def ReadMeterStats(self, pre = False):
        resp = self.GetMeterStats()
        self.__parse_meter_stats_get_response(resp, pre)
        return

    def Show(self, id):
        logger.info("Meter Stats in bytes ID: %u, Pre-tx: %u,  Post-tx:%u, Pre-rx: %u, Post-rx: %u" % \
                    (id, self.PreStats[id].TxBytes, self.PostStats[id].TxBytes,
                     self.PreStats[id].RxBytes, self.PostStats[id].RxBytes))

    def VerifyMeterStats(self):
        rv = True
        for c in range(self.__sid, self.__mid):
            if self.PostStats[c].TxBytes != self.PreStats[c].TxBytes:
                self.Show(c)
                rv = False
            if self.PostStats[c].RxBytes != self.PreStats[c].RxBytes:
                self.Show(c)
                rv = False
        return rv

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
    def __init__(self, node, parent, af, rules):
        super().__init__(api.ObjectTypes.METER, node)
        ################# PUBLIC ATTRIBUTES OF METER OBJECT #####################
        self.VPCId = parent.VPCId
        if af == utils.IP_VERSION_6:
            self.MeterId = next(ResmgrClient[node].V6MeterIdAllocator)
            self.AddrFamily = 'IPV6'
        else:
            self.MeterId = next(ResmgrClient[node].V4MeterIdAllocator)
            self.AddrFamily = 'IPV4'
        self.GID('Meter%d'%self.MeterId)
        self.UUID = utils.PdsUuid(self.MeterId)
        self.Rules = rules
        self.DeriveOperInfo()
        self.Show()
        return

    def __repr__(self):
        return "Meter: %s|Af:%s|VPCId:%d" %\
               (self.UUID, self.AddrFamily, self.VPCId)

    def Show(self):
        logger.info("Meter object:", self)
        logger.info("- %s" % repr(self))
        for rule in self.Rules:
            rule.Show()
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def FillMeterRulePrefixes(self, rulespec, rule):
        for pfx in rule.Prefixes:
            pfxobj = rulespec.Prefix.add()
            utils.GetRpcIPPrefix(pfx, pfxobj)
        return

    def FillMeterRuleSpec(self, spec, rule):
        ruleobj = spec.rules.add()
        self.FillMeterRulePrefixes(ruleobj, rule)
        ruleobj.Priority = rule.Priority
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.Af = utils.GetRpcIPAddrFamily(self.AddrFamily)
        for rule in self.Rules:
            self.FillMeterRuleSpec(spec, rule)
        return


class MeterObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.METER, Resmgr.MAX_METER)
        self.__v4objs = defaultdict(dict)
        self.__v6objs = defaultdict(dict)
        self.__v4iter = defaultdict(dict)
        self.__v6iter = defaultdict(dict)
        self.__num_v4_meter_per_vpc = []
        self.__num_v6_meter_per_vpc = []
        return

    def GetV4MeterId(self, node, vpcid):
        if self.GetNumObjects(node):
            assert(len(self.__v4objs[node][vpcid]) != 0)
            return self.__v4iter[node][vpcid].rrnext().MeterId
        else:
            return 0

    def GetV6MeterId(self, node, vpcid):
        if self.GetNumObjects(node):
            assert(len(self.__v6objs[node][vpcid]) != 0)
            return self.__v6iter[node][vpcid].rrnext().MeterId
        else:
            return 0

    def GetNumMeterPerVPC(self):
        return self.__num_v4_meter_per_vpc,self.__num_v6_meter_per_vpc

    def GenerateObjects(self, node, parent, vpcspecobj):
        vpcid = parent.VPCId
        isV4Stack = utils.IsV4Stack(parent.Stack)
        isV6Stack = utils.IsV6Stack(parent.Stack)
        self.__v4objs[node][vpcid] = []
        self.__v6objs[node][vpcid] = []
        self.__v4iter[node][vpcid] = None
        self.__v6iter[node][vpcid] = None

        if getattr(vpcspecobj, 'meter', None) == None:
            self.__num_v4_meter_per_vpc.append(0)
            self.__num_v6_meter_per_vpc.append(0)
            return

        if not utils.IsMeteringSupported():
            return

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

        def __add_meter_rules_from_routetable(meterspec, af):
            base_priority = meterspec.base_priority
            rule_type = meterspec.rule_type
            rules = []

            if af == utils.IP_VERSION_4:
                total_rt = route.client.GetRouteV4Tables(node, vpcid)
            else:
                total_rt = route.client.GetRouteV6Tables(node, vpcid)
            if total_rt != None:
                for rt_id, rt_obj in total_rt.items():
                    if rt_obj.RouteType != 'overlap':
                        # one rule for all routes in one route table
                        pfxs = list(rt_obj.routes.values())
                        prefixes = []
                        for pfx in pfxs:
                            prefixes.append(pfx.ipaddr)
                        ruleobj = MeterRuleObject(rule_type, base_priority, prefixes)
                        base_priority += 1
                        rules.append(ruleobj)
            return rules

        for meter in vpcspecobj.meter:
            c = 0
            v4_count = 0
            v6_count = 0
            if meter.auto_fill:
                if isV4Stack:
                    rules = __add_meter_rules_from_routetable(meter, utils.IP_VERSION_4)
                    obj = MeterObject(node, parent, utils.IP_VERSION_4, rules)
                    self.__v4objs[node][vpcid].append(obj)
                    self.Objs[node].update({obj.MeterId: obj})
                    v4_count += len(rules)
                if isV6Stack:
                    rules = __add_meter_rules_from_routetable(meter, utils.IP_VERSION_6)
                    obj = MeterObject(node, parent, utils.IP_VERSION_6, rules)
                    self.__v6objs[node][vpcid].append(obj)
                    self.Objs[node].update({obj.MeterId: obj})
                    v6_count += len(rules)
            else:
                while c < meter.count:
                    if isV4Stack:
                        rules = __add_meter_rules(meter.rule, utils.IP_VERSION_4, c)
                        obj = MeterObject(node, parent, utils.IP_VERSION_4, rules)
                        self.__v4objs[node][vpcid].append(obj)
                        self.Objs[node].update({obj.MeterId: obj})
                        v4_count += len(rules)
                    if isV6Stack:
                        rules = __add_meter_rules(meter.rule, utils.IP_VERSION_6, c)
                        obj = MeterObject(node, parent, utils.IP_VERSION_6, rules)
                        self.__v6objs[node][vpcid].append(obj)
                        self.Objs[node].update({obj.MeterId: obj})
                        v6_count += len(rules)
                    c += 1

        if len(self.__v4objs[node][vpcid]):
            self.__v4iter[node][vpcid] = utils.rrobiniter(self.__v4objs[node][vpcid])
        if len(self.__v6objs[node][vpcid]):
            self.__v6iter[node][vpcid] = utils.rrobiniter(self.__v6objs[node][vpcid])
        self.__num_v4_meter_per_vpc.append(v4_count)
        self.__num_v6_meter_per_vpc.append(v6_count)
        return

    def ReadObjects(self, node):
        # TODO: Add validation
        msg = self.GetGrpcReadAllMessage(node)
        api.client[node].Get(self.ObjType, [msg])
        return

client = MeterObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
