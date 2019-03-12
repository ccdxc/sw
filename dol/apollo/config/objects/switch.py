#! /usr/bin/python3
import enum
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.tunnel as tunnel

import switch_pb2 as switch_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class SwitchObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__()
        self.GID("Switch1")
        
        ################# PUBLIC ATTRIBUTES OF SWITCH OBJECT #####################
        self.LocalIP = next(resmgr.TepIpAddressAllocator)
        self.Gateway = ipaddress.IPv4Address(spec.gateway)
        self.MACAddress = spec.macaddress

        ################# PRIVATE ATTRIBUTES OF SWITCH OBJECT #####################
        self.__spec = spec

        self.Show()
        tunnel.client.GenerateObjects(self, spec.tunnel)
        return

    def __repr__(self):
        return "Switch1/LocalIP:%s/Gateway:%s/MAC:%s" %\
               (self.LocalIP, self.Gateway, self.MACAddress.get())

    def GetGrpcCreateMessage(self):
        swspec = switch_pb2.SwitchSpec()
        swspec.IPAddr.Af = types_pb2.IP_AF_INET
        swspec.IPAddr.V4Addr = int(self.LocalIP)
        swspec.GatewayIP.Af = types_pb2.IP_AF_INET
        swspec.GatewayIP.V4Addr = int(self.Gateway)
        swspec.MACAddr = self.MACAddress.getnum()
        return swspec
   
    def Show(self):
        logger.info("Switch Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

class SwitchObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, topospec):
        obj = SwitchObject(topospec.switch)
        self.__objs.append(obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.SWITCH, msgs)
        tunnel.client.CreateObjects()
        return

client = SwitchObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
