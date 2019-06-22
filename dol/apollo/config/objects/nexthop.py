#! /usr/bin/python3
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import nh_pb2 as nh_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class NexthopObject(base.ConfigObjectBase):
    def __init__(self, parent, spec):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF SUBNET OBJECT #####################
        self.NexthopId = next(resmgr.NexthopIdAllocator)
        self.GID('Nexthop%d'%self.NexthopId)
        self.VPC = parent
        self.PfxSel = parent.PfxSel
        self.IPAddr = {}
        self.IPAddr[0] = next(resmgr.NexthopIpV4AddressAllocator)
        self.IPAddr[1] = next(resmgr.NexthopIpV6AddressAllocator)
        self.MacAddr = resmgr.NexthopMacAllocator.get()
        self.Vni = next(resmgr.NexthopVxlanIdAllocator)
        self.Show()
        return

    def __repr__(self):
        return "NexthopID:%d|VPCId:%d|PfxSel:%d|IP:%s" %\
               (self.NexthopId, self.VPC.VPCId, self.PfxSel, self.IPAddr[self.PfxSel])

    def GetGrpcCreateMessage(self):
        grpcmsg = nh_pb2.NexthopRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.NexthopId
        spec.Type = nh_pb2.NEXTHOP_TYPE_IP
        spec.IPNhInfo.VPCId = self.VPC.VPCId
        spec.IPNhInfo.Mac = self.MacAddr.getnum()
        spec.IPNhInfo.Vlan = self.Vni
        utils.GetRpcIPAddr(self.IPAddr[self.PfxSel], spec.IPNhInfo.IP)
        return grpcmsg

    def Show(self):
        logger.info("Nexthop object:", self)
        logger.info("- %s" % repr(self))
        return

    def SetupTestcaseConfig(self, obj):
        return

class NexthopObjectClient:
    def __init__(self):
        self.__objs = []
        self.__v4objs = {}
        self.__v6objs = {}
        self.__v4iter = {}
        self.__v6iter = {}
        return

    def Objects(self):
        return self.__objs

    def GetV4NexthopId(self, vpcid):
        if len(self.__objs):
            assert(len(self.__v4objs[vpcid]) != 0)
            return self.__v4iter[vpcid].rrnext().NexthopId
        else:
            return 0

    def GetV6NexthopId(self, vpcid):
        if len(self.__objs):
            assert(len(self.__v6objs[vpcid]) != 0)
            return self.__v6iter[vpcid].rrnext().NexthopId
        else:
            return 0

    def GetNumNextHopPerVPC(self):
        return self.__num_nh_per_vpc

    def GenerateObjects(self, parent, vpc_spec_obj):
        vpcid = parent.VPCId
        stack = parent.Stack
        self.__v4objs[vpcid] = []
        self.__v6objs[vpcid] = []
        self.__v4iter[vpcid] = None
        self.__v6iter[vpcid] = None
        self.__num_nh_per_vpc = 0

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

        for nh_spec_obj in vpc_spec_obj.nexthop:
            for c in range(nh_spec_obj.count):
                obj = NexthopObject(parent, nh_spec_obj)
                self.__objs.append(obj)
                if __is_v4stack():
                    self.__v4objs[vpcid].append(obj)
                if __is_v6stack():
                    self.__v6objs[vpcid].append(obj)
        if len(self.__v4objs[vpcid]):
            self.__v4iter[vpcid] = utils.rrobiniter(self.__v4objs[vpcid])
        if len(self.__v6objs[vpcid]):
            self.__v6iter[vpcid] = utils.rrobiniter(self.__v6objs[vpcid])
        self.__num_nh_per_vpc = nh_spec_obj.count
        return

    def CreateObjects(self):
        if utils.IsPipelineArtemis():
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
            api.client.Create(api.ObjectTypes.NEXTHOP, msgs)
        return

client = NexthopObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
