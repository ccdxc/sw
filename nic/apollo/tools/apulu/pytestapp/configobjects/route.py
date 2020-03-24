#! /usr/bin/python3
import pdb

import route_pb2 as route_pb2
import types_pb2 as types_pb2
import ipaddress
import utils

class RouteObject():
    def __init__(self, prefix, nhtype, nhid, nataction, meteren):
        assert(nhtype == "tunnel")
        self.prefix = prefix
        self.nhtype = nhtype
        self.nhid = nhid
        self.nataction = nataction
        self.meteren = meteren

class RouteTableObject():
    def __init__(self, id, addrfamily, routes=None):
        self.id = id
        self.uuid = utils.PdsUuid(self.id)
        self.addrfamily = addrfamily
        self.routes = routes
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = route_pb2.RouteTableRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        spec.Af = self.addrfamily
        for route in self.routes:
            rtspec = spec.Routes.add()
            if route.nhtype == 'tunnel':
                rtspec.TunnelId = utils.PdsUuid.GetUUIDfromId(route.nhid)
            else:
                assert(0)
            rtspec.Prefix.Addr.Af = self.addrfamily
            rtspec.Prefix.Addr.V4Addr = int(route.prefix.network_address)
            rtspec.Prefix.Len = route.prefix.prefixlen
            if route.nataction:
                rtspec.NatAction = route.nataction
            rtspec.MeterEn = route.meteren
        return grpcmsg
