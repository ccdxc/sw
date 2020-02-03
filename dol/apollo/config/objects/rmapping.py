#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from apollo.config.store import client as EzAccessStoreClient

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.utils as utils
import apollo.config.agent.api as api
import apollo.config.objects.base as base
import ipaddress

class RemoteMappingObject(base.ConfigObjectBase):
    def __init__(self, node, parent, spec, tunobj, ipversion, count):
        super().__init__(api.ObjectTypes.MAPPING, node)
        parent.AddChild(self)
        if (EzAccessStoreClient[node].IsDeviceOverlayRoutingEnabled()):
            self.SetOrigin(topo.OriginTypes.DISCOVERED)
        ################# PUBLIC ATTRIBUTES OF REMOTE MAPPING OBJECT ##########
        if (hasattr(spec, 'id')):
            self.MappingId = spec.id
        else:
            self.MappingId = next(ResmgrClient[node].RemoteMappingIdAllocator)
        self.GID('RemoteMapping%d'%self.MappingId)
        self.SUBNET = parent
        if (hasattr(spec, 'rmacaddr')):
            self.MACAddr = spec.rmacaddr
        else:
            self.MACAddr = ResmgrClient[node].RemoteMappingMacAllocator.get()
        self.TunID = tunobj.Id
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
        self.ProviderIPAddr, self.TunFamily = EzAccessStoreClient[node].GetProviderIPAddr(count)
        self.ProviderIP = str(self.ProviderIPAddr) # For testspec
        self.Label = 'NETWORKING'
        self.FlType = "MAPPING"
        self.IP = str(self.IPAddr) # For testspec
        self.AppPort = ResmgrClient[node].TransportDstPort

        ################# PRIVATE ATTRIBUTES OF MAPPING OBJECT #####################
        self.DeriveOperInfo()
        self.Show()
        return

    def __repr__(self):
        return "RemoteMappingID:%d|Subnet: %s |VPC: %s " %\
               (self.MappingId, self.SUBNET.UUID, self.SUBNET.VPC.UUID)

    def Show(self):
        logger.info("RemoteMapping object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- IPAddr:%s|TEP: %s |TunIPAddr:%s|MAC:%s|Mpls:%d|Vxlan:%d|PIP:%s" %\
                (str(self.IPAddr), self.TUNNEL.UUID, str(self.TUNNEL.RemoteIPAddr), self.MACAddr,
                self.MplsSlot, self.Vnid, self.ProviderIPAddr))
        return

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.flow.filters)

    def PopulateKey(self, grpcmsg):
        key = grpcmsg.Id.add()
        key.IPKey.VPCId = self.SUBNET.VPC.GetKey()
        utils.GetRpcIPAddr(self.IPAddr, key.IPKey.IPAddr)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id.IPKey.VPCId = self.SUBNET.VPC.GetKey()
        utils.GetRpcIPAddr(self.IPAddr, spec.Id.IPKey.IPAddr)
        spec.SubnetId = self.SUBNET.GetKey()
        spec.TunnelId = self.TUNNEL.GetKey()
        spec.MACAddr = self.MACAddr.getnum()
        utils.GetRpcEncap(self.Node, self.MplsSlot, self.Vnid, spec.Encap)
        if utils.IsPipelineArtemis():
            utils.GetRpcIPAddr(self.ProviderIPAddr, spec.ProviderIp)
        return

    def ValidateSpec(self, spec):
        if spec.Id.IPKey.VPCId != self.SUBNET.VPC.GetKey():
            return False
        if not utils.ValidateRpcIPAddr(self.IPAddr, spec.Id.IPKey.IPAddr):
            return False
        if spec.MACAddr != self.MACAddr.getnum():
            return False
        return True

    def GetDependees(self, node):
        """
        depender/dependent - remote mapping
        dependee - tunnel
        """
        dependees = [ self.TUNNEL ]
        return dependees

    def RestoreNotify(self, cObj):
        logger.info("Notify %s for %s creation" % (self, cObj))
        if not self.IsHwHabitant():
            logger.info(" - Skipping notification as %s already deleted" % self)
            return
        logger.info(" - Linking %s to %s " % (cObj, self))
        if cObj.ObjType == api.ObjectTypes.TUNNEL:
            self.TunID = cObj.Id
        else:
            logger.error(" - ERROR: %s not handling %s restoration" %\
                         (self.ObjType.name, cObj.ObjType))
            assert(0)
        # self.Update()
        # Update is not supported on Mapping objects. hence deleting and re-adding them
        self.Delete()
        self.Create()
        return

    def DeleteNotify(self, dObj):
        logger.info("Notify %s for %s deletion" % (self, dObj))
        if not self.IsHwHabitant():
            logger.info(" - Skipping notification as %s already deleted" % self)
            return
        logger.info(" - Unlinking %s from %s " % (dObj, self))
        if dObj.ObjType == api.ObjectTypes.TUNNEL:
            self.TunID = 0
        else:
            logger.error(" - ERROR: %s not handling %s deletion" %\
                         (self.ObjType.name, dObj.ObjType))
            assert(0)
        # self.Update()
        # Update is not supported on Mapping objects. hence deleting and re-adding them
        self.Delete()
        self.Create()
        return

class RemoteMappingObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.MAPPING)
        return

    def PdsctlRead(self, node):
        # pdsctl show not supported for remote mapping
        return

    def GenerateObjects(self, node, parent, subnet_spec_obj):
        if getattr(subnet_spec_obj, 'rmap', None) == None:
            return

        isV4Stack = utils.IsV4Stack(parent.VPC.Stack)
        isV6Stack = utils.IsV6Stack(parent.VPC.Stack)
        if utils.IsPipelineApulu():
            tunnelAllocator = ResmgrClient[node].UnderlayTunAllocator
        else:
            tunnelAllocator = ResmgrClient[node].RemoteMplsVnicTunAllocator

        for rmap_spec_obj in subnet_spec_obj.rmap:
            c = 0
            v6c = 0
            v4c = 0
            while c < rmap_spec_obj.count:
                tunobj = tunnelAllocator.rrnext()
                if isV6Stack:
                    obj = RemoteMappingObject(node, parent, rmap_spec_obj, tunobj, utils.IP_VERSION_6, v6c)
                    self.Objs[node].update({obj.MappingId: obj})
                    c = c + 1
                    v6c = v6c + 1
                if c < rmap_spec_obj.count and isV4Stack:
                    obj = RemoteMappingObject(node, parent, rmap_spec_obj, tunobj, utils.IP_VERSION_4, v4c)
                    self.Objs[node].update({obj.MappingId: obj})
                    c = c + 1
                    v4c = v4c + 1
        return

client = RemoteMappingObjectClient()

def GetMatchingObjects(selectors, node):
    objs = []
    for obj in client.Objects(node):
        if obj.IsFilterMatch(selectors) == True:
            objs.append(obj)
    return objs
