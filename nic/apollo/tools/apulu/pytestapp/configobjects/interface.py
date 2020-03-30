#! /usr/bin/python3
import pdb

import interface_pb2 as interface_pb2
import types_pb2 as types_pb2
import ipaddress
import utils
import re
import socket


class L3IfObject():
    def __init__(self, vpcid, prefix, portid, encap, macaddr, node_uuid=None):
        self.vpcid       = vpcid
        self.prefix      = ipaddress.IPv4Interface(prefix)
        self.portid      = portid
        self.ethifidx    = utils.PortToEthIfIdx(self.portid)
        if node_uuid:
            self.portuuid = utils.PdsUuid(self.ethifidx, node_uuid)
        else:
            self.portuuid = utils.PdsUuid(self.ethifidx)
        self.encap       = encap
        self.macaddr     = macaddr
        return

class MgmtIfObject():
    def __init__(self, prefix, macaddr, gateway=None):
        self.prefix      = ipaddress.IPv4Interface(prefix)
        self.macaddr     = macaddr
        self.gateway     = gateway
        return

class LoopbackIfObject():
    def __init__(self, prefix):
        self.prefix     = ipaddress.IPv4Interface(prefix)

class InterfaceObject():
    def __init__(self, id, iftype, ifadminstatus, vpcid=None, prefix=None, portid=None, encap=None, macaddr=None, node_uuid=None, gateway=None):
        self.id = id
        self.uuid = utils.PdsUuid(self.id)
        self.iftype = iftype
        self.ifadminstatus = ifadminstatus
        if iftype == interface_pb2.IF_TYPE_L3:
            self.ifobj = L3IfObject( vpcid, prefix, portid, encap, macaddr, node_uuid )
        elif iftype == interface_pb2.IF_TYPE_CONTROL:
            self.ifobj = MgmtIfObject( prefix, macaddr, gateway=gateway )
        elif iftype == interface_pb2.IF_TYPE_LOOPBACK:
            self.ifobj = LoopbackIfObject(prefix)
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = interface_pb2.InterfaceRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        spec.Type = self.iftype
        spec.AdminStatus = self.ifadminstatus
        if self.iftype == interface_pb2.IF_TYPE_L3:
            spec.L3IfSpec.VpcId = utils.PdsUuid.GetUUIDfromId(self.ifobj.vpcid)
            spec.L3IfSpec.Prefix.Addr.Af = 1
            spec.L3IfSpec.Prefix.Len = int(self.ifobj.prefix._prefixlen)
            spec.L3IfSpec.Prefix.Addr.V4Addr = socket.htonl(int(self.ifobj.prefix.ip))
            spec.L3IfSpec.PortId = self.ifobj.portuuid.GetUuid()
            spec.L3IfSpec.Encap.type = self.ifobj.encap
            spec.L3IfSpec.MACAddress = utils.getmac2num(self.ifobj.macaddr,reorder=False)
        elif self.iftype == interface_pb2.IF_TYPE_CONTROL:
            spec.ControlIfSpec.Prefix.Addr.Af = 1
            spec.ControlIfSpec.Prefix.Len = int(self.ifobj.prefix._prefixlen)
            spec.ControlIfSpec.Prefix.Addr.V4Addr = int(self.ifobj.prefix.ip)
            spec.ControlIfSpec.MACAddress = utils.getmac2num(self.ifobj.macaddr,reorder=False)
            if self.ifobj.gateway:
                spec.ControlIfSpec.Gateway.Af = types_pb2.IP_AF_INET
                spec.ControlIfSpec.Gateway.V4Addr = int(self.ifobj.gateway)
        elif self.iftype == interface_pb2.IF_TYPE_LOOPBACK:
            spec.LoopbackIfSpec.Prefix.Len = int(self.ifobj.prefix._prefixlen)
            spec.LoopbackIfSpec.Prefix.Addr.Af = types_pb2.IP_AF_INET
            spec.LoopbackIfSpec.Prefix.Addr.V4Addr = socket.htonl(int(self.ifobj.prefix.ip))
        return grpcmsg
