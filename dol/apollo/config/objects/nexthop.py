#! /usr/bin/python3
import pdb
from collections import defaultdict

from infra.common.logging import logger

from apollo.config.store import client as EzAccessStoreClient

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.objects.base as base
from apollo.config.agent.api import ObjectTypes as ObjectTypes
from apollo.config.objects.interface import client as InterfaceClient
from apollo.config.objects.tunnel    import client as TunnelClient
import copy
import nh_pb2 as nh_pb2

class NexthopStatus(base.StatusObjectBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.NEXTHOP)

class NexthopObject(base.ConfigObjectBase):
    def __init__(self, node, parent, spec):
        super().__init__(api.ObjectTypes.NEXTHOP, node)
        if hasattr(spec, 'origin'):
            self.SetOrigin(spec.origin)
        elif (EzAccessStoreClient[node].IsDeviceOverlayRoutingEnabled()):
            self.SetOrigin('discovered')
        ################# PUBLIC ATTRIBUTES OF NEXTHOP OBJECT #####################
        if (hasattr(spec, 'id')):
            self.NexthopId = spec.id
        else:
            self.NexthopId = next(ResmgrClient[node].NexthopIdAllocator)
        self.GID('Nexthop%d'%self.NexthopId)
        self.UUID = utils.PdsUuid(self.NexthopId, self.ObjType)
        self.VPC = parent
        nh_type = getattr(spec, 'type', 'ip')
        self.DualEcmp = utils.IsDualEcmp(spec)
        if nh_type == 'ip':
            self.__type = topo.NhType.IP
            self.PfxSel = parent.PfxSel
            self.IPAddr = {}
            self.IPAddr[0] = next(ResmgrClient[node].NexthopIpV4AddressAllocator)
            self.IPAddr[1] = next(ResmgrClient[node].NexthopIpV6AddressAllocator)
            self.VlanId = next(ResmgrClient[node].NexthopVlanIdAllocator)
            if (hasattr(spec, 'macaddress')):
                self.MACAddr = spec.macaddress
            else:
                self.MACAddr = ResmgrClient[node].NexthopMacAllocator.get()
        elif nh_type == 'underlay':
            self.__type = topo.NhType.UNDERLAY
            self.L3Interface = InterfaceClient.GetL3UplinkInterface(node)
            self.L3InterfaceId = self.L3Interface.InterfaceId
            if (hasattr(spec, 'macaddress')):
                self.underlayMACAddr = spec.macaddress
            else:
                self.underlayMACAddr = ResmgrClient[node].NexthopMacAllocator.get()
        elif nh_type == 'overlay':
            self.__type = topo.NhType.OVERLAY
            if self.DualEcmp:
                self.TunnelId = ResmgrClient[node].UnderlayECMPTunAllocator.rrnext().Id
            elif (hasattr(spec, 'tunnelid')):
                self.TunnelId = spec.tunnelid
            else:
                self.TunnelId = ResmgrClient[node].UnderlayTunAllocator.rrnext().Id
        else:
            self.__type = topo.NhType.NONE
        self.Mutable = utils.IsUpdateSupported()
        self.Status = NexthopStatus()
        self.DeriveOperInfo()
        self.Show()
        return

    def __repr__(self):
        if self.__type == topo.NhType.IP:
            nh_str = "VPCId:%d|PfxSel:%d|IP:%s|Mac:%s|Vlan:%d" %\
                     (self.VPC.VPCId, self.PfxSel, self.IPAddr[self.PfxSel],
                     self.MACAddr, self.VlanId)
        elif self.__type == topo.NhType.UNDERLAY:
            nh_str = "L3IfID:%d|UnderlayMac:%s" %\
                     (self.L3InterfaceId, self.underlayMACAddr)
        elif self.__type == topo.NhType.OVERLAY:
            nh_str = "TunnelId:%d" % (self.TunnelId)
        else:
            nh_str = ""
        ecmp_str = ""
        if self.DualEcmp:
            ecmp_str = ", Dual ecmp"
        return "NexthopID:%d|Type:%s|%s%s" %\
               (self.NexthopId, self.__type, nh_str, ecmp_str)

    def Show(self):
        logger.info("Nexthop object:", self)
        logger.info("- %s" % repr(self))
        self.Status.Show()
        return

    def Dup(self):
        dupObj = copy.copy(self)
        dupObj.NexthopId = next(ResmgrClient[self.Node].NexthopIdAllocator) + Resmgr.BaseDuplicateIdAllocator
        dupObj.GID('DupNexthop%d'%dupObj.NexthopId)
        dupObj.UUID = utils.PdsUuid(dupObj.NexthopId, dupObj.ObjType)
        dupObj.Interim = True
        self.Duplicate = dupObj
        return dupObj

    def UpdateAttributes(self):
        if self.__type == topo.NhType.IP:
            self.IPAddr[0] = next(ResmgrClient[self.Node].NexthopIpV4AddressAllocator)
            self.IPAddr[1] = next(ResmgrClient[self.Node].NexthopIpV6AddressAllocator)
            self.VlanId = next(ResmgrClient[self.Node].NexthopVlanIdAllocator)
            self.MACAddr = ResmgrClient[self.Node].NexthopMacAllocator.get()
        elif self.__type == topo.NhType.UNDERLAY:
            self.L3Interface = InterfaceClient.GetL3UplinkInterface(self.Node)
            self.L3InterfaceId = self.L3Interface.InterfaceId
            self.underlayMACAddr = ResmgrClient[self.Node].NexthopMacAllocator.get()
        elif self.__type == topo.NhType.OVERLAY:
            if self.DualEcmp:
                self.TunnelId = ResmgrClient[self.Node].UnderlayECMPTunAllocator.rrnext().Id
            else:
                self.TunnelId = ResmgrClient[self.Node].UnderlayTunAllocator.rrnext().Id
        return

    def RollbackAttributes(self):
        if self.__type == topo.NhType.IP:
            attrlist = ["IPAddr", "VlanId", "MACAddr"]
        elif self.__type == topo.NhType.UNDERLAY:
            attrlist = ["L3Interface", "underlayMACAddr", "L3InterfaceId"]
        elif self.__type == topo.NhType.OVERLAY:
            attrlist = ["TunnelId"]
        self.RollbackMany(attrlist)
        return

    def GetGrpcReadMessage(self):
        # TODO: NH read req has only filters
        grpcreq = api.client[self.Node].GetGRPCMsgReq(self.ObjType, api.ApiOps.GET)
        grpcmsg = grpcreq()
        grpcmsg.Id = self.GetKey()
        return grpcmsg

    def PopulateKey(self, grpcmsg):
        # TODO FIX THIS nh read message can take only one id, whereas nhdelete can take a list
        grpcmsg.Id.append(self.GetKey())
        return

    def FillSpec(self, spec):
        spec.Id = self.GetKey()
        if self.__type == topo.NhType.IP:
            spec.IPNhInfo.VPCId = self.VPC.GetKey()
            spec.IPNhInfo.Mac = self.MACAddr.getnum()
            spec.IPNhInfo.Vlan = self.VlanId
            utils.GetRpcIPAddr(self.IPAddr[self.PfxSel], spec.IPNhInfo.IP)
        elif self.__type == topo.NhType.UNDERLAY:
            spec.UnderlayNhInfo.L3Interface = utils.PdsUuid.GetUUIDfromId(self.L3InterfaceId, ObjectTypes.INTERFACE)
            spec.UnderlayNhInfo.UnderlayMAC = self.underlayMACAddr.getnum()
        elif self.__type == topo.NhType.OVERLAY:
            spec.OverlayNhInfo.TunnelId = utils.PdsUuid.GetUUIDfromId(self.TunnelId, ObjectTypes.TUNNEL)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        self.FillSpec(spec)
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if self.__type == topo.NhType.IP:
            if spec.IPNhInfo.Mac != self.MACAddr.getnum():
                return False
            if spec.IPNhInfo.Vlan != self.VlanId:
                return False
            if spec.IPNhInfo.VPCId != self.VPC.GetKey():
                return False
            if utils.ValidateRpcIPAddr(self.IPAddr[self.PfxSel], spec.IPNhInfo.IP) == False:
                return False
        elif self.__type == topo.NhType.UNDERLAY:
            if spec.UnderlayNhInfo.L3Interface != utils.PdsUuid.GetUUIDfromId(self.L3InterfaceId, ObjectTypes.INTERFACE):
                return False
            if spec.UnderlayNhInfo.UnderlayMAC != self.underlayMACAddr.getnum():
                return False
        elif self.__type != topo.NhType.OVERLAY:
            if spec.OverlayNhInfo.TunnelId != utils.PdsUuid.GetUUIDfromId(self.TunnelId, ObjectTypes.TUNNEL):
                return False
        return True

    def GetDependees(self, node):
        """
        depender/dependent - nexthop
        dependee - l3interface, tunnel
        """
        dependees = [ ]
        if self.IsUnderlay():
            l3intfObj = InterfaceClient.GetObjectByKey(node, self.L3InterfaceId)
            dependees.append(l3intfObj)
        elif self.IsOverlay():
            tunnelObj = TunnelClient.GetObjectByKey(node, self.TunnelId)
            dependees.append(tunnelObj)
        return dependees

    def RestoreNotify(self, cObj):
        logger.info("Notify %s for %s creation" % (self, cObj))
        if not self.IsHwHabitant():
            logger.info(" - Skipping notification as %s already deleted" % self)
            return
        logger.info(" - Linking %s to %s " % (cObj, self))
        if cObj.ObjType == api.ObjectTypes.TUNNEL:
            self.TunnelId = cObj.Id
        elif cObj.ObjType == api.ObjectTypes.INTERFACE:
            self.L3InterfaceId = cObj.InterfaceId
        else:
            logger.error(" - ERROR: %s not handling %s restoration" %\
                         (self.ObjType.name, cObj.ObjType))
            assert(0)
        self.SetDirty(True)
        self.CommitUpdate()
        return

    def DeleteNotify(self, dObj):
        logger.info("Notify %s for %s deletion" % (self, dObj))
        if not self.IsHwHabitant():
            logger.info(" - Skipping notification as %s already deleted" % self)
            return
        logger.info(" - Unlinking %s from %s " % (dObj, self))
        if dObj.ObjType == api.ObjectTypes.TUNNEL:
            logger.info(" - Linking %s to %s " % (dObj.Duplicate, self))
            self.TunnelId = dObj.Duplicate.Id
        elif dObj.ObjType == api.ObjectTypes.INTERFACE:
            self.L3InterfaceId = dObj.Duplicate.InterfaceId
        else:
            logger.error(" - ERROR: %s not handling %s deletion" %\
                         (self.ObjType.name, dObj.ObjType))
            assert(0)
        self.SetDirty(True)
        self.CommitUpdate()
        return

    def IsUnderlay(self):
        if self.__type == topo.NhType.UNDERLAY:
            return True
        return False

    def IsUnderlayEcmp(self):
        if self.__type == topo.NhType.UNDERLAY_ECMP:
            return True
        return False

    def IsOverlay(self):
        if self.__type == topo.NhType.OVERLAY:
            return True
        return False

    def IsOverlayEcmp(self):
        if self.__type == topo.NhType.OVERLAY_ECMP:
            return True
        return False

class NexthopObjectClient(base.ConfigClientBase):
    def __init__(self):
        def __isObjSupported():
            if utils.IsPipelineArtemis() or utils.IsPipelineApulu():
                return True
            return False

        super().__init__(api.ObjectTypes.NEXTHOP, Resmgr.MAX_NEXTHOP)
        self.__underlay_objs = defaultdict(dict)
        self.__v4objs = defaultdict(dict)
        self.__v6objs = defaultdict(dict)
        self.__v4iter = defaultdict(dict)
        self.__v6iter = defaultdict(dict)
        self.__num_nh_per_vpc = []
        self.__supported = __isObjSupported()
        return

    def GetNexthopObject(self, node, nexthopid):
        return self.GetObjectByKey(node, nexthopid)

    def GetV4Nexthop(self, node, vpcid):
        if len(self.Objects(node)):
            assert(len(self.__v4objs[node][vpcid]) != 0)
            return self.__v4iter[node][vpcid].rrnext()
        else:
            return None

    def GetV6Nexthop(self, node, vpcid):
        if len(self.Objects(node)):
            assert(len(self.__v6objs[node][vpcid]) != 0)
            return self.__v6iter[node][vpcid].rrnext()
        else:
            return None

    def GetNumNextHopPerVPC(self):
        return self.__num_nh_per_vpc

    def GetUnderlayNexthops(self, node):
        return self.__underlay_objs[node].values()

    def AssociateObjects(self, node):
        EzAccessStoreClient[node].SetNexthops(self.Objects(node))
        ResmgrClient[node].CreateUnderlayNHAllocator()
        ResmgrClient[node].CreateOverlayNHAllocator()
        ResmgrClient[node].CreateDualEcmpNhAllocator()
        TunnelClient.AssociateObjects(node)

    def GenerateObjects(self, node, parent, vpc_spec_obj):
        if not self.__supported:
            return

        def __isNhFeatureSupported(nh_type):
            if nh_type == 'underlay' or nh_type == 'overlay':
                return utils.IsPipelineApulu()
            return not utils.IsPipelineApulu()

        vpcid = parent.VPCId
        isV4Stack = utils.IsV4Stack(parent.Stack)
        isV6Stack = utils.IsV6Stack(parent.Stack)
        self.__v4objs[node][vpcid] = []
        self.__v6objs[node][vpcid] = []
        self.__v4iter[node][vpcid] = None
        self.__v6iter[node][vpcid] = None

        if getattr(vpc_spec_obj, 'nexthop', None) == None:
            self.__num_nh_per_vpc.append(0)
            return

        for nh_spec_obj in vpc_spec_obj.nexthop:
            nh_type = getattr(nh_spec_obj, 'type', 'ip')
            if not __isNhFeatureSupported(nh_type):
                continue
            for c in range(nh_spec_obj.count):
                obj = NexthopObject(node, parent, nh_spec_obj)
                self.Objs[node].update({obj.NexthopId: obj})
                if nh_type == "underlay":
                    self.__underlay_objs[node].update({obj.NexthopId: obj})
                if isV4Stack:
                    self.__v4objs[node][vpcid].append(obj)
                if isV6Stack:
                    self.__v6objs[node][vpcid].append(obj)
        if len(self.__v4objs[node][vpcid]):
            self.__v4iter[node][vpcid] = utils.rrobiniter(self.__v4objs[node][vpcid])
        if len(self.__v6objs[node][vpcid]):
            self.__v6iter[node][vpcid] = utils.rrobiniter(self.__v6objs[node][vpcid])
        self.__num_nh_per_vpc.append(nh_spec_obj.count)
        return

    def AddObjToDict(self, obj):
        self.Objs[obj.Node].update({obj.NexthopId: obj})
        return

    def DeleteObjFromDict(self, obj):
        self.Objs[obj.Node].pop(obj.NexthopId, None)
        return

    def CreateObjects(self, node):
        if (EzAccessStoreClient[node].IsDeviceOverlayRoutingEnabled()):
            logger.info("Skipping creating nexthop objects")
            return
        cookie = utils.GetBatchCookie(node)
        if utils.IsPipelineApulu():
            logger.info(f"Creating {len(self.__underlay_objs[node])} underlay {self.ObjType.name} Objects in {node}")
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__underlay_objs[node].values()))
            list(map(lambda x: x.SetHwHabitant(False), self.Objects(node)))
            list(map(lambda x: x.SetHwHabitant(True), self.__underlay_objs[node].values()))
        else:
            logger.info(f"Creating {len(self.Objects(node))} {self.ObjType.name} Objects in {node}")
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.Objects(node)))
            list(map(lambda x: x.SetHwHabitant(True), self.Objects(node)))
        api.client[node].Create(self.ObjType, msgs)
        return

    def GetGrpcReadAllMessage(self, node):
        grpcmsg = nh_pb2.NexthopGetRequest()
        return grpcmsg

    def ReadObjects(self, node):
        if utils.IsPipelineApulu():
            cfgObjects = self.__underlay_objs[node].values()
        else:
            cfgObjects = self.Objects(node)
        result = list(map(lambda x: x.Read(), cfgObjects))
        if not all(result):
            logger.critical(f"Reading {len(cfgObjects)} {self.ObjType.name} Objects FAILED in {node}")
            return False
        return True

    def DeleteObjects(self, node):
        if utils.IsPipelineApulu():
            cfgObjects = self.__underlay_objs[node].copy().values()
        else:
            cfgObjects = self.Objs[node].copy().values()
        logger.info(f"Deleting {len(cfgObjects)} {self.ObjType.name} Objects in {node}")
        result = list(map(lambda x: x.Delete(), cfgObjects))
        if not all(result):
            logger.info(f"Deleting {len(cfgObjects)} {self.ObjType.name} Objects FAILED in {node}")
            return False
        list(map(lambda x: x.SetHwHabitant(False), cfgObjects))
        return True

    def RestoreObjects(self, node):
        if utils.IsPipelineApulu():
            temp = self.__underlay_objs[node].copy().values()
        else:
            temp = self.Objs[node].copy().values()
        cfgObjects = list(filter(lambda x: not(x.IsInterim()), temp))
        logger.info(f"Restoring {len(cfgObjects)} {self.ObjType.name} Objects in {node}")
        result = list(map(lambda x: x.Create(), cfgObjects))
        if not all(result):
            logger.info(f"Restoring {len(cfgObjects)} {self.ObjType.name} Objects FAILED in {node}")
            return False
        list(map(lambda x: x.SetHwHabitant(True), cfgObjects))
        return True

    def UpdateObjects(self, node):
        if utils.IsPipelineApulu():
            cfgObjects = self.__underlay_objs[node].values()
        else:
            cfgObjects = self.Objects(node)
        logger.info(f"Updating {len(cfgObjects)} {self.ObjType.name} Objects in {node}")
        result = list(map(lambda x: x.Update(), cfgObjects))
        if not all(result):
            logger.info(f"Updating {len(cfgObjects)} {self.ObjType.name} Objects FAILED in {node}")
            return False
        return True

    def UpdateUnderlayObjects(self, node):
        if utils.IsDryRun(): return True
        cfgObjects = self.__underlay_objs[node].values()
        logger.info(f"Updating {len(cfgObjects)} underlay {self.ObjType.name} Objects in {node}")
        result = list(map(lambda x: utils.UpdateObject(x), cfgObjects))
        if not all(result):
            logger.info(f"Updating {len(cfgObjects)} underlay {self.ObjType.name} Objects FAILED in {node}")
            return False
        return True

    def RollbackUpdateObjects(self, node):
        if utils.IsPipelineApulu():
            cfgObjects = self.__underlay_objs[node].values()
        else:
            cfgObjects = self.Objects(node)
        logger.info(f"RollbackUpdate {len(cfgObjects)} {self.ObjType.name} Objects in {node}")
        result = list(map(lambda x: x.RollbackUpdate(), cfgObjects))
        if not all(result):
            logger.info(f"RollbackUpdate {len(cfgObjects)} {self.ObjType.name} Objects FAILED in {node}")
            return False
        return True

client = NexthopObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
