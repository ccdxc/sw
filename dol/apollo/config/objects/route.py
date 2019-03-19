#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.utils as utils
import route_pb2 as route_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class RouteObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, tunip, ipversion, nroutes):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF SUBNET OBJECT #####################
        self.RouteTblId = next(resmgr.RouteTableIdAllocator)
        self.GID('RouteTbl%d'%self.RouteTblId)
        if ipversion == 6:
            self.Prefix = next(resmgr.RouteIPv6Allocator)
            self.af = 'IPV6'
        else:
            self.Prefix = next(resmgr.RouteIPv4Allocator)
            self.af = 'IPV4'
        self.TunIP = tunip
        self.PCNId = parent.PCNId
        self.label = 'NETWORKING'
        assert nroutes == 1 # TODO

        ##########################################################################

        self.Show()
        return

    def __repr__(self):
        return "RouteTblID:%d/PCNId:%d/Prefix:%s/NextHop:%s" %\
               (self.RouteTblId, self.PCNId, str(self.Prefix), str(self.TunIP))

    def GetGrpcCreateMessage(self):
        grpcmsg = route_pb2.RouteTableRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.RouteTblId
        if self.Prefix.version == 6:
            spec.AF = types_pb2.IP_AF_INET6
        else:
            spec.AF = types_pb2.IP_AF_INET

        rtspec = spec.Routes.add()
        if self.Prefix.version == 6:
            rtspec.Prefix.Addr.Af = types_pb2.IP_AF_INET6
            rtspec.Prefix.Addr.V6Addr = self.Prefix.network_address.packed
        else:
            rtspec.Prefix.Addr.Af = types_pb2.IP_AF_INET
            rtspec.Prefix.Addr.V4Addr = int(self.Prefix.network_address)
        rtspec.Prefix.Len = self.Prefix.prefixlen
        rtspec.NextHop.Af = types_pb2.IP_AF_INET
        rtspec.NextHop.V4Addr = int(self.TunIP)
        rtspec.PCNId = self.PCNId
        return grpcmsg

    def Show(self):
        logger.info("RouteTbl object:", self)
        logger.info("- %s" % repr(self))
        return

    def SetupTestcaseConfig(self, obj):
        return

class RouteObjectClient:
    def __init__(self):
        self.__objs = []
        self.__internet_tunnel = None;
        self.__v4objs = {}
        self.__v6objs = {}
        self.__v4iter = {}
        self.__v6iter = {}
        return

    def __internet_tunnel_get(self):
        return self.__internet_tunnel.rrnext().RemoteIP

    def Objects(self):
        return self.__objs

    def GetRouteV4TableId(self, pcnid):
        if len(self.__v4objs[pcnid]) == 0:
            return 0
        return self.__v4iter[pcnid].rrnext().RouteTblId

    def GetRouteV6TableId(self, pcnid):
        if len(self.__v6objs[pcnid]) == 0:
            return 0
        return self.__v6iter[pcnid].rrnext().RouteTblId

    def GenerateObjects(self, parent, pcn_spec_obj):
        pcnid = parent.PCNId
        self.__internet_tunnel = utils.rrobiniter(Store.GetTunnelsMplsOverUdp1())
        self.__v4objs[pcnid] = []
        self.__v6objs[pcnid] = []
        self.__v4iter[pcnid] = None
        self.__v6iter[pcnid] = None

        for route_spec_obj in pcn_spec_obj.route:
            tunip = self.__internet_tunnel_get()
            if getattr(route_spec_obj, 'v4count', None) != None:
                for c in range(route_spec_obj.v4count):
                    obj = RouteObject(parent, route_spec_obj, tunip, 4, 1)
                    self.__objs.append(obj)
                    self.__v4objs[pcnid].append(obj)
                self.__v4iter[pcnid] = utils.rrobiniter(self.__v4objs[pcnid])
            if getattr(route_spec_obj, 'v6count', None) != None:
                for c in range(route_spec_obj.v6count):
                    obj = RouteObject(parent, route_spec_obj, tunip, 6, 1)
                    self.__objs.append(obj)
                    self.__v6objs[pcnid].append(obj)
                self.__v6iter[pcnid] = utils.rrobiniter(self.__v6objs[pcnid])

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.ROUTE, msgs)
        return

client = RouteObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
