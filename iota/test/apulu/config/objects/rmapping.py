#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from iota.test.apulu.config.store import Store

import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.utils as utils
import iota.test.apulu.config.agent.api as api
import iota.test.apulu.config.objects.base as base
import ipaddress

class RemoteMappingObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, tunobj, ipversion, count):
        super().__init__(api.ObjectTypes.MAPPING)

        ################# PUBLIC ATTRIBUTES OF REMOTE MAPPING OBJECT ##########
        if (hasattr(spec, 'id')):
            self.MappingId = spec.id
        else:
            self.MappingId = next(resmgr.RemoteMappingIdAllocator)
        self.GID('RemoteMapping%d'%self.MappingId)
        self.SUBNET = parent
        if (hasattr(spec, 'rmacaddr')):
            self.MACAddr = spec.rmacaddr
        else:
            self.MACAddr = resmgr.RemoteMappingMacAllocator.get()
        self.TunID = tunobj.Id
        self.TunIPAddr = tunobj.RemoteIPAddr
        self.HasDefaultRoute = False
        if tunobj.IsWorkload():
            self.MplsSlot = next(tunobj.RemoteVnicMplsSlotIdAllocator)
            self.Vnid = next(tunobj.RemoteVnicVxlanIdAllocator)
        else:
            self.MplsSlot = 0
            self.Vnid = 0
        self.TUNNEL = tunobj
        if ipversion == utils.IP_VERSION_6:
            self.IPAddr = parent.AllocIPv6Address();
            self.AddrFamily = 'IPV6'
            if self.SUBNET.V6RouteTable:
                self.HasDefaultRoute = self.SUBNET.V6RouteTable.HasDefaultRoute
        else:
            if getattr(spec, 'ripaddr', None) != None:
                self.IPAddr = ipaddress.IPv4Address(spec.ripaddr)
            else:
                self.IPAddr = parent.AllocIPv4Address();
            self.AddrFamily = 'IPV4'
            if self.SUBNET.V4RouteTable:
                self.HasDefaultRoute = self.SUBNET.V4RouteTable.HasDefaultRoute
        # Provider IP can be v4 or v6
        self.ProviderIPAddr, self.TunFamily = Store.GetProviderIPAddr(count)
        self.ProviderIP = str(self.ProviderIPAddr) # For testspec
        self.Label = 'NETWORKING'
        self.FlType = "MAPPING"
        self.IP = str(self.IPAddr) # For testspec
        self.TunIP = str(self.TunIPAddr) # For testspec
        self.AppPort = resmgr.TransportDstPort

        ################# PRIVATE ATTRIBUTES OF MAPPING OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "RemoteMappingID:%d|SubnetId:%d|VPCId:%d" %\
               (self.MappingId, self.SUBNET.SubnetId, self.SUBNET.VPC.VPCId)

    def Show(self):
        logger.info("RemoteMapping object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- IPAddr:%s|TunID:%u|TunIPAddr:%s|MAC:%s|Mpls:%d|Vxlan:%d|PIP:%s" %\
                (str(self.IPAddr), self.TunID, str(self.TunIPAddr), self.MACAddr.get(),
                self.MplsSlot, self.Vnid, self.ProviderIPAddr))
        return

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.flow.filters)

    def PopulateKey(self, grpcmsg):
        key = grpcmsg.Id.add()
        key.IPKey.VPCId = str.encode(str(self.SUBNET.VPC.VPCId))
        utils.GetRpcIPAddr(self.IPAddr, key.IPKey.IPAddr)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id.IPKey.VPCId = str.encode(str(self.SUBNET.VPC.VPCId))
        utils.GetRpcIPAddr(self.IPAddr, spec.Id.IPKey.IPAddr)
        spec.SubnetId = self.SUBNET.SubnetId
        spec.TunnelId = self.TunID
        spec.MACAddr = self.MACAddr.getnum()
        utils.GetRpcEncap(self.MplsSlot, self.Vnid, spec.Encap)
        if utils.IsPipelineArtemis():
            utils.GetRpcIPAddr(self.ProviderIPAddr, spec.ProviderIp)
        spec.TunnelId = self.TunID
        return


class RemoteMappingObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.MAPPING)
        return

    def GenerateObjects(self, node, parent, subnet_spec_obj):
        if getattr(subnet_spec_obj, 'rmap', None) == None:
            return

        isV4Stack = utils.IsV4Stack(parent.VPC.Stack)
        isV6Stack = utils.IsV6Stack(parent.VPC.Stack)
        if utils.IsPipelineApulu():
            tunnelAllocator = resmgr.UnderlayTunAllocator
        else:
            tunnelAllocator = resmgr.RemoteMplsVnicTunAllocator

        for rmap_spec_obj in subnet_spec_obj.rmap:
            c = 0
            v6c = 0
            v4c = 0
            while c < rmap_spec_obj.count:
                tunobj = tunnelAllocator.rrnext()
                if isV6Stack:
                    obj = RemoteMappingObject(parent, rmap_spec_obj, tunobj, utils.IP_VERSION_6, v6c)
                    self.Objs[node].update({obj.MappingId: obj})
                    c = c + 1
                    v6c = v6c + 1
                if c < rmap_spec_obj.count and isV4Stack:
                    obj = RemoteMappingObject(parent, rmap_spec_obj, tunobj, utils.IP_VERSION_4, v4c)
                    self.Objs[node].update({obj.MappingId: obj})
                    c = c + 1
                    v4c = v4c + 1
        return

client = RemoteMappingObjectClient()

def GetMatchingObjects(selectors):
    objs = []
    for obj in client.Objects():
        if obj.IsFilterMatch(selectors) == True:
            objs.append(obj)
    return objs
