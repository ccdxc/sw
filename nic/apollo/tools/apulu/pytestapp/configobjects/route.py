#! /usr/bin/python3
import pdb

import route_pb2 as route_pb2
import types_pb2 as types_pb2
import ipaddress
import utils

class RouteObject():
    def __init__(self, prefix, nhtype, nhid, nat_type=None, nat_addr_type="public", meteren=False):
        assert(nhtype == "tunnel")
        self.prefix = prefix
        self.nhtype = nhtype
        self.nhid = nhid
        self.nat_type = nat_type
        self.nat_addr_type = nat_addr_type
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
                rtspec.Attrs.TunnelId = utils.PdsUuid.GetUUIDfromId(route.nhid)
            else:
                assert(0)
            rtspec.Attrs.Prefix.Addr.Af = self.addrfamily
            rtspec.Attrs.Prefix.Addr.V4Addr = int(route.prefix.network_address)
            rtspec.Attrs.Prefix.Len = route.prefix.prefixlen
            if route.nat_type:
                if route.nat_type == "napt":
                    if route.nat_addr_type == "public":
                        rtspec.Attrs.NatAction.SrcNatAction = types_pb2.NAT_ACTION_NAPT_PUBLIC
                    else:
                        rtspec.Attrs.NatAction.SrcNatAction = types_pb2.NAT_ACTION_NAPT_SVC
                elif route.nat_type == "static":
                    rtspec.Attrs.NatAction.SrcNatAction = types_pb2.NAT_ACTION_STATIC
            rtspec.Attrs.MeterEn = route.meteren
        return grpcmsg
