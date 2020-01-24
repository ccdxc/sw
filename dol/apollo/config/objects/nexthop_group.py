#! /usr/bin/python3
import pdb
from collections import defaultdict

from infra.common.logging import logger
from apollo.config.store import EzAccessStore

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.objects.base as base
import apollo.config.objects.nexthop as nexthop

import nh_pb2 as nh_pb2

class NexthopGroupObject(base.ConfigObjectBase):
    def __init__(self, node, parent, spec):
        super().__init__(api.ObjectTypes.NEXTHOPGROUP, node)
        if (EzAccessStore.IsDeviceOverlayRoutingEnabled()):
            self.SetOrigin(topo.OriginTypes.DISCOVERED)
        ################# PUBLIC ATTRIBUTES OF NEXTHOPGROUP OBJECT ############
        if (hasattr(spec, 'id')):
            self.Id = spec.id
        else:
            self.Id = next(resmgr.NexthopGroupIdAllocator)
        self.GID('NexthopGroup%d'%self.Id)
        self.Nexthops = {}
        self.DualEcmp = utils.IsDualEcmp(spec)
        self.Type = None
        if spec.type == 'overlay':
            self.Type = nh_pb2.NEXTHOP_GROUP_TYPE_OVERLAY_ECMP
            self.NumNexthops = resmgr.OverlayNumNexthopsAllocator.rrnext()
        elif spec.type == 'underlay':
            self.Type = nh_pb2.NEXTHOP_GROUP_TYPE_UNDERLAY_ECMP
            self.NumNexthops = resmgr.UnderlayNumNexthopsAllocator.rrnext()
        self.Mutable = utils.IsUpdateSupported()
        self.DeriveOperInfo()
        self.Show()
        return

    def get_nexthop_list(self):
        nexthop_list = ""
        for i in self.Nexthops:
            nexthop_list += str(self.Nexthops[i].NexthopId)
            self.Nexthops[i].Show()
            nexthop_list += ", "
        nexthop_list = nexthop_list[:-2]
        return nexthop_list

    def __repr__(self):
        return "NexthopGroupID:%d|Num of nexthops:%d|Nexthop ID List:%s" %\
                (self.Id, self.NumNexthops, self.get_nexthop_list())

    def Show(self):
        logger.info("NexthopGroup object:", self)
        logger.info("- %s" % repr(self))
        type = ""
        if self.Type == nh_pb2.NEXTHOP_GROUP_TYPE_OVERLAY_ECMP:
            type = "OVERLAY ECMP"
        else:
            type = "UNDERLAY ECMP"
        logger.info("- Type %s" % type)
        if self.DualEcmp:
            logger.info("- Dual ecmp")
        return

    def UpdateAttributes(self):
        if self.NumNexthops > 1:
            self.NumNexthops -= 1
        return

    def RollbackAttributes(self):
        self.NumNexthops = self.GetPrecedent().NumNexthops
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(str.encode(str(self.Id)))
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = str.encode(str(self.Id))
        spec.Type = self.Type
        for i in range(self.NumNexthops):
            nhspec = spec.Members.add()
            nexthop_obj = self.Nexthops[i]
            nexthop_obj.FillSpec(nhspec)
        return

    def RestoreNotify(self, cObj):
        logger.info("Notify %s for %s creation" % (self, cObj))
        if not self.IsHwHabitant():
            logger.info(" - Skipping notification as %s already deleted" % self)
            return
        logger.info(" - Linking %s to %s " % (cObj, self))
        logger.error(" - ERROR: %s not handling %s restoration" %\
                     (self.ObjType.name, cObj.ObjType))
        assert(0)
        # self.Update()
        return

    def DeleteNotify(self, dObj):
        logger.info("Notify %s for %s deletion" % (self, dObj))
        if not self.IsHwHabitant():
            logger.info(" - Skipping notification as %s already deleted" % self)
            return
        logger.info(" - Unlinking %s from %s " % (dObj, self))
        logger.error(" - ERROR: %s not handling %s deletion" %\
                     (self.ObjType.name, dObj.ObjType))
        assert(0)
        # self.Update()
        return

    def IsUnderlay(self):
        if self.Type == nh_pb2.NEXTHOP_GROUP_TYPE_UNDERLAY_ECMP:
            return True
        return False

    def IsOverlay(self):
        if self.Type == nh_pb2.NEXTHOP_GROUP_TYPE_OVERLAY_ECMP:
            return True
        return False

class NexthopGroupObjectClient(base.ConfigClientBase):
    def __init__(self):
        def __isObjSupported():
            if utils.IsPipelineApulu():
                return True
            return False

        super().__init__(api.ObjectTypes.NEXTHOPGROUP, resmgr.MAX_NEXTHOPGROUP)
        self.__v4objs = defaultdict(dict)
        self.__v6objs = defaultdict(dict)
        self.__v4iter = defaultdict(dict)
        self.__v6iter = defaultdict(dict)
        self.__num_nhgs_per_vpc = []
        self.__supported = __isObjSupported()
        return

    def GetKeyfromSpec(self, spec, yaml=False):
        if yaml: return utils.GetYamlSpecAttr(spec, 'id')
        return int(spec.Id)

    def GetNexthopGroupObject(self, node, nexthopid):
        return self.Objs[node].get(nexthopid, None)

    def GetNumNextHopGroupsPerVPC(self):
        return self.__num_nhgs_per_vpc

    def CreateAllocator(self, node):
        nh_groups = self.Objects(node)
        if not nh_groups:
            return
        EzAccessStore.SetNexthopgroups(nh_groups)
        resmgr.CreateUnderlayNhGroupAllocator()
        resmgr.CreateOverlayNhGroupAllocator()
        resmgr.CreateDualEcmpNhGroupAllocator()

    def AssociateObjects(self, node):
        logger.info("Filling nexthops")
        nh_groups = self.Objects(node)
        for nhg in nh_groups:
            logger.info("NexthopGroup%d - %d nexthops" % (nhg.Id, nhg.NumNexthops))
            for i in range(nhg.NumNexthops):
                if nhg.Type == nh_pb2.NEXTHOP_GROUP_TYPE_UNDERLAY_ECMP:
                    nhg.Nexthops[i] = resmgr.UnderlayNHAllocator.rrnext()
                elif nhg.Type == nh_pb2.NEXTHOP_GROUP_TYPE_OVERLAY_ECMP:
                    if nhg.DualEcmp is True:
                        nhg.Nexthops[i] = resmgr.DualEcmpNhAllocator.rrnext()
                    else:
                        nhg.Nexthops[i] = resmgr.OverlayNHAllocator.rrnext()
                logger.info("   Nexthop%d" % (nhg.Nexthops[i].NexthopId))

    def GenerateObjects(self, node, parent, vpc_spec_obj):
        if not self.__supported:
            return

        vpcid = parent.VPCId
        isV4Stack = utils.IsV4Stack(parent.Stack)
        isV6Stack = utils.IsV6Stack(parent.Stack)
        self.__v4objs[node][vpcid] = []
        self.__v6objs[node][vpcid] = []
        self.__v4iter[node][vpcid] = None
        self.__v6iter[node][vpcid] = None

        nhg_spec = getattr(vpc_spec_obj, 'nexthop-group', None)
        if nhg_spec == None:
            self.__num_nhgs_per_vpc.append(0)
            return

        for nhg_spec_obj in nhg_spec:
            for c in range(nhg_spec_obj.count):
                obj = NexthopGroupObject(node, parent, nhg_spec_obj)
                self.Objs[node].update({obj.Id: obj})
                if isV4Stack:
                    self.__v4objs[node][vpcid].append(obj)
                if isV6Stack:
                    self.__v6objs[node][vpcid].append(obj)
        if len(self.__v4objs[node][vpcid]):
            self.__v4iter[node][vpcid] = utils.rrobiniter(self.__v4objs[node][vpcid])
        if len(self.__v6objs[node][vpcid]):
            self.__v6iter[node][vpcid] = utils.rrobiniter(self.__v6objs[node][vpcid])
        self.__num_nhgs_per_vpc.append(nhg_spec_obj.count)
        return

    def CreateObjects(self, node):
        cookie = utils.GetBatchCookie(node)
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.Objects(node)))
        api.client[node].Create(api.ObjectTypes.NEXTHOPGROUP, msgs)
        return

    def GetGrpcReadAllMessage(self, node):
        grpcmsg = nh_pb2.NhgroupGetRequest()
        return grpcmsg

    def ReadObjects(self, node):
        msg = self.GetGrpcReadAllMessage(node)
        api.client[node].Get(api.ObjectTypes.NEXTHOPGROUP, [msg])
        return

client = NexthopGroupObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
