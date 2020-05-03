#! /usr/bin/python3
import pdb

import mapping_pb2 as mapping_pb2
import types_pb2 as types_pb2
import ipaddress
import utils
import re


class MappingObject():
    def __init__(self, id, key_type, macaddr, ip, vpcid, subnetid=None, tunnelid=None, encaptype=None, encapslotid=None, nexthopgroupid=None, vnicid=None, public_ip=None, tags=[]):
        self.id = id
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
        self.public_ip = None
        if public_ip:
            self.public_ip = int(public_ip)
        self.tags = tags
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = mapping_pb2.MappingRequest()
        spec = grpcmsg.Request.add()
        spec.Id = utils.PdsUuid.GetUUIDfromId(self.id)
        if re.search( 'l3', self.keytype, re.I ):
            spec.IPKey.VPCId = utils.PdsUuid.GetUUIDfromId(self.vpcid)
            spec.IPKey.IPAddr.Af = types_pb2.IP_AF_INET
            spec.IPKey.IPAddr.V4Addr = self.ip

        if self.subnetid is not None:
           spec.SubnetId = utils.PdsUuid.GetUUIDfromId(self.subnetid)
        if self.tunnelid is not None:
           spec.TunnelId = utils.PdsUuid.GetUUIDfromId(self.tunnelid)
        if self.nexthopgroupid is not None:
           spec.NexthopGroupId = utils.PdsUuid.GetUUIDfromId(self.nexthopgroupid)

        spec.MACAddr  = self.macaddr
        if self.public_ip:
            spec.PublicIP.Af = types_pb2.IP_AF_INET
            spec.PublicIP.V4Addr = self.public_ip
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
        spec.Tags.extend(self.tags)
        return grpcmsg
