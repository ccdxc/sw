#! /usr/bin/python3
import pdb

import route_pb2 as route_pb2
import types_pb2 as types_pb2
import ipaddress
import utils

class RouteObject():
    def __init__(self, id, addrfamily, routes):
        self.id        = id
        self.uuid      = utils.PdsUuid(self.id)
        self.addrfamily = addrfamily
        if addrfamily ==  0:
            self.addrfamly = types_pb2.IP_AF_INET
        elif addrfamily == 1:
            self.addrfamly = types_pb2.IP_AF_INET6
        self.routes = routes
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = route_pb2.RouteTableRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.id
        spec.Id = self.uuid.GetUuid()
        spec.Af = self.addrfamily
        for route in self.routes:
            rtspec = spec.Routes.add()
            if route[2] == 'nh':
                rtspec.NextHop.Af = self.addrfamily
                rtspec.NextHop.V4Addr = int(ipaddress.IPv4Address(route[3]))
                rtspec.Prefix.Len = route[1]
                rtspec.Prefix.Addr.Af = self.addrfamily
                if self.addrfamily == types_pb2.IP_AF_INET:
                    rtspec.Prefix.Addr.V4Addr = int(ipaddress.IPv4Network("%s/%s"%(route[0],route[1])).network_address)
                elif self.addrfamily == types_pb2.IP_AF_INET6:
                    rtspec.Prefix.Addr.V6Addr = ipaddress.IPv6Network("%s/%s"%(route[0],route[1])).network_address.packed
            elif route[2] == "vpc":
                rtspec.VPCId = utils.PdsUuid.GetUUIDfromId(route[3])
                rtspec.Prefix.Len = route[1]
                rtspec.Prefix.Addr.Af = self.addrfamily
                if self.addrfamily == types_pb2.IP_AF_INET:
                    rtspec.Prefix.Addr.V4Addr = int(ipaddress.IPv4Network("%s/%s"%(route[0],route[1])).network_address)
                elif self.addrfamily == types_pb2.IP_AF_INET6:
                    rtspec.Prefix.Addr.V6Addr = ipaddress.IPv6Network("%s/%s"%(route[0],route[1])).network_address.packed
        return grpcmsg
