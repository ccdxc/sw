#! /usr/bin/python3
import ipaddress
import pdb

from infra.common.logging import logger

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
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
    def __init__(self, af, rules):
        super().__init__()
        self.SetBaseClassAttr()
        ################# PUBLIC ATTRIBUTES OF TAG TABLE OBJECT #####################
        if af == utils.IP_VERSION_6:
            self.TagTblId = next(resmgr.V6TagIdAllocator)
            self.AddrFamily = 'IPV6'
            self.GID('IPv6TagTbl%d' %self.TagTblId)
        else:
            self.TagTblId = next(resmgr.V4TagIdAllocator)
            self.AddrFamily = 'IPV4'
            self.GID('IPv4TagTbl%d' %self.TagTblId)
        self.Rules = rules
        ##########################################################################
        self.Show()
        return

    def __repr__(self):
        return "TagTblID:%dAddrFamily:%s|NumRules:%d"\
               %(self.TagTblId, self.AddrFamily, len(self.Rules))

    def Show(self):
        logger.info("TagTbl object:", self)
        logger.info("- %s" % repr(self))
        for rule in self.Rules:
            rule.Show()
        return

    def SetBaseClassAttr(self):
        self.ObjType = api.ObjectTypes.TAG
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.TagTblId)
        return

    def PopulateSpec(self, grpcmsg):
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
        return


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

    def GetTagTable(self, vpcid, af):
        tagtblid = self.GetTagV6TableId(vpcid) if af == utils.IP_VERSION_6 else self.GetTagV4TableId(vpcid)
        tagtbl = self.GetTagV6Table(vpcid, tagtblid) if af == utils.IP_VERSION_6 else self.GetTagV4Table(vpcid, tagtblid)
        return tagtbl

    def GetCreateTag(self, vpcid, af, pfx, tagid=0):
        tagtbl = self.GetTagTable(vpcid, af)
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

    def GenerateObjects(self, parent, vpc_spec_obj):
        vpcid = parent.VPCId
        isV4Stack = utils.IsV4Stack(parent.Stack)
        isV6Stack = utils.IsV6Stack(parent.Stack)
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

        if self.__v6objs[vpcid]:
            self.__v6iter[vpcid] = utils.rrobiniter(self.__v6objs[vpcid].values())

        if self.__v4objs[vpcid]:
            self.__v4iter[vpcid] = utils.rrobiniter(self.__v4objs[vpcid].values())

    def ShowObjects(self):
        objs = self.__objs.values()
        for obj in objs:
            obj.Show()
        return

    def CreateObjects(self):
        if utils.IsPipelineArtemis():
            #Show before create as tag gets modified after GenerateObjects()
            self.ShowObjects()
            logger.info("Creating TAG Objects in agent")
            cookie = utils.GetBatchCookie()
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__objs.values()))
            api.client.Create(api.ObjectTypes.TAG, msgs)
        return

    def GetGrpcReadAllMessage(self):
        grpcmsg = tags_pb2.TagGetRequest()
        return grpcmsg

    def ReadObjects(self):
        if utils.IsPipelineArtemis():
            msg = self.GetGrpcReadAllMessage()
            api.client.Get(api.ObjectTypes.TAG, [msg])
        return

client = TagObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
