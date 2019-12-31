#! /usr/bin/python3
import pdb
from collections import defaultdict

from infra.common.logging import logger

from iota.test.apulu.config.store import Store

import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.agent.api as api
import iota.test.apulu.config.utils as utils
import iota.test.apulu.config.objects.base as base
from iota.test.apulu.config.objects.interface import client as InterfaceClient
import iota.test.apulu.config.objects.tunnel as tunnel

import nh_pb2 as nh_pb2

class NexthopObject(base.ConfigObjectBase):
    def __init__(self, node, parent, spec):
        super().__init__(api.ObjectTypes.NEXTHOP)
        ################# PUBLIC ATTRIBUTES OF NEXTHOP OBJECT #####################
        if (hasattr(spec, 'id')):
            self.NexthopId = spec.id
        else:
            self.NexthopId = next(resmgr.NexthopIdAllocator)
        self.GID('Nexthop%d'%self.NexthopId)
        self.VPC = parent
        nh_type = getattr(spec, 'type', 'ip')
        self.DualEcmp = utils.IsDualEcmp(spec)
        if nh_type == 'ip':
            self.__type = utils.NhType.IP
            self.PfxSel = parent.PfxSel
            self.IPAddr = {}
            self.IPAddr[0] = next(resmgr.NexthopIpV4AddressAllocator)
            self.IPAddr[1] = next(resmgr.NexthopIpV6AddressAllocator)
            self.VlanId = next(resmgr.NexthopVlanIdAllocator)
            if (hasattr(spec, 'macaddress')):
                self.MACAddr = spec.macaddress
            else:
                self.MACAddr = resmgr.NexthopMacAllocator.get()
        elif nh_type == 'underlay':
            self.__type = utils.NhType.UNDERLAY
            self.L3Interface = InterfaceClient.GetL3UplinkInterface(node)
            if (hasattr(spec, 'macaddress')):
                self.underlayMACAddr = spec.macaddress
            else:
                self.underlayMACAddr = resmgr.NexthopMacAllocator.get()
        elif nh_type == 'overlay':
            self.__type = utils.NhType.OVERLAY
            if self.DualEcmp:
                self.TunnelId = resmgr.UnderlayECMPTunAllocator.rrnext().Id
            if (hasattr(spec, 'tunnelid')):
                self.TunnelId = spec.tunnelid
            else:
                self.TunnelId = resmgr.UnderlayTunAllocator.rrnext().Id
        else:
            self.__type = utils.NhType.NONE
        self.Show()
        return

    def __repr__(self):
        if self.__type == utils.NhType.IP:
            nh_str = "VPCId:%d|PfxSel:%d|IP:%s|Mac:%s|Vlan:%d" %\
                     (self.VPC.VPCId, self.PfxSel, self.IPAddr[self.PfxSel],
                     self.MACAddr.get(), self.VlanId)
        elif self.__type == utils.NhType.UNDERLAY:
            nh_str = "L3IfID:%d|UnderlayMac:%s" %\
                     (self.L3Interface.InterfaceId, self.underlayMACAddr.get())
        elif self.__type == utils.NhType.OVERLAY:
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
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.NexthopId)
        return

    def FillSpec(self, spec):
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

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        self.FillSpec(spec)
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.NexthopId:
            return False
        if self.__type == utils.NhType.IP:
            if spec.IPNhInfo.Mac != self.MACAddr.getnum():
                return False
            if spec.IPNhInfo.Vlan != self.VlanId:
                return False
            if spec.IPNhInfo.VPCId != self.VPC.VPCId:
                return False
            if utils.ValidateRpcIPAddr(self.IPAddr[self.PfxSel], spec.IPNhInfo.IP) == False:
                return False
        elif self.__type == utils.NhType.UNDERLAY:
            if spec.UnderlayNhInfo.L3InterfaceId != self.L3Interface.InterfaceId:
                return False
            if spec.UnderlayNhInfo.UnderlayMAC != self.underlayMACAddr.getnum():
                return False
        elif self.__type != utils.NhType.OVERLAY:
            if spec.TunnelId != self.TunnelId:
                return False
        return True

    def IsUnderlay(self):
        if self.__type == utils.NhType.UNDERLAY:
            return True
        return False

    def IsUnderlayEcmp(self):
        if self.__type == utils.NhType.UNDERLAY_ECMP:
            return True
        return False

    def IsOverlay(self):
        if self.__type == utils.NhType.OVERLAY:
            return True
        return False

    def IsOverlayEcmp(self):
        if self.__type == utils.NhType.OVERLAY_ECMP:
            return True
        return False

class NexthopObjectClient:
    def __init__(self):
        def __isObjSupported():
            if utils.IsPipelineArtemis() or utils.IsPipelineApulu():
                return True
            return False

        self.__objs = defaultdict(dict)
        self.__underlay_objs = defaultdict(dict)
        self.__v4objs = defaultdict(dict)
        self.__v6objs = defaultdict(dict)
        self.__v4iter = defaultdict(dict)
        self.__v6iter = defaultdict(dict)
        self.__num_nh_per_vpc = []
        self.__supported = __isObjSupported()
        return

    def Objects(self, node):
        return self.__objs[node].values()

    def GetNexthopObject(self, nexthopid):
        return self.__objs.get(nexthopid, None)

    def GetV4Nexthop(self, node, vpcid):
        if len(self.__objs[node].values()):
            assert(len(self.__v4objs[node][vpcid]) != 0)
            return self.__v4iter[node][vpcid].rrnext()
        else:
            return None

    def GetV6Nexthop(self, node, vpcid):
        if len(self.__objs[node].values()):
            assert(len(self.__v6objs[node][vpcid]) != 0)
            return self.__v6iter[node][vpcid].rrnext()
        else:
            return None

    def GetNumNextHopPerVPC(self):
        return self.__num_nh_per_vpc

    def AssociateObjects(self, node):
        Store.SetNexthops(self.Objects(node))
        resmgr.CreateUnderlayNHAllocator()
        resmgr.CreateOverlayNHAllocator()
        resmgr.CreateDualEcmpNhAllocator()
        tunnel.client.AssociateObjects(node)

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
                self.__objs[node].update({obj.NexthopId: obj})
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

    def CreateObjects(self, node):
        cookie = utils.GetBatchCookie(node)
        if utils.IsPipelineApulu():
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(node, cookie), self.__underlay_objs[node].values()))
        else:
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(node, cookie), self.__objs[node].values()))
        api.client[node].Create(api.ObjectTypes.NEXTHOP, msgs)
        return

    def GetGrpcReadAllMessage(self, node):
        grpcmsg = nh_pb2.NexthopGetRequest()
        return grpcmsg

    def ReadObjects(self, node):
        if len(self.__objs[node].values()) == 0:
            return
        msg = self.GetGrpcReadAllMessage(node)
        resp = api.client[node].Get(api.ObjectTypes.NEXTHOP, [msg])
        # TODO: Fix get all for nh, currently not supported
        # nh read is based on id and type only
        if False:
            result = self.ValidateObjects(resp)
            if result is False:
                logger.critical("NEXTHOP object validation failed!!!")
                sys.exit(1)
        return

    def ValidateObjects(self, getResp):
        if utils.IsDryRun(): return True
        for obj in getResp:
            if not utils.ValidateGrpcResponse(obj):
                logger.error("NEXTHOP get request failed for ", obj)
                return False
            for resp in obj.Response:
                spec = resp.Spec
                nh = self.GetNexthopObject(spec.Id)
                if not utils.ValidateObject(nh, resp):
                    logger.error("NEXTHOP validation failed for ", obj)
                    vnic.Show()
                    return False
        return True

client = NexthopObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
