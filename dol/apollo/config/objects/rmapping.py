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
    def __init__(self, parent, spec, encap, tunobj):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF MAPPING OBJECT #####################
        self.MappingId = next(resmgr.RemoteMappingIdAllocator)
        self.GID('RemoteMapping%d'%self.MappingId)
        self.IPAddr = parent.AllocIPAddress();
        self.SUBNET = parent
        self.MacAddr = resmgr.RemoteMappingMacAllocator.get()
        self.Encap = tunobj.Encap
        self.TunIP = tunobj.RemoteIP
        self.MplsSlot =  next(tunobj.RemoteVnicMplsSlotIdAllocator)
        if self.Encap == tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_1:
            self.fwtype = 'INTERNET'
        else:
            self.fwtype = 'VNIC'
        if self.IPAddr.version == 4:
            self.af = 'IPV4'
        else:
            self.af = 'IPV6'
        self.label = 'NETWORKING'

        ################# PRIVATE ATTRIBUTES OF MAPPING OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "RemoteMappingID:%d/SubnetId:%d" %\
               (self.MappingId, self.SUBNET.SubnetId)

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.mapping.filters)

    def GetGrpcCreateMessage(self):
        grpcmsg = mapping_pb2.MappingRequest()
        spec = grpcmsg.Request.add()
        spec.Id.PCNId = self.SUBNET.PCN.PCNId
        if self.IPAddr.version == 4:
            spec.Id.IPAddr.V4Addr = int(self.IPAddr)
            spec.Id.IPAddr.Af = types_pb2.IP_AF_INET
        else:
            spec.Id.IPAddr.V6Addr = self.IPAddr.packed
            spec.Id.IPAddr.Af = types_pb2.IP_AF_INET6
        spec.SubnetId = self.SUBNET.SubnetId
        spec.TunnelId = int(self.TunIP)
        spec.MACAddr = self.MacAddr.getnum()
        spec.Encap.type = self.Encap
        spec.Encap.value.MPLSTag  = self.MplsSlot
        return grpcmsg

    def Show(self):
        logger.info("RemoteMapping object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- IP address %s, TUN IP %s" % (str(self.IPAddr), str(self.TunIP)))
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

        self.__vnic_tunnel = utils.rrobiniter(Store.GetTunnelsMplsOverUdp2())
        self.__internet_tunnel = utils.rrobiniter(Store.GetTunnelsMplsOverUdp1())

        for rmap_spec_obj in subnet_spec_obj.rmap:
            encap = utils.GetTunnelEncapType(rmap_spec_obj.encap)
            for c in range (0, rmap_spec_obj.count):
                if  encap == tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_1:
                    tunobj = self.__internet_tunnel.rrnext()
                else:
                    tunobj = self.__vnic_tunnel.rrnext()
                obj = RemoteMappingObject(parent, subnet_spec_obj, encap, tunobj)
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
        if obj.IsFilterMatch(selectors):
            objs.append(obj)
    return objs
