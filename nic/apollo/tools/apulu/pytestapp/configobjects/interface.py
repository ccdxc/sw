#! /usr/bin/python3
import pdb

import interface_pb2 as interface_pb2
import types_pb2 as types_pb2
import ipaddress
import utils
import re


class L3IfObject():
    def __init__(self, vpcid, prefix, portid, encap, macaddr, node_uuid):
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


class InterfaceObject():
    def __init__(self, id, iftype, ifadminstatus, vpcid, prefix, portid, encap, macaddr=None, node_uuid=None):
        self.id = id
        self.uuid = utils.PdsUuid(self.id)
        self.iftype = iftype
        self.ifadminstatus = ifadminstatus
        self.ifobj = L3IfObject( vpcid, prefix, portid, encap, macaddr, node_uuid )
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = interface_pb2.InterfaceRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        spec.Type = self.iftype
        spec.AdminStatus = self.ifadminstatus
        spec.L3IfSpec.VpcId = utils.PdsUuid.GetUUIDfromId(self.ifobj.vpcid)
        spec.L3IfSpec.Prefix.Addr.Af = 1
        spec.L3IfSpec.Prefix.Len = int(self.ifobj.prefix._prefixlen)
        spec.L3IfSpec.Prefix.Addr.V4Addr = int(self.ifobj.prefix.ip)
        spec.L3IfSpec.PortId = self.ifobj.portuuid.GetUuid()
        spec.L3IfSpec.Encap.type = self.ifobj.encap
        spec.L3IfSpec.MACAddress = utils.getmac2num(self.ifobj.macaddr,reorder=False)
        return grpcmsg
