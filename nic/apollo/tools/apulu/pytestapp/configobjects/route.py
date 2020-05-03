#! /usr/bin/python3
import pdb

import route_pb2 as route_pb2
import types_pb2 as types_pb2
import ipaddress
import utils

class RouteObject():
    def __init__(self, id, route_table_id, addrfamily, prefix, nhtype, nhid, nat_type=None, nat_addr_type="public", meteren=False, dnat_ip=None):
        assert(nhtype == "tunnel")
        self.id = id
        self.uuid = utils.PdsUuid(self.id)
        self.prefix = prefix
        self.nhtype = nhtype
        self.nhid = nhid
        self.nat_type = nat_type
        self.nat_addr_type = nat_addr_type
        if dnat_ip:
            self.nat_addr_type = "service"
        self.dnat_ip = dnat_ip
        self.meteren = meteren
        self.route_table_id = route_table_id
        self.addrfamily = addrfamily

    def GetGrpcCreateMessage(self):
        grpcmsg = route_pb2.RouteRequest()
        spec = grpcmsg.Request
        spec.Id = self.uuid.GetUuid()
        spec.RouteTableId = utils.PdsUuid.GetUUIDfromId(self.route_table_id)

        if self.nhtype == 'tunnel':
            spec.Attrs.TunnelId = utils.PdsUuid.GetUUIDfromId(self.nhid)
        else:
            assert(0)
        spec.Attrs.Prefix.Addr.Af = self.addrfamily
        spec.Attrs.Prefix.Addr.V4Addr = int(self.prefix.network_address)
        spec.Attrs.Prefix.Len = self.prefix.prefixlen
        if self.nat_type:
            if self.nat_type == "napt":
                if self.nat_addr_type == "public":
                    spec.Attrs.NatAction.SrcNatAction = types_pb2.NAT_ACTION_NAPT_PUBLIC
                else:
                    spec.Attrs.NatAction.SrcNatAction = types_pb2.NAT_ACTION_NAPT_SVC
        if self.dnat_ip:
            spec.Attrs.NatAction.DstNatIP.Af = types_pb2.IP_AF_INET
            spec.Attrs.NatAction.DstNatIP.V4Addr = int(self.dnat_ip)
        spec.Attrs.MeterEn = self.meteren
        return grpcmsg

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
            if route.dnat_ip:
                rtspec.Attrs.NatAction.DstNatIp.Af = types_pb2.IP_AF_INET
                rtspec.Attrs.NatAction.DstNatIp.V4Addr = int(route.dnat_ip)
            rtspec.Attrs.MeterEn = route.meteren
        return grpcmsg
