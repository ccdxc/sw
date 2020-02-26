#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from apollo.config.store import client as EzAccessStoreClient
from apollo.config.store import EzAccessStore

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr
from apollo.config.agent.api import ObjectTypes as ObjectTypes
import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.objects.base as base

import tunnel_pb2 as tunnel_pb2
import ipaddress
import copy

class TunnelStatus(base.StatusObjectBase):
    def __init__(self):
        super().__init__(ObjectTypes.TUNNEL)

class TunnelObject(base.ConfigObjectBase):
    def __init__(self, node, parent, spec, local):
        super().__init__(ObjectTypes.TUNNEL, node)
        self.__spec = spec
        if (hasattr(spec, 'id')):
            self.Id = spec.id
        else:
            self.Id = next(ResmgrClient[node].TunnelIdAllocator)
        self.GID("Tunnel%d"%self.Id)
        self.UUID = utils.PdsUuid(self.Id, self.ObjType)
        # TODO: Tunnel gets generated from VPC / DEVICE. Fix this
        self.DEVICE = parent
        self.__nhtype = topo.NhType.NONE
        if utils.IsDol() and self.DEVICE.OverlayRoutingEn:
            self.SetOrigin(topo.OriginTypes.DISCOVERED)
        ################# PUBLIC ATTRIBUTES OF TUNNEL OBJECT #####################
        if (hasattr(spec, 'srcaddr')):
            self.LocalIPAddr = ipaddress.IPv4Address(spec.srcaddr)
        else:
            self.LocalIPAddr = self.DEVICE.IPAddr
        self.EncapValue = 0
        self.Nat = False
        self.NexthopId = 0
        self.NEXTHOP = None
        self.NexthopGroupId = 0
        self.NEXTHOPGROUP = None
        if (hasattr(spec, 'nat')):
            self.Nat = spec.nat
        if local == True:
            self.RemoteIPAddr = self.LocalIPAddr
            self.Type = tunnel_pb2.TUNNEL_TYPE_NONE
        else:
            self.Type = utils.GetTunnelType(spec.type)
            if self.Type == tunnel_pb2.TUNNEL_TYPE_WORKLOAD:
                self.RemoteIPAddr = next(ResmgrClient[node].TepIpAddressAllocator)
                self.RemoteVnicMplsSlotIdAllocator = ResmgrClient[node].CreateRemoteVnicMplsSlotAllocator()
                self.RemoteVnicVxlanIdAllocator = ResmgrClient[node].CreateRemoteVnicVxlanIdAllocator()
            elif self.Type == tunnel_pb2.TUNNEL_TYPE_IGW:
                self.RemoteIPAddr = next(ResmgrClient[node].TepIpAddressAllocator)
                if self.DEVICE.IsEncapTypeMPLS():
                    self.EncapValue = next(ResmgrClient[node].IGWMplsSlotIdAllocator)
                else:
                    self.EncapValue = next(ResmgrClient[node].IGWVxlanIdAllocator)
            elif self.Type == tunnel_pb2.TUNNEL_TYPE_SERVICE:
                self.RemoteIPAddr = next(ResmgrClient[node].TepIpv6AddressAllocator)
                if hasattr(spec, "remote") and spec.remote is True:
                    self.Remote = True
                    self.RemoteServicePublicIP = next(ResmgrClient[node].RemoteSvcTunIPv4Addr)
                    self.RemoteServiceEncap = next(ResmgrClient[node].IGWVxlanIdAllocator)
                else:
                    self.Remote = False
                self.EncapValue = next(ResmgrClient[node].IGWVxlanIdAllocator)
            else:
                if utils.IsV4Stack(self.DEVICE.Stack):
                    if getattr(spec, 'dstaddr', None) != None:
                        self.RemoteIPAddr = ipaddress.IPv4Address(spec.dstaddr)
                    else:
                        self.RemoteIPAddr = next(ResmgrClient[node].TepIpAddressAllocator)
                else:
                    self.RemoteIPAddr = next(ResmgrClient[node].TepIpv6AddressAllocator)
                # nexthop / nh_group association happens later
                print("type: ", spec.type)
                if spec.type == 'underlay':
                    self.__nhtype = topo.NhType.UNDERLAY
                    self.NEXTHOP = None
                    if hasattr(spec, 'nhid'):
                        self.NexthopId = spec.nhid
                    else:
                        self.NexthopId = None
                elif spec.type == 'underlay-ecmp':
                    self.__nhtype = topo.NhType.UNDERLAY_ECMP
                    self.NEXTHOPGROUP = None
        self.RemoteIP = str(self.RemoteIPAddr) # for testspec
        if getattr(spec, 'macaddress', None) != None:
            self.MACAddr = spec.macaddress
        else:
            self.MACAddr = ResmgrClient[node].TepMacAllocator.get()
        self.Mutable = utils.IsUpdateSupported()

        ################# PRIVATE ATTRIBUTES OF TUNNEL OBJECT #####################
        self.Status = TunnelStatus()
        self.DeriveOperInfo()
        self.Show()
        return

    def Dup(self):
        dupObj = copy.copy(self)
        dupObj.Id = next(ResmgrClient[self.Node].TunnelIdAllocator) + Resmgr.BaseDuplicateIdAllocator
        dupObj.GID("DupTunnel%d"%dupObj.Id)
        dupObj.UUID = utils.PdsUuid(dupObj.Id, dupObj.ObjType)
        dupObj.Interim = True
        self.Duplicate = dupObj
        return dupObj

    def __repr__(self):
        remote = ""
        if hasattr(self, "Remote") and self.Remote is True:
            remote = " Remote:%s"% (self.Remote)
        return "TEP: %s |LocalIPAddr:%s|RemoteIPAddr:%s|TunnelType:%s%s|" \
               "EncapValue:%d|Nat:%s|Mac:%s|NhType:%s|NexthopId:%d" % \
               (self.UUID, self.LocalIPAddr, self.RemoteIPAddr,
               utils.GetTunnelTypeString(self.Type), remote, self.EncapValue,
               self.Nat, self.MACAddr, self.__nhtype, (self.NexthopId if self.NexthopId != None else 0))

    def Show(self):
        logger.info("Tunnel Object: %s" % self)
        logger.info("- %s" % repr(self))
        self.Status.Show()
        return

    def UpdateAttributes(self):
        if self.LocalIPAddr != self.RemoteIPAddr:
            if self.Type == tunnel_pb2.TUNNEL_TYPE_WORKLOAD:
                self.RemoteIPAddr = next(ResmgrClient[self.Node].TepIpAddressAllocator)
            elif self.Type == tunnel_pb2.TUNNEL_TYPE_IGW:
                self.RemoteIPAddr = next(ResmgrClient[self.Node].TepIpAddressAllocator)
            else:
                if utils.IsV4Stack(self.DEVICE.Stack):
                    self.RemoteIPAddr = next(ResmgrClient[self.Node].TepIpAddressAllocator)
                else:
                    self.RemoteIPAddr = next(ResmgrClient[self.Node].TepIpv6AddressAllocator)
            if self.IsUnderlay():
                self.NEXTHOP = ResmgrClient[self.Node].UnderlayNHAllocator.rrnext()
                self.NexthopId = self.NEXTHOP.NexthopId
            elif self.IsUnderlayEcmp():
                self.NEXTHOPGROUP = ResmgrClient[self.Node].UnderlayNhGroupAllocator.rrnext()
                self.NexthopGroupId = self.NEXTHOPGROUP.Id
        self.RemoteIP = str(self.RemoteIPAddr) # for testspec
        self.MACAddr = ResmgrClient[self.Node].TepMacAllocator.get()
        return

    def RollbackAttributes(self):
        attrlist = ["RemoteIPAddr", "NEXTHOP", "NexthopId", "NEXTHOPGROUP", "NexthopGroupId", "RemoteIP", "MACAddr"]
        self.RollbackMany(attrlist)
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.VPCId = utils.PdsUuid.GetUUIDfromId(0, ObjectTypes.VPC) # TODO: Create Underlay VPC
        utils.GetRpcEncap(self.Node, self.EncapValue, self.EncapValue, spec.Encap)
        spec.Type = self.Type
        utils.GetRpcIPAddr(self.LocalIPAddr, spec.LocalIP)
        utils.GetRpcIPAddr(self.RemoteIPAddr, spec.RemoteIP)
        spec.Nat = self.Nat
        # TODO: Fix mac addr in testspec
        if not utils.IsPipelineApollo():
            spec.MACAddress = self.MACAddr.getnum()
        if utils.IsServiceTunnelSupported():
            if self.Type is tunnel_pb2.TUNNEL_TYPE_SERVICE and self.Remote is True:
                spec.RemoteService = self.Remote
                utils.GetRpcIPAddr(self.RemoteServicePublicIP, spec.RemoteServicePublicIP)
                utils.GetRpcEncap(self.Node, self.RemoteServiceEncap, self.RemoteServiceEncap, spec.RemoteServiceEncap)
        if self.IsUnderlay():
            spec.NexthopId = utils.PdsUuid.GetUUIDfromId(self.NexthopId, ObjectTypes.NEXTHOP)
        elif self.IsUnderlayEcmp():
            spec.NexthopGroupId = utils.PdsUuid.GetUUIDfromId(self.NexthopGroupId, ObjectTypes.NEXTHOPGROUP)
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if utils.ValidateTunnelEncap(self.Node, self.EncapValue, spec.Encap) == False:
            return False
        if utils.ValidateRpcIPAddr(self.LocalIPAddr, spec.LocalIP) == False:
            return False
        if utils.ValidateRpcIPAddr(self.RemoteIPAddr, spec.RemoteIP) == False:
            return False
        if spec.Type != self.Type:
            return False
        if spec.Nat != self.Nat:
            return False
        if not utils.IsPipelineApollo():
            if spec.MACAddress != self.MACAddr.getnum():
                return False
        if utils.IsServiceTunnelSupported():
            if self.Type is tunnel_pb2.TUNNEL_TYPE_SERVICE and self.Remote is True:
                if spec.RemoteService != self.Remote:
                    return False
                if utils.ValidateRpcIPAddr(self.RemoteServicePublicIP, spec.RemoteServicePublicIP) == False:
                    return False
                if utils.ValidateTunnelEncap(self.Node, self.RemoteServiceEncap, spec.RemoteServiceEncap) == False:
                    return False
        return True

    def ValidateYamlSpec(self, spec):
        if  utils.GetYamlSpecAttr(spec, 'id') != self.GetKey():
            return False
        return True

    def GetDependees(self, node):
        """
        depender/dependent - tunnel
        dependee - nexthop, & nexthop_group
        """
        # TODO: nh & nhg will be linked later
        dependees = [ ]
        if self.NEXTHOP:
            dependees.append(self.NEXTHOP)
        if self.NEXTHOPGROUP:
            dependees.append(self.NEXTHOPGROUP)
        return dependees

    def RestoreNotify(self, cObj):
        logger.info("Notify %s for %s creation" % (self, cObj))
        if not self.IsHwHabitant():
            logger.info(" - Skipping notification as %s already deleted" % self)
            return
        logger.info(" - Linking %s to %s " % (cObj, self))
        if cObj.ObjType == ObjectTypes.NEXTHOP:
            self.NexthopId = cObj.NexthopId
        elif cObj.ObjType == ObjectTypes.NEXTHOPGROUP:
            self.NexthopGroupId = cObj.Id
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
        if dObj.ObjType == ObjectTypes.NEXTHOP:
            self.NexthopId = dObj.Duplicate.NexthopId
        elif dObj.ObjType == ObjectTypes.NEXTHOPGROUP:
            self.NexthopGroupId = dObj.Duplicate.Id
        else:
            logger.error(" - ERROR: %s not handling %s deletion" %\
                         (self.ObjType.name, dObj.ObjType))
            assert(0)
        self.SetDirty(True)
        self.CommitUpdate()
        return

    def IsWorkload(self):
        if self.Type == tunnel_pb2.TUNNEL_TYPE_WORKLOAD:
            return True
        return False

    def IsIgw(self):
        if self.Type == tunnel_pb2.TUNNEL_TYPE_IGW:
            return True
        return False

    def IsSvc(self):
        if self.Type == tunnel_pb2.TUNNEL_TYPE_SERVICE:
            return True
        return False

    def IsNat(self):
        if self.Nat is True:
            return True
        return False

    def IsUnderlay(self):
        if self.__nhtype == topo.NhType.UNDERLAY:
            return True
        return False

    def IsUnderlayEcmp(self):
        if self.__nhtype == topo.NhType.UNDERLAY_ECMP:
            return True
        return False

class TunnelObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(ObjectTypes.TUNNEL, Resmgr.MAX_TUNNEL)
        return

    def AddObject(self, node, obj):
        self.Objs[node].update({obj.Id: obj})
        return

    def GetTunnelObject(self, node, tunnelid):
        return self.GetObjectByKey(node, tunnelid)

    def AssociateObjects(self, node):
        logger.info(f"Filling nexthops on {node}")
        NhClient = EzAccessStore.GetConfigClient(ObjectTypes.NEXTHOP)
        for tun in self.Objects(node):
            if tun.IsUnderlay():
                if tun.NexthopId == None:
                    nhObj = ResmgrClient[node].UnderlayNHAllocator.rrnext()
                    tun.NEXTHOP = nhObj
                    tun.NexthopId = nhObj.NexthopId
                    logger.info("Linking %s - %s" % (tun, nhObj))
                    nhObj.AddDependent(tun)
                elif tun.NEXTHOP == None:
                    logger.info("Linking Tunnel%d with Nexthop%d" %
                                (tun.Id, tun.NexthopId))
                    tun.NEXTHOP = NhClient.GetNexthopObject(node, tun.NexthopId)
                    tun.NEXTHOP.AddDependent(tun)
        return

    def FillUnderlayNhGroups(self, node):
        logger.info("Filling nexthop groups")
        for tun in self.Objects(node):
            if tun.IsUnderlayEcmp():
                nhGroupObj = ResmgrClient[node].UnderlayNhGroupAllocator.rrnext()
                tun.NEXTHOPGROUP = nhGroupObj
                tun.NexthopGroupId = nhGroupObj.Id
                logger.info("Linking %s - %s" % (tun, nhGroupObj))
                nhGroupObj.AddDependent(tun)
        return

    def AddObjToDict(self, obj, node):
        self.Objs[node].update({obj.Id: obj})
        return

    def DeleteObjFromDict(self, obj, node):
        self.Objs[node].pop(obj.Id, None)
        return

    def GenerateObjects(self, node, parent, tunnelspec):
        def __isTunFeatureSupported(tunnel_type):
            if tunnel_type == 'service':
                return utils.IsServiceTunnelSupported()
            elif tunnel_type == 'underlay' or tunnel_type == 'underlay-ecmp':
                return utils.IsUnderlayTunnelSupported()
            elif tunnel_type == 'internet-gateway':
                return utils.IsIGWTunnelSupported()
            elif tunnel_type == 'workload':
                return utils.IsWorkloadTunnelSupported()
            return False

        # Generate Remote Tunnel object
        for t in tunnelspec:
            if not __isTunFeatureSupported(t.type):
                continue
            for c in range(t.count):
                print("creating tunnel")
                obj = TunnelObject(node, parent, t, False)
                self.Objs[node].update({obj.Id: obj})
        EzAccessStoreClient[node].SetTunnels(self.Objects(node))
        ResmgrClient[node].CreateInternetTunnels()
        ResmgrClient[node].CreateVnicTunnels()
        ResmgrClient[node].CollectSvcTunnels()
        ResmgrClient[node].CreateUnderlayTunnels()
        return

client = TunnelObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
