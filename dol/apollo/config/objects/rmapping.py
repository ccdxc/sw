#! /usr/bin/python3
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.objects.utils as utils
import apollo.config.agent.api as api
import mapping_pb2 as mapping_pb2
import types_pb2 as types_pb2
import tunnel_pb2 as tunnel_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class RemoteMappingObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, encap, tunobj, ipversion):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF MAPPING OBJECT #####################
        self.MappingId = next(resmgr.RemoteMappingIdAllocator)
        self.GID('RemoteMapping%d'%self.MappingId)
        self.SUBNET = parent
        self.MACAddr = resmgr.RemoteMappingMacAllocator.get()
        self.Encap = tunobj.Encap
        self.TunIPAddr = tunobj.RemoteIPAddr
        self.MplsSlot =  next(tunobj.RemoteVnicMplsSlotIdAllocator)
        if self.Encap == tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_1:
            self.FwType = 'INTERNET'
        else:
            self.FwType = 'VNIC'
        if ipversion == utils.IP_VERSION_6:
            self.IPAddr = parent.AllocIPv6Address();
            self.AddrFamily = 'IPV6'
        else:
            self.IPAddr = parent.AllocIPv4Address();
            self.AddrFamily = 'IPV4'
        self.Label = 'NETWORKING'
        self.FlType = "MAPPING"
        self.IP = str(self.IPAddr) # For testspec
        self.TunIP = str(self.TunIPAddr) # For testspec

        ################# PRIVATE ATTRIBUTES OF MAPPING OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "RemoteMappingID:%d|SubnetId:%d|PCNId:%d" %\
               (self.MappingId, self.SUBNET.SubnetId, self.SUBNET.PCN.PCNId)

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.flow.filters)

    def GetGrpcCreateMessage(self):
        grpcmsg = mapping_pb2.MappingRequest()
        spec = grpcmsg.Request.add()
        spec.Id.PCNId = self.SUBNET.PCN.PCNId
        utils.GetRpcIPAddr(self.IPAddr, spec.Id.IPAddr)
        spec.SubnetId = self.SUBNET.SubnetId
        spec.TunnelId = int(self.TunIPAddr)
        spec.MACAddr = self.MACAddr.getnum()
        spec.Encap.type = self.Encap
        spec.Encap.value.MPLSTag  = self.MplsSlot
        return grpcmsg

    def Show(self):
        logger.info("RemoteMapping object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- IPAddr:%s|TunIPAddr:%s|MAC:%s" %\
                (str(self.IPAddr), str(self.TunIPAddr), self.MACAddr))
        return

    def SetupTestcaseConfig(self, obj):
        return

class RemoteMappingObjectClient:
    def __init__(self):
        self.__objs = []
        self.__vnic_tunnel = None
        self.__internet_tunnel = None
        self.__tunip = 0
        self.__mplsslot = 0
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, parent, subnet_spec_obj):
        if getattr(subnet_spec_obj, 'rmap', None) == None:
            return
        stack = parent.PCN.Stack
        self.__vnic_tunnel = utils.rrobiniter(Store.GetTunnelsMplsOverUdp2())
        self.__internet_tunnel = utils.rrobiniter(Store.GetTunnelsMplsOverUdp1())

        for rmap_spec_obj in subnet_spec_obj.rmap:
            encap = utils.GetTunnelEncapType(rmap_spec_obj.encap)
            for c in range (0, rmap_spec_obj.count):
                if  encap == tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_1:
                    tunobj = self.__internet_tunnel.rrnext()
                else:
                    tunobj = self.__vnic_tunnel.rrnext()
                if stack == "dual" or stack == 'ipv6':
                    obj = RemoteMappingObject(parent, subnet_spec_obj, encap, tunobj, utils.IP_VERSION_6)
                    self.__objs.append(obj)
                if stack == "dual" or stack == 'ipv4':
                    obj = RemoteMappingObject(parent, subnet_spec_obj, encap, tunobj, utils.IP_VERSION_4)
                    self.__objs.append(obj)
        return

    def CreateObjects(self):
        if len(self.__objs) == 0:
            return
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.MAPPING, msgs)
        return

client = RemoteMappingObjectClient()

def GetMatchingObjects(selectors):
    objs = []
    for obj in client.Objects():
        if obj.IsFilterMatch(selectors) == True:
            objs.append(obj)
    return objs
