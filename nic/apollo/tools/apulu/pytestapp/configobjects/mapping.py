#! /usr/bin/python3
import pdb

import mapping_pb2 as mapping_pb2
import types_pb2 as types_pb2
import ipaddress
import utils
import re


class MappingObject():
    def __init__(self, key_type, macaddr, ip, vpcid, subnetid=None, tunnelid=None, encaptype=None, encapslotid=None, nexthopgroupid=None, vnicid=None):
        self.keytype = key_type
        self.macaddr  = utils.getmac2num(macaddr)
        if type(ip) is ipaddress.IPv4Address:
            self.af   = types_pb2.IP_AF_INET
            self.ip   = int(ip)
        elif type(ip) is ipaddress.IPv6Address:
            self.af   = types_pb2.IP_AF_INET6
            self.ip   = ip.packed
        self.nexthopgroupid = nexthopgroupid
        self.vpcid    = vpcid
        self.subnetid = subnetid
        self.tunnelid = tunnelid
        self.encaptype = encaptype
        self.encapslotid = encapslotid
        self.vnicid = vnicid
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = mapping_pb2.MappingRequest()
        spec = grpcmsg.Request.add()
        if re.search( 'l3', self.keytype, re.I ):
            spec.Id.IPKey.VPCId = utils.PdsUuid.GetUUIDfromId(self.vpcid)
            spec.Id.IPKey.IPAddr.Af = types_pb2.IP_AF_INET
            spec.Id.IPKey.IPAddr.V4Addr = self.ip

        if self.subnetid is not None:
           spec.SubnetId = utils.PdsUuid.GetUUIDfromId(self.subnetid)
        if self.tunnelid is not None:
           spec.TunnelId = utils.PdsUuid.GetUUIDfromId(self.tunnelid)
        if self.nexthopgroupid is not None:
           spec.NexthopGroupId = utils.PdsUuid.GetUUIDfromId(self.nexthopgroupid)

        spec.MACAddr  = self.macaddr
        #spec.PublicIP = self.publicip
        #spec.ProviderIp = self.providerip
        if self.encaptype == types_pb2.ENCAP_TYPE_VXLAN:
           spec.Encap.type = self.encaptype
           spec.Encap.value.Vnid = self.encapslotid
        #if self.af == types_pb2.IP_AF_INET:
        #    spec.Id.IPAddr.V4Addr = self.ip
        #elif self.af == types_pb2.IP_AF_INET6:
        #    spec.Id.IPAddr.V6Addr = self.ip
        if self.vnicid is not None:
           spec.VnicId = utils.PdsUuid.GetUUIDfromId(self.vnicid)
        return grpcmsg
