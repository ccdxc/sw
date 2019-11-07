#! /usr/bin/python3
import pdb
import enum

from infra.common.logging import logger

from apollo.config.store import Store

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base
from apollo.config.objects.interface import client as InterfaceClient
import apollo.config.objects.tunnel as tunnel

import nh_pb2 as nh_pb2

class NexthopObject(base.ConfigObjectBase):
    def __init__(self, parent, spec):
        super().__init__()
        self.SetBaseClassAttr()
        ################# PUBLIC ATTRIBUTES OF NEXTHOP OBJECT #####################
        self.NexthopId = next(resmgr.NexthopIdAllocator)
        self.GID('Nexthop%d'%self.NexthopId)
        self.VPC = parent
        nh_type = getattr(spec, 'type', 'ip')
        if nh_type == 'ip':
            self.__type = utils.NhType.IP
            self.PfxSel = parent.PfxSel
            self.IPAddr = {}
            self.IPAddr[0] = next(resmgr.NexthopIpV4AddressAllocator)
            self.IPAddr[1] = next(resmgr.NexthopIpV6AddressAllocator)
            self.VlanId = next(resmgr.NexthopVlanIdAllocator)
            self.MACAddr = resmgr.NexthopMacAllocator.get()
        elif nh_type == 'underlay':
            self.__type = utils.NhType.UNDERLAY
            self.L3Interface = InterfaceClient.GetL3UplinkInterface()
            self.underlayMACAddr = resmgr.NexthopMacAllocator.get()
        elif nh_type == 'overlay':
            self.__type = utils.NhType.OVERLAY
            self.TunnelId = 1 # TODO use iterator from tunnel
        else:
            self.__type = utils.NhType.NONE
        self.Show()
        return

    def __repr__(self):
        if self.__type == utils.NhType.IP:
            nh_str = "VPCId:%d|PfxSel:%d|IP:%s|Mac:%s|Vlan:%d" %\
                     (self.VPC.VPCId, self.PfxSel, self.IPAddr[self.PfxSel],
                     self.MACAddr, self.VlanId)
        elif self.__type == utils.NhType.UNDERLAY:
            nh_str = "L3IfID:%d|UnderlayMac:%s" %\
                     (self.L3Interface.InterfaceId, self.underlayMACAddr)
        elif self.__type == utils.NhType.OVERLAY:
            nh_str = "TunnelId:%d" % (self.TunnelId)
        else:
            nh_str = ""
        return "NexthopID:%d|Type:%s|%s" %\
               (self.NexthopId, self.__type, nh_str)

    def Show(self):
        logger.info("Nexthop object:", self)
        logger.info("- %s" % repr(self))
        return

    def SetBaseClassAttr(self):
        self.ObjType = api.ObjectTypes.NEXTHOP
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.NexthopId)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.NexthopId
        if self.__type == utils.NhType.IP:
            spec.IPNhInfo.VPCId = self.VPC.VPCId
            spec.IPNhInfo.Mac = self.MACAddr.getnum()
            spec.IPNhInfo.Vlan = self.VlanId
            utils.GetRpcIPAddr(self.IPAddr[self.PfxSel], spec.IPNhInfo.IP)
        elif self.__type == utils.NhType.UNDERLAY:
            spec.UnderlayNhInfo.L3InterfaceId = self.L3Interface.InterfaceId
            spec.UnderlayNhInfo.UnderlayMAC = self.underlayMACAddr.getnum()
        elif self.__type == utils.NhType.OVERLAY:
            spec.TunnelId = self.TunnelId
        return

    def IsUnderlay(self):
        if self.__type == utils.NhType.UNDERLAY:
            return True
        return False

class NexthopObjectClient:
    def __init__(self):
        def __isObjSupported():
            if utils.IsPipelineArtemis() or utils.IsPipelineApulu():
                return True
            return False

        self.__objs = dict()
        self.__v4objs = {}
        self.__v6objs = {}
        self.__v4iter = {}
        self.__v6iter = {}
        self.__num_nh_per_vpc = []
        self.__supported = __isObjSupported()
        return

    def Objects(self):
        return self.__objs.values()

    def GetNexthopObject(self, nexthopid):
        self.__objs.get(nexthopid, None).Show()
        return self.__objs.get(nexthopid, None)

    def GetV4Nexthop(self, vpcid):
        if len(self.__objs.values()):
            assert(len(self.__v4objs[vpcid]) != 0)
            return self.__v4iter[vpcid].rrnext()
        else:
            return None

    def GetV6Nexthop(self, vpcid):
        if len(self.__objs.values()):
            assert(len(self.__v6objs[vpcid]) != 0)
            return self.__v6iter[vpcid].rrnext()
        else:
            return None

    def GetNumNextHopPerVPC(self):
        return self.__num_nh_per_vpc

    def AssociateObjects(self):
        Store.SetNexthops(self.Objects())
        resmgr.CreateUnderlayNHAllocator()
        tunnel.client.AssociateObjects()

    def GenerateObjects(self, parent, vpc_spec_obj):
        if not self.__supported:
            return

        def __isNhFeatureSupported(nh_type):
            if nh_type == 'underlay':# or nh_type == 'underlay-ecmp':
                return utils.IsPipelineApulu()
            return not utils.IsPipelineApulu()

        vpcid = parent.VPCId
        isV4Stack = utils.IsV4Stack(parent.Stack)
        isV6Stack = utils.IsV6Stack(parent.Stack)
        self.__v4objs[vpcid] = []
        self.__v6objs[vpcid] = []
        self.__v4iter[vpcid] = None
        self.__v6iter[vpcid] = None

        if getattr(vpc_spec_obj, 'nexthop', None) == None:
            self.__num_nh_per_vpc.append(0)
            return

        for nh_spec_obj in vpc_spec_obj.nexthop:
            nh_type = getattr(nh_spec_obj, 'type', 'ip')
            if not __isNhFeatureSupported(nh_type):
                continue
            for c in range(nh_spec_obj.count):
                obj = NexthopObject(parent, nh_spec_obj)
                self.__objs.update({obj.NexthopId: obj})
                if isV4Stack:
                    self.__v4objs[vpcid].append(obj)
                if isV6Stack:
                    self.__v6objs[vpcid].append(obj)
        if len(self.__v4objs[vpcid]):
            self.__v4iter[vpcid] = utils.rrobiniter(self.__v4objs[vpcid])
        if len(self.__v6objs[vpcid]):
            self.__v6iter[vpcid] = utils.rrobiniter(self.__v6objs[vpcid])
        self.__num_nh_per_vpc.append(nh_spec_obj.count)
        return

    def CreateObjects(self):
        cookie = utils.GetBatchCookie()
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__objs.values()))
        api.client.Create(api.ObjectTypes.NEXTHOP, msgs)
        return

    def GetGrpcReadAllMessage(self):
        grpcmsg = nh_pb2.NexthopGetRequest()
        return grpcmsg

    def ReadObjects(self):
        msg = self.GetGrpcReadAllMessage()
        api.client.Get(api.ObjectTypes.NEXTHOP, [msg])
        return

client = NexthopObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
