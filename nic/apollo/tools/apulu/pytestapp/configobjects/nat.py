#! /usr/bin/python3
import pdb
import utils
import re

import nat_pb2 as nat_pb2
import types_pb2 as types_pb2

class NatPbObject():
    def __init__(self, id, vpc_id=None, prefix=None, port_lo=None, port_hi=None, proto=None, addr_type="public"):
        super().__init__()
        self.id = id
        self.uuid = utils.PdsUuid(self.id)
        self.vpc_id = vpc_id
        self.prefix = prefix
        self.port_lo = port_lo
        self.port_hi = port_hi
        self.proto_name = proto
        self.proto_num = utils.GetIPProtoByName(proto)
        self.addr_type = addr_type
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = nat_pb2.NatPortBlockRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        spec.VpcId = utils.PdsUuid.GetUUIDfromId(self.vpc_id)
        spec.Protocol = self.proto_num
        spec.NatAddress.Prefix.IPv4Subnet.Addr.Af = types_pb2.IP_AF_INET
        spec.NatAddress.Prefix.IPv4Subnet.Addr.V4Addr = int(self.prefix.network_address)
        spec.NatAddress.Prefix.IPv4Subnet.Len = self.prefix.prefixlen
        spec.Ports.PortLow = self.port_lo
        spec.Ports.PortHigh = self.port_hi
        if self.addr_type == "public":
            spec.AddressType = types_pb2.ADDR_TYPE_PUBLIC
        else:
            spec.AddressType = types_pb2.ADDR_TYPE_SERVICE

        return grpcmsg

