#! /usr/bin/python3
import copy
import enum
import pdb
import ipaddress

import infra.config.base as base

import iota.test.apollo.config.resmgr as resmgr
import iota.test.apollo.config.agent.api as agent_api
import iota.harness.api as api

import iota.test.apollo.config.objects.tunnel as tunnel
import iota.test.apollo.config.objects.batch as batch

import device_pb2 as device_pb2
import types_pb2 as types_pb2

from iota.harness.infra.utils.logger import Logger as logger
from iota.test.apollo.config.store import Store

class DeviceObject(base.ConfigObjectBase):
    def __init__(self, name, node_name, index, ip, spec):
        super().__init__()
        self.GID("Device"+str(index))

        self.batch_client = None
        self.name = name
        self.node_name = node_name
        if ip != None:
            self.client = agent_api.ApolloAgentClient(ip)
            self.batch_client = batch.BatchObjectClient(self.client)
            self.mgmtIP = ip
            self.batch_client.GenerateObjects(spec)
        else:
            self.mgmtIP = "N/A"



        ################# PUBLIC ATTRIBUTES OF SWITCH OBJECT #####################
        self.IPAddr = next(resmgr.TepIpAddressAllocator)
        self.GatewayAddr = next(resmgr.TepIpAddressAllocator)
        self.MACAddr = spec.macaddress
        self.IP = str(self.IPAddr) # For testspec
        self.EncapType = types_pb2.ENCAP_TYPE_VXLAN


        ################# PRIVATE ATTRIBUTES OF SWITCH OBJECT #####################
        self.__spec = spec

        self.Show()
        if ip != None:
            self.tunnel_client = tunnel.TunnelObjectClient(self, self.client)
            self.tunnel_client.GenerateLocalTunnelObject(self)
        return

    def __repr__(self):
        return "%s:%s|MgmtIP:%s|IPAddr:%s|GatewayAddr:%s|MAC:%s|Encap:vxlan" %\
               (self.name, self.GID(), self.mgmtIP, self.IPAddr, self.GatewayAddr, self.MACAddr.get())

    def GetGrpcCreateMessage(self):
        grpcmsg = device_pb2.DeviceRequest()
        grpcmsg.Request.IPAddr.Af = types_pb2.IP_AF_INET
        grpcmsg.Request.IPAddr.V4Addr = int(self.IPAddr)
        grpcmsg.Request.GatewayIP.Af = types_pb2.IP_AF_INET
        grpcmsg.Request.GatewayIP.V4Addr = int(self.GatewayAddr)
        grpcmsg.Request.MACAddr = self.MACAddr.getnum()
        return grpcmsg

    def IsEncapTypeMPLS(self):
        if self.EncapType == types_pb2.ENCAP_TYPE_MPLSoUDP:
            return True
        return False

    def IsEncapTypeVXLAN(self):
        if self.EncapType == types_pb2.ENCAP_TYPE_VXLAN:
            return True
        return False

    def Show(self):
        logger.info("Device Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def GetRemoteTunnel(self, device):
        return self.tunnel_client.GetRemoteTunnel(device)

class DeviceObjectClient:
    def __init__(self):
        self.__objs = []
        self.devices = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, topospec):
        naplesHosts = api.GetNaplesHostnames()
        if len(naplesHosts) < len(topospec.devices):
            logger.error("Not sufficient number of naples devices")
            assert(0)

        deviceSpec = None
        for index, device in enumerate(topospec.devices):
            nicIP = api.GetNicMgmtIP(naplesHosts[index])
            obj = DeviceObject(device.naples.name, naplesHosts[index], index, nicIP, device.naples.device)
            self.__objs.append(obj)
            Store.AddDevice(device.naples.name, obj)
            self.devices.append(obj)
            deviceSpec = copy.deepcopy(device)

        for device in self.__objs:
            for other_device in self.__objs:
                if device != other_device:
                    device.tunnel_client.AddRemoteTunnel(device, other_device)

        #Create remote internet tunnels
        index = len(topospec.devices)
        for t in topospec.tunnel:
            for index in range(t.count):
                index += 1
                internetDevice = DeviceObject(index, "internet", "N/A", None, deviceSpec.naples.device)
                for device in self.__objs:
                    device.tunnel_client.AddInternetTunnel(device, internetDevice)


        resmgr.CreateInternetTunnels()

        return

    def CreateObjects(self):
        for obj in self.__objs:
            obj.client.Create(agent_api.ObjectTypes.SWITCH, [obj.GetGrpcCreateMessage()])
            obj.tunnel_client.CreateObjects()
        return

    def Start(self):
        for device in self.devices:
            device.batch_client.Start()

    def Commit(self):
        for device in self.devices:
            device.batch_client.Commit()


client = DeviceObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
