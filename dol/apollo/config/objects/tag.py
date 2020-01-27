#! /usr/bin/python3
import ipaddress
import pdb
from collections import defaultdict

from infra.common.logging import logger

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.objects.base as base

import tags_pb2 as tags_pb2

class TagRuleObject:
    def __init__(self, prefixes, tag_id, priority=0):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF TAG RULE OBJECT #####################
        self.Priority = priority
        self.TagId = tag_id
        self.Prefixes = prefixes
        ##########################################################################
        return

    def Show(self):
        logger.info(" -- priority:%d|TagId:%d|NumPfxs:%d"\
                    %(self.Priority, self.TagId, len(self.Prefixes)))
        for pfx in self.Prefixes:
            logger.info("  --- %s" % str(pfx))
        return

class TagObject(base.ConfigObjectBase):
    def __init__(self, node, af, rules):
        super().__init__(api.ObjectTypes.TAG, node)
        ################# PUBLIC ATTRIBUTES OF TAG TABLE OBJECT #####################
        if af == utils.IP_VERSION_6:
            self.TagTblId = next(resmgr.V6TagIdAllocator)
            self.AddrFamily = 'IPV6'
            self.GID('IPv6TagTbl%d' %self.TagTblId)
        else:
            self.TagTblId = next(resmgr.V4TagIdAllocator)
            self.AddrFamily = 'IPV4'
            self.GID('IPv4TagTbl%d' %self.TagTblId)
        self.UUID = utils.PdsUuid(self.TagTblId)
        self.Rules = rules
        ##########################################################################
        self.DeriveOperInfo()
        self.Show()
        return

    def __repr__(self):
        return "TagTbl: %s |AddrFamily:%s|NumRules:%d"\
               %(self.UUID, self.AddrFamily, len(self.Rules))

    def Show(self):
        logger.info("TagTbl object:", self)
        logger.info("- %s" % repr(self))
        for rule in self.Rules:
            rule.Show()
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.Af = utils.GetRpcIPAddrFamily(self.AddrFamily)
        for rule in self.Rules:
            tagrulespec = spec.Rules.add()
            tagrulespec.Tag = rule.TagId
            tagrulespec.Priority = rule.Priority
            for prefix in rule.Prefixes:
                tagprefix = tagrulespec.Prefix.add()
                utils.GetRpcIPPrefix(prefix, tagprefix)
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.Af != utils.GetRpcIPAddrFamily(self.AddrFamily):
            return False
        return True

    def ValidateYamlSpec(self, spec):
        if utils.GetYamlSpecAttr(spec, 'id') != self.GetKey():
            return False
        if spec['af'] != utils.GetRpcIPAddrFamily(self.AddrFamily):
            return False
        return True

class TagObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.TAG, resmgr.MAX_TAG)
        self.__v4objs = defaultdict(dict)
        self.__v6objs = defaultdict(dict)
        self.__v4iter = defaultdict(dict)
        self.__v6iter = defaultdict(dict)
        return

    def IsValidConfig(self):
        count = sum(list(map(lambda x: len(x.values()), self.__v4objs[node].values())))
        if  count > self.Maxlimit:
            return False, "V4 Tag Table count %d exceeds allowed limit of %d" %\
                          (count, self.Maxlimit)
        count = sum(list(map(lambda x: len(x.values()), self.__v6objs[node].values())))
        if  count > self.Maxlimit:
            return False, "V6 Tag Table count %d exceeds allowed limit of %d" %\
                          (count, self.Maxlimit)
        #TODO: check route table count equals subnet count in that VPC
        #TODO: check scale of routes per route table
        return True, ""

    def GetTagV4Table(self, node, vpcid, tagtblid):
        return self.__v4objs[node][vpcid].get(tagtblid, None)

    def GetTagV6Table(self, node, vpcid, tagtblid):
        return self.__v6objs[node][vpcid].get(tagtblid, None)

    def GetTagV4TableId(self, node, vpcid):
        if self.__v4objs[node][vpcid]:
            return self.__v4iter[node][vpcid].rrnext().TagTblId
        return 0

    def GetTagV6TableId(self, node, vpcid):
        if self.__v6objs[node][vpcid]:
            return self.__v6iter[node][vpcid].rrnext().TagTblId
        return 0

    def GetTagTable(self, node, vpcid, af):
        tagtblid = self.GetTagV6TableId(node, vpcid) if af == utils.IP_VERSION_6 else self.GetTagV4TableId(node, vpcid)
        tagtbl = self.GetTagV6Table(node, vpcid, tagtblid) if af == utils.IP_VERSION_6 else self.GetTagV4Table(node, vpcid, tagtblid)
        return tagtbl

    def GetCreateTag(self, node, vpcid, af, pfx, tagid=0):
        tagtbl = self.GetTagTable(node, vpcid, af)
        tagtbl.Rules.sort(key=lambda x: x.TagId)
        for rule in tagtbl.Rules:
            for tagpfx in rule.Prefixes:
                if pfx == tagpfx:
                    return rule
        if tagid == 0:
            lastrule = tagtbl.Rules[-1]
            tagid = lastrule.TagId + 1
        obj = TagRuleObject([pfx], tagid)
        tagtbl.Rules.append(obj)
        return obj

    def GenerateObjects(self, node, parent, vpc_spec_obj):
        vpcid = parent.VPCId
        isV4Stack = utils.IsV4Stack(parent.Stack)
        isV6Stack = utils.IsV6Stack(parent.Stack)
        self.__v4objs[node][vpcid] = dict()
        self.__v6objs[node][vpcid] = dict()
        self.__v4iter[node][vpcid] = None
        self.__v6iter[node][vpcid] = None

        if not utils.IsTagSupported():
            return

        def __get_adjacent_routes(base, count):
            routes = []
            c = 1
            routes.append(ipaddress.ip_network(base))
            while c < count:
                routes.append(utils.GetNextSubnet(routes[c-1]))
                c += 1
            return routes

        def __get_overlap(basepfx, base, count):
            # for overlap, add user specified base prefix with original prefixlen
            routes = __get_user_specified_routes([basepfx])
            routes.extend(__get_adjacent_routes(base, count))
            return routes

        def __get_first_subnet(ip, prefixlen):
            for ip in ip.subnets(new_prefix=prefixlen):
                return (ip)
            return

        def __add_v4tagtable(v4rules):
            obj = TagObject(node, utils.IP_VERSION_4, v4rules)
            self.__v4objs[node][vpcid].update({obj.TagTblId: obj})
            self.Objs[node].update({obj.TagTblId: obj})

        def __add_v6tagtable(v6rules):
            obj = TagObject(node, utils.IP_VERSION_6, v6rules)
            self.__v6objs[node][vpcid].update({obj.TagTblId: obj})
            self.Objs[node].update({obj.TagTblId: obj})

        def __get_user_specified_rules(rulesspec):
            rules = []
            if rulesspec:
                for rule in rulesspec:
                    prefixes = []
                    for prefix in rule.prefixes:
                        prefixes.append(ipaddress.ip_network(prefix.replace('\\', '/')))
                    obj = TagRuleObject(prefixes, rule.tag, rule.priority)
                    rules.append(obj)
            return rules

        def __add_user_specified_tagtable(tagtablespec, pfxtype):
            if isV4Stack:
                __add_v4tagtable(__get_user_specified_rules(tagtablespec.v4rules))

            if isV6Stack:
                __add_v6tagtable(__get_user_specified_rules(tagtablespec.v6rules))

        if not hasattr(vpc_spec_obj, 'tagtbl'):
            return

        for tagtbl_spec_obj in vpc_spec_obj.tagtbl:
            tagtbltype = tagtbl_spec_obj.type
            tagpfxtype = tagtbl_spec_obj.pfxtype
            if tagtbltype == "specific":
                __add_user_specified_tagtable(tagtbl_spec_obj, tagpfxtype)
                continue

        if self.__v6objs[node][vpcid]:
            self.__v6iter[node][vpcid] = utils.rrobiniter(self.__v6objs[node][vpcid].values())

        if self.__v4objs[node][vpcid]:
            self.__v4iter[node][vpcid] = utils.rrobiniter(self.__v4objs[node][vpcid].values())
        return

    """
    def ReadObjects(self):
        msg = self.GetGrpcReadAllMessage()
        api.client.Get(self.ObjType, [msg])
        return
    """

client = TagObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
