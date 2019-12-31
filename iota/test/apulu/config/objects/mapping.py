#! /usr/bin/python3
import pdb
import ipaddress
import random
import socket
from collections import defaultdict


#Following come from dol/infra
import infra.config.base as base
import infra.common.defs as defs
import infra.common.parser as parser


import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.utils as utils
import iota.test.apulu.config.agent.api as agent_api

import mapping_pb2 as mapping_pb2
import types_pb2 as types_pb2

from iota.harness.infra.utils.logger import Logger as logger
from iota.test.apulu.config.store import Store


class LocalMappingObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, ipversion):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF MAPPING OBJECT #####################
        self.MappingId = next(resmgr.LocalMappingIdAllocator)
        self.GID('LocalMapping%d'%self.MappingId)
        self.VNIC = parent
        self.PublicIPAddr = None
        self.SourceGuard = parent.SourceGuard
        if ipversion == utils.IP_VERSION_6:
            self.AddrFamily = 'IPV6'
            self.IPAddr = parent.SUBNET.AllocIPv6Address();
            if (hasattr(spec, 'public')):
                self.PublicIPAddr = next(resmgr.PublicIpv6AddressAllocator)
            self.HasDefaultRoute = parent.SUBNET.V6RouteTable.HasDefaultRoute # For testspec
        else:
            self.AddrFamily = 'IPV4'
            self.IPAddr = parent.SUBNET.AllocIPv4Address();
            if (hasattr(spec, 'public')):
                self.PublicIPAddr = next(resmgr.PublicIpAddressAllocator)
            self.HasDefaultRoute = parent.SUBNET.V4RouteTable.HasDefaultRoute # For testspec
        self.Label = 'NETWORKING'
        self.FlType = "MAPPING"
        self.IP = str(self.IPAddr) # for testspec
        if self.PublicIPAddr is not None:
            self.PublicIP = str(self.PublicIPAddr) # for testspec
        ################# PRIVATE ATTRIBUTES OF MAPPING OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "LocalMappingID:%d|VnicId:%d|SubnetId:%d|IpAdddr:%s|Vlan:%d|VPCId:%d" %\
               (self.MappingId, self.VNIC.VnicId, self.VNIC.SUBNET.SubnetId, self.IPAddr, self.VNIC.VlanId, self.VNIC.SUBNET.VPC.VPCId)

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.flow.filters)

    def GetGrpcCreateMessage(self):
        grpcmsg = mapping_pb2.MappingRequest()
        spec = grpcmsg.Request.add()
        spec.Id.VPCId = self.VNIC.SUBNET.VPC.VPCId
        utils.GetRpcIPAddr(self.IPAddr, spec.Id.IPAddr)
        spec.SubnetId = self.VNIC.SUBNET.SubnetId
        spec.VnicId = self.VNIC.VnicId
        spec.MACAddr = self.VNIC.MACAddr.getnum()
        utils.GetRpcEncap(self.VNIC.device.name, self.VNIC.MplsSlot, self.VNIC.Vnid, spec.Encap)
        spec.PublicIP.Af = types_pb2.IP_AF_NONE
        if self.PublicIPAddr is not None:
            utils.GetRpcIPAddr(self.PublicIPAddr, spec.PublicIP)
        return grpcmsg

    def Show(self):
        logger.info("LocalMapping Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- IPAddr:%s" % str(self.IPAddr))
        if self.PublicIPAddr is not None:
            logger.info("- Public IPAddr:%s" % str(self.PublicIPAddr))
        return

    def SetupTestcaseConfig(self, obj):
        return

class RemoteMappingObject(base.ConfigObjectBase):
    def __init__(self, parent, ipaddr, tunobj, ipversion):
        super().__init__()

        ################# PUBLIC ATTRIBUTES OF MAPPING OBJECT #####################
        self.MappingId = next(resmgr.RemoteMappingIdAllocator)
        self.GID('RemoteMapping%d'%self.MappingId)
        self.SUBNET = parent.SUBNET
        self.parent = parent
        self.VNIC = parent

        self.MACAddr = resmgr.RemoteMappingMacAllocator.get()
        self.TunIPAddr = tunobj.RemoteIPAddr
        self.MplsSlot =  next(tunobj.RemoteVnicMplsSlotIdAllocator)
        self.Vnid = self.VNIC.Vnid
        self.TunObj = tunobj
        self.IPAddr = ipaddr
        if ipversion == utils.IP_VERSION_6:
            self.AddrFamily = 'IPV6'
            #self.HasDefaultRoute = self.SUBNET.V6RouteTable.HasDefaultRoute # For testspec
        else:
            self.AddrFamily = 'IPV4'
            self.HasDefaultRoute = self.SUBNET.V4RouteTable.HasDefaultRoute # For testspec
        self.Label = 'NETWORKING'
        self.FlType = "MAPPING"
        self.IP = str(self.IPAddr) # For testspec
        self.TunIP = str(self.TunIPAddr) # For testspec

        ################# PRIVATE ATTRIBUTES OF MAPPING OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "RemoteMappingID:%d|SubnetId:%d|IPAddr:%s|VPCId:%d" %\
               (self.MappingId, self.SUBNET.SubnetId, self.IPAddr, self.SUBNET.VPC.VPCId)

    def __repr__(self):
        return "RemoteMappingID:%d|SubnetId:%d|VPCId:%d" %\
               (self.MappingId, self.SUBNET.SubnetId, self.SUBNET.VPC.VPCId)

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.flow.filters)

    def GetGrpcCreateMessage(self):
        grpcmsg = mapping_pb2.MappingRequest()
        spec = grpcmsg.Request.add()
        spec.Id.VPCId = self.SUBNET.VPC.VPCId
        utils.GetRpcIPAddr(self.IPAddr, spec.Id.IPAddr)
        spec.SubnetId = self.SUBNET.SubnetId
        utils.GetRpcIPAddr(self.TunIPAddr, spec.TunnelIP)
        spec.MACAddr = self.MACAddr.getnum()
        utils.GetRpcEncap(self.parent.device.name, self.MplsSlot, self.Vnid, spec.Encap)
        return grpcmsg

    def Show(self):
        logger.info("RemoteMapping object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- IPAddr:%s|TunIPAddr:%s|MAC:%s|Mpls:%d|Vxlan:%d" %\
                (str(self.IPAddr), str(self.TunIPAddr), self.MACAddr,
                self.MplsSlot, self.Vnid))
        return



class LocalMappingObjectClient:
    def __init__(self):
        self.__localMappings = defaultdict(list)
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, parent, vnic_spec_obj):
        stack = parent.SUBNET.VPC.Stack
        c = 0
        while c < vnic_spec_obj.ipcount:
            if stack == "dual" or stack == 'ipv6':
                obj = LocalMappingObject(parent, vnic_spec_obj, utils.IP_VERSION_6)
                self.__localMappings[parent.device].append(obj)
                #ADd remote tunnel for rest of them.
                for device in Store.GetDevices():
                    if device != parent.device:
                        remoteTunnel = device.GetRemoteTunnel(parent.device)
                        assert(remoteTunnel)
                        obj = RemoteMappingObject(parent, obj.IPAddr, remoteTunnel, utils.IP_VERSION_6)
                        self.__localMappings[device].append(obj)
                c = c + 1
            if c < vnic_spec_obj.ipcount and (stack == "dual" or stack == 'ipv4'):
                obj = LocalMappingObject(parent, vnic_spec_obj, utils.IP_VERSION_4)
                self.__localMappings[parent.device].append(obj)
                #ADd remote tunnel for rest of them.
                for device in Store.GetDevices():
                    if device != parent.device:
                        remoteTunnel = device.GetRemoteTunnel(parent.device)
                        assert(remoteTunnel)
                        obj = RemoteMappingObject(parent, obj.IPAddr, remoteTunnel, utils.IP_VERSION_4)
                        self.__localMappings[device].append(obj)
                c = c + 1
        return

    def CreateObjects(self):

        for device, mappings in self.__localMappings.items():
            logger.info("Creating Mapping objects for device :", device)
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(), mappings))
            device.client.Create(agent_api.ObjectTypes.MAPPING, msgs)

    def GetVnicAddresses(self, vnic):
        ip_addresses = []
        for device, mappings in self.__localMappings.items():
            if vnic.device.GID() == device.GID():
                for mapping in mappings:
                    if hasattr(mapping, "VNIC") and mapping.VNIC.GID() == vnic.GID():
                        if mapping.AddrFamily == 'IPV6':
                            ip_addresses.append(str(mapping.IPAddr) + "/" + str(mapping.VNIC.SUBNET.IPPrefix[0].prefixlen))
                        else:
                            ip_addresses.append(str(mapping.IPAddr) + "/" + str(mapping.VNIC.SUBNET.IPPrefix[1].prefixlen))

        return ip_addresses


client = LocalMappingObjectClient()

def GetMatchingObjects(selectors):
    objs = []
    for obj in client.Objects():
        if obj.IsFilterMatch(selectors):
            objs.append(obj)
    return objs
