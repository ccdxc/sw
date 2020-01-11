#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from iota.test.apulu.config.store import Store

import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.utils as utils
import iota.test.apulu.config.agent.api as api
import iota.test.apulu.config.objects.base as base
import ipaddress

import tunnel_pb2 as tunnel_pb2

class TunnelObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, local):
        super().__init__(api.ObjectTypes.TUNNEL)
        self.__spec = spec
        if (hasattr(spec, 'id')):
            self.Id = spec.id
        else:
            self.Id = next(resmgr.TunnelIdAllocator)
        self.GID("Tunnel%d"%self.Id)

        self.__nhtype = utils.NhType.NONE
        ################# PUBLIC ATTRIBUTES OF TUNNEL OBJECT #####################
        #self.LocalIPAddr = parent.IPAddr
        self.LocalIPAddr = ipaddress.IPv4Address(spec.srcaddr)
        self.EncapValue = 0
        self.Nat = False
        if (hasattr(spec, 'nat')):
            self.Nat = spec.nat
        if local == True:
            self.RemoteIPAddr = self.LocalIPAddr
            self.Type = tunnel_pb2.TUNNEL_TYPE_NONE
        else:
            self.Type = utils.GetTunnelType(spec.type)
            if self.Type == tunnel_pb2.TUNNEL_TYPE_WORKLOAD:
                self.RemoteIPAddr = next(resmgr.TepIpAddressAllocator)
                self.RemoteVnicMplsSlotIdAllocator = resmgr.CreateRemoteVnicMplsSlotAllocator()
                self.RemoteVnicVxlanIdAllocator = resmgr.CreateRemoteVnicVxlanIdAllocator()
            elif self.Type == tunnel_pb2.TUNNEL_TYPE_IGW:
                self.RemoteIPAddr = next(resmgr.TepIpAddressAllocator)
                if parent.IsEncapTypeMPLS():
                    self.EncapValue = next(resmgr.IGWMplsSlotIdAllocator)
                else:
                    self.EncapValue = next(resmgr.IGWVxlanIdAllocator)
            elif self.Type == tunnel_pb2.TUNNEL_TYPE_SERVICE:
                self.RemoteIPAddr = next(resmgr.TepIpv6AddressAllocator)
                if hasattr(spec, "remote") and spec.remote is True:
                    self.Remote = True
                    self.RemoteServicePublicIP = next(resmgr.RemoteSvcTunIPv4Addr)
                    self.RemoteServiceEncap = next(resmgr.IGWVxlanIdAllocator)
                else:
                    self.Remote = False
                self.EncapValue = next(resmgr.IGWVxlanIdAllocator)
            else:
                if utils.IsV4Stack(parent.Stack):
                    if getattr(spec, 'dstaddr', None) != None:
                        self.RemoteIPAddr = ipaddress.IPv4Address(spec.dstaddr)
                    else:
                        self.RemoteIPAddr = next(resmgr.TepIpAddressAllocator)
                else:
                    self.RemoteIPAddr = next(resmgr.TepIpv6AddressAllocator)
                # nexthop / nh_group association happens later
                if spec.type == 'underlay':
                    self.__nhtype = utils.NhType.UNDERLAY
                    self.NEXTHOP = None
                    self.NextHopId = spec.nhid
                elif spec.type == 'underlay-ecmp':
                    self.__nhtype = utils.NhType.UNDERLAY_ECMP
                    self.NEXTHOPGROUP = None
        #self.RemoteIP = str(self.RemoteIPAddr) # for testspec
        self.MACAddr = getattr(spec, 'macaddress', None)
        #self.MACAddr = resmgr.TepMacAllocator.get()
        ################# PRIVATE ATTRIBUTES OF TUNNEL OBJECT #####################

        self.Show()
        return

    def __repr__(self):
        remote = ""
        if hasattr(self, "Remote") and self.Remote is True:
            remote = " Remote:%s"% (self.Remote)
        return "Tunnel%d|LocalIPAddr:%s|RemoteIPAddr:%s|TunnelType:%s%s|" \
               "EncapValue:%d|Nat:%s|Mac:%s|NhType:%s" % \
               (self.Id,self.LocalIPAddr, self.RemoteIPAddr,
               utils.GetTunnelTypeString(self.Type), remote, self.EncapValue,
               self.Nat, self.MACAddr.get(), self.__nhtype)

    def Show(self):
        logger.info("Tunnel Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(str.encode(str(self.Id)))
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = str.encode(str(self.Id))
        spec.VPCId = str.encode(str(0)) # TODO: Create Underlay VPC
        utils.GetRpcEncap(self.EncapValue, self.EncapValue, spec.Encap)
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
                utils.GetRpcEncap(self.RemoteServiceEncap, self.RemoteServiceEncap, spec.RemoteServiceEncap)
        if self.IsUnderlay():
            if self.NextHopId is not None:
                spec.NexthopId = str.encode(str(self.NextHopId))
            else:
                spec.NexthopId = str.encode(str(self.NEXTHOP.NexthopId))
        elif self.IsUnderlayEcmp():
            spec.NexthopGroupId = str.encode(str(self.NEXTHOPGROUP.Id))
        return

    def ValidateSpec(self, spec):
        if int(spec.Id) != self.Id:
            return False
        if utils.ValidateTunnelEncap(self.EncapValue, spec.Encap) == False:
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
                if utils.ValidateTunnelEncap(self.RemoteServiceEncap, spec.RemoteServiceEncap) == False:
                    return False
        return True

    def ValidateYamlSpec(self, spec):
        if  utils.GetYamlSpecAttr(spec, 'id') != self.Id:
            return False
        return True

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
        if self.__nhtype == utils.NhType.UNDERLAY:
            return True
        return False

    def IsUnderlayEcmp(self):
        if self.__nhtype == utils.NhType.UNDERLAY_ECMP:
            return True
        return False

class TunnelObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.TUNNEL, resmgr.MAX_TUNNEL)
        return

    def GetTunnelObject(self, node, tunnelid):
        return self.GetObjectByKey(node, tunnelid)

    def AssociateObjects(self, node):
        logger.info("Filling nexthops")
        for tun in self.Objects(node):
            if tun.IsUnderlay():
                if tun.NextHopId == None:
                    tun.NEXTHOP = resmgr.UnderlayNHAllocator.rrnext()
                    logger.info("Tunnel%d - Nexthop%d" %
                                (tun.Id, tun.NEXTHOP.NexthopId))
                else:
                    logger.info("Tunnel%d - Nexthop%d" %
                                (tun.Id, tun.NextHopId))
        return

    def FillUnderlayNhGroups(self, node):
        logger.info("Filling nexthop groups")
        for tun in self.Objects(node):
            if tun.IsUnderlayEcmp():
                tun.NEXTHOPGROUP = resmgr.UnderlayNhGroupAllocator.rrnext()
                logger.info("Tunnel%d - NexthopGroup%d" %
                            (tun.Id, tun.NEXTHOPGROUP.Id))
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
                obj = TunnelObject(parent, t, False)
                self.Objs[node].update({obj.Id: obj})
        Store.SetTunnels(self.Objects(node))
        resmgr.CreateInternetTunnels()
        resmgr.CreateVnicTunnels()
        resmgr.CollectSvcTunnels()
        resmgr.CreateUnderlayTunnels()
        return

client = TunnelObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
