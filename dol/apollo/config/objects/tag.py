#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.nexthop as nexthop
import tags_pb2 as tags_pb2
import types_pb2 as types_pb2
import ipaddress

from infra.common.logging import logger
from apollo.config.store import Store

class TagRuleObject(base.ConfigObjectBase):
    def __init__(self, prefixes, tag_id, priority):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF ROUTE TABLE OBJECT #####################
        self.Priority = priority
        self.TagId = tag_id
        self.Prefixes = prefixes
        ##########################################################################
        return

class TagObject(base.ConfigObjectBase):
    def __init__(self, af, rules):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF ROUTE TABLE OBJECT #####################
        if af == utils.IP_VERSION_6:
            self.TagTblId = next(resmgr.V6TagIdAllocator)
            self.AddrFamily = 'IPV6'
            self.GID('Ipv4TagTbl%d' %self.TagTblId)
        else:
            self.TagTblId = next(resmgr.V4TagIdAllocator)
            self.AddrFamily = 'IPV4'
            self.GID('Ipv6TagTbl%d' %self.TagTblId)
        self.Rules = rules
        ##########################################################################
        self.Show()
        return

    def __repr__(self):
        return "TagTblID:%dAddrFamily:%s|NumRules:%d|"\
               %(self.TagTblId, self.AddrFamily, len(self.Rules))

    def GetGrpcCreateMessage(self):
        grpcmsg = tags_pb2.TagRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.TagTblId
        spec.Af = utils.GetRpcIPAddrFamily(self.AddrFamily)
        for rule in self.Rules:
            tagrulespec = spec.Rules.add()
            tagrulespec.Tag = rule.TagId
            tagrulespec.Priority = rule.Priority
            for prefix in rule.Prefixes:
                tagprefix = tagrulespec.Prefix.add()
                utils.GetRpcIPPrefix(prefix, tagprefix)

        return grpcmsg

    def Show(self):
        logger.info("TagTbl object:", self)
        logger.info("- %s" % repr(self))
        for rule in self.Rules:
            logger.info("-- priority:%d|TagId:%d" %(rule.Priority, rule.TagId))
            for pfx in rule.Prefixes:
                logger.info("-- %s" % str(pfx))
        return

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.route.filters)

class TagObjectClient:
    def __init__(self):
        self.__objs = dict()
        self.__v4objs = {}
        self.__v6objs = {}
        self.__v4iter = {}
        self.__v6iter = {}
        return

    def Objects(self):
        return self.__objs.values()

    def IsValidConfig(self):
        count = sum(list(map(lambda x: len(x.values()), self.__v4objs.values())))
        if  count > resmgr.MAX_TAG:
            return False, "V4 Tag Table count %d exceeds allowed limit of %d" %\
                          (count, resmgr.MAX_TAG)
        count = sum(list(map(lambda x: len(x.values()), self.__v6objs.values())))
        if  count > resmgr.MAX_TAG:
            return False, "V6 Tag Table count %d exceeds allowed limit of %d" %\
                          (count, resmgr.MAX_TAG)
        #TODO: check route table count equals subnet count in that VPC
        #TODO: check scale of routes per route table
        return True, ""

    def GetTagV4Table(self, vpcid, tagtblid):
        return self.__v4objs[vpcid].get(tagtblid, None)

    def GetTagV6Table(self, vpcid, tagtblid):
        return self.__v6objs[vpcid].get(tagtblid, None)

    def GetTagV4TableId(self, vpcid):
        if self.__v4objs[vpcid]:
            return self.__v4iter[vpcid].rrnext().TagTblId
        return 0

    def GetTagV6TableId(self, vpcid):
        if self.__v6objs[vpcid]:
            return self.__v6iter[vpcid].rrnext().TagTblId
        return 0

    def GenerateObjects(self, parent, vpc_spec_obj):
        vpcid = parent.VPCId
        stack = parent.Stack
        self.__v4objs[vpcid] = dict()
        self.__v6objs[vpcid] = dict()
        self.__v4iter[vpcid] = None
        self.__v6iter[vpcid] = None

        if utils.IsPipelineArtemis() == False:
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

        def __is_v4stack():
            if stack == "dual" or stack == 'ipv4':
                return True
            return False

        def __is_v6stack():
            if stack == "dual" or stack == 'ipv6':
                return True
            return False

        def __add_v4tagtable(v4rules):
            obj = TagObject(utils.IP_VERSION_4, v4rules)
            self.__v4objs[vpcid].update({obj.TagTblId: obj})
            self.__objs.update({obj.TagTblId: obj})

        def __add_v6tagtable(v6rules):
            obj = TagObject(utils.IP_VERSION_6, v6rules)
            self.__v6objs[vpcid].update({obj.TagTblId: obj})
            self.__objs.update({obj.TagTblId: obj})

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
            if __is_v4stack():
                __add_v4tagtable(__get_user_specified_rules(tagtablespec.v4rules))

            if __is_v6stack():
                __add_v6tagtable(__get_user_specified_rules(tagtablespec.v6rules))

        if not hasattr(vpc_spec_obj, 'tagtbl'):
            return

        for tagtbl_spec_obj in vpc_spec_obj.tagtbl:
            tagtbltype = tagtbl_spec_obj.type
            tagpfxtype = tagtbl_spec_obj.pfxtype
            if tagtbltype == "specific":
                __add_user_specified_tagtable(tagtbl_spec_obj, tagpfxtype)
                continue

        if self.__v6objs[vpcid]:
            self.__v6iter[vpcid] = utils.rrobiniter(self.__v6objs[vpcid].values())

        if self.__v4objs[vpcid]:
            self.__v4iter[vpcid] = utils.rrobiniter(self.__v4objs[vpcid].values())


    def CreateObjects(self):
        if utils.IsPipelineArtemis():
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs.values()))
            api.client.Create(api.ObjectTypes.TAG, msgs)
        return

client = TagObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
