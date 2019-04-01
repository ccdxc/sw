#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.lmapping as lmapping
import route_pb2 as route_pb2
import types_pb2 as types_pb2
import ipaddress

from infra.common.logging import logger
from apollo.config.store import Store

class RouteObject(base.ConfigObjectBase):
    def __init__(self, parent, af, routes, routetype, tunobj):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF ROUTE TABLE OBJECT #####################
        if af == utils.IP_VERSION_6:
            self.RouteTblId = next(resmgr.V6RouteTableIdAllocator)
            self.AddrFamily = 'IPV6'
        else:
            self.RouteTblId = next(resmgr.V4RouteTableIdAllocator)
            self.AddrFamily = 'IPV4'
        self.GID('RouteTbl%d'%self.RouteTblId)
        self.routes = routes
        self.Tunnel = tunobj
        self.TunIPAddr = tunobj.RemoteIPAddr
        self.TunIP = str(self.TunIPAddr)
        self.VPCId = parent.VPCId
        self.Label = 'NETWORKING'
        self.RouteType = routetype # used for lpm route cases
        if 'default' in routetype:
            self.HasDefaultRoute = True
        else:
            self.HasDefaultRoute = False
        ##########################################################################
        self.Show()
        return

    def __repr__(self):
        return "RouteTblID:%d|VPCId:%d|NumRoutes:%d|HasDefaultRoute:%d|AddrFamily:%s|RouteType:%s|NextHop:%s" %\
               (self.RouteTblId, self.VPCId, len(self.routes), self.HasDefaultRoute,
                self.AddrFamily, self.RouteType, str(self.TunIPAddr))

    def GetGrpcCreateMessage(self):
        grpcmsg = route_pb2.RouteTableRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.RouteTblId
        spec.AF = utils.GetRpcIPAddrFamily(self.AddrFamily)
        for route in self.routes:
            rtspec = spec.Routes.add()
            utils.GetRpcIPPrefix(route, rtspec.Prefix)
            rtspec.NextHop.Af = types_pb2.IP_AF_INET
            rtspec.NextHop.V4Addr = int(self.TunIPAddr)
            rtspec.VPCId = self.VPCId
        return grpcmsg

    def Show(self):
        logger.info("RouteTbl object:", self)
        logger.info("- %s" % repr(self))
        for route in self.routes:
            logger.info("-- %s" % str(route))
        return

    def SetupTestcaseConfig(self, obj):
        obj.local_mapping = self.l_obj
        obj.route = self
        obj.hostport = 1
        obj.switchport = 2
        obj.devicecfg = Store.GetDevice()
        return

class RouteObjectClient:
    def __init__(self):
        self.__objs = []
        self.__v4objs = {}
        self.__v6objs = {}
        self.__v4iter = {}
        self.__v6iter = {}
        return

    def __internet_tunnel_get(self):
        return resmgr.RemoteMplsInternetTunAllocator.rrnext()

    def Objects(self):
        return self.__objs

    def GetRouteV4Table(self, vpcid, routetblid):
        for routetbl in self.__v4objs[vpcid]:
            if routetbl.RouteTblId == routetblid:
                return routetbl
        return None

    def GetRouteV6Table(self, vpcid, routetblid):
        for routetbl in self.__v6objs[vpcid]:
            if routetbl.RouteTblId == routetblid:
                return routetbl
        return None

    def GetRouteV4TableId(self, vpcid):
        if len(self.__v4objs[vpcid]) == 0:
            return 0
        return self.__v4iter[vpcid].rrnext().RouteTblId

    def GetRouteV6TableId(self, vpcid):
        if len(self.__v6objs[vpcid]) == 0:
            return 0
        return self.__v6iter[vpcid].rrnext().RouteTblId

    def GenerateObjects(self, parent, vpc_spec_obj):
        vpcid = parent.VPCId
        stack = parent.Stack
        self.__v4objs[vpcid] = []
        self.__v6objs[vpcid] = []
        self.__v4iter[vpcid] = None
        self.__v6iter[vpcid] = None

        if resmgr.RemoteMplsInternetTunAllocator == None:
            logger.info("Skipping route creation as there are no Internet tunnels")
            return

        tunobj = self.__internet_tunnel_get()

        def __get_next_subnet(ip):
            if ip.version == 4:
                return ipaddress.ip_network(str(ip.network_address + 2 ** (32 - ip.prefixlen )) +'/'+ str(ip.prefixlen))
            else:
                return ipaddress.ip_network(str(ip.network_address + 2 ** (128 - ip.prefixlen )) +'/'+ str(ip.prefixlen))

        def __get_adjacent_routes(base, count):
            routes = []
            c = 1
            routes.append(ipaddress.ip_network(base))
            while c < count:
                routes.append(__get_next_subnet(routes[c-1]))
                c += 1
            return routes

        def __get_overlap(basepfx, base, count):
            # for overlap, add user specified base prefix with original prefixlen
            routes = __get_user_specified_routes([basepfx])
            routes.extend(__get_adjacent_routes(base, count))
            return routes

        def __get_first_subnet(ip, prefixlen):
            for ip in ip.subnets(new_prefix=prefixlen):
                return (ip)
            return

        def __is_v4stack():
            if stack == "dual" or stack == 'ipv4':
                return True
            return False

        def __is_v6stack():
            if stack == "dual" or stack == 'ipv6':
                return True
            return False

        def __add_v4routetable(v4routes, routetype):
            obj = RouteObject(parent, utils.IP_VERSION_4, v4routes, routetype, tunobj)
            self.__v4objs[vpcid].append(obj)
            self.__objs.append(obj)

        def __add_v6routetable(v6routes, routetype):
            obj = RouteObject(parent, utils.IP_VERSION_6, v6routes, routetype, tunobj)
            self.__v6objs[vpcid].append(obj)
            self.__objs.append(obj)

        def __get_user_specified_routes(routespec):
            routes = []
            if routespec:
                for route in routespec:
                    routes.append(ipaddress.ip_network(route.replace('\\', '/')))
            return routes

        def __add_user_specified_routetable(routetablespec, routetype):
            if __is_v4stack():
                __add_v4routetable(__get_user_specified_routes(routetablespec.v4routes), routetype)

            if __is_v6stack():
                __add_v6routetable(__get_user_specified_routes(routetablespec.v6routes), routetype)

        for routetbl_spec_obj in vpc_spec_obj.routetbl:
            routetbltype = routetbl_spec_obj.type
            routetype = routetbl_spec_obj.routetype
            if routetbltype == "specific":
                __add_user_specified_routetable(routetbl_spec_obj, routetype)
                continue
            routetablecount = routetbl_spec_obj.count
            v4routecount = routetbl_spec_obj.nv4routes
            v6routecount = routetbl_spec_obj.nv6routes
            v4prefixlen = routetbl_spec_obj.v4prefixlen
            v6prefixlen = routetbl_spec_obj.v6prefixlen
            v4base = __get_first_subnet(ipaddress.ip_network(routetbl_spec_obj.v4base.replace('\\', '/')), v4prefixlen)
            v6base = __get_first_subnet(ipaddress.ip_network(routetbl_spec_obj.v6base.replace('\\', '/')), v6prefixlen)
            # get user specified routes if any for 'base' routetbltype
            user_specified_v4routes = __get_user_specified_routes(routetbl_spec_obj.v4routes)
            user_specified_v6routes = __get_user_specified_routes(routetbl_spec_obj.v6routes)
            for i in range(routetablecount):
                if 'adjacent' in routetype:
                    if __is_v4stack():
                        routes = user_specified_v4routes + __get_adjacent_routes(v4base, v4routecount)
                        __add_v4routetable(routes, routetype)
                        v4base = __get_next_subnet(routes[-1])
                    if __is_v6stack():
                        routes = user_specified_v6routes + __get_adjacent_routes(v6base, v6routecount)
                        __add_v6routetable(routes, routetype)
                        v6base = __get_next_subnet(routes[-1])

                elif 'overlap' in routetype:
                    if __is_v4stack():
                        routes = user_specified_v4routes + __get_overlap(routetbl_spec_obj.v4base, v4base, v4routecount)
                        __add_v4routetable(routes, routetype)
                        v4base = __get_next_subnet(routes[-1])
                    if __is_v6stack():
                        routes = user_specified_v6routes + __get_overlap(routetbl_spec_obj.v6base, v6base, v6routecount)
                        __add_v6routetable(routes, routetype)
                        v6base = __get_next_subnet(routes[-1])

        if len(self.__v6objs[vpcid]) != 0:
            self.__v6iter[vpcid] = utils.rrobiniter(self.__v6objs[vpcid])

        if len(self.__v4objs[vpcid]) != 0:
            self.__v4iter[vpcid] = utils.rrobiniter(self.__v4objs[vpcid])

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.ROUTE, msgs)
        return

client = RouteObjectClient()

class RouteTableObjectHelper:
    def __init__(self):
        return

    def __is_lmapping_match(self, route_obj, lobj):
        if lobj.AddrFamily == 'IPV4':
            return lobj.AddrFamily == route_obj.AddrFamily and\
               lobj.VNIC.SUBNET.V4RouteTableId == route_obj.RouteTblId
        if lobj.AddrFamily == 'IPV6':
            return lobj.AddrFamily == route_obj.AddrFamily and\
               lobj.VNIC.SUBNET.V6RouteTableId == route_obj.RouteTblId

    def GetMatchingConfigObjects(self, selectors):
        objs = []
        rtype = selectors.route.GetValueByKey('RouteType')
        for route_obj in client.Objects():
            if not route_obj.IsFilterMatch(selectors.route.filters):
                continue
            if not route_obj.routes:
                # skip route tables with no routes
                continue
            for lobj in lmapping.GetMatchingObjects(selectors):
                if self.__is_lmapping_match(route_obj, lobj):
                    route_obj.l_obj = lobj
                    objs.append(route_obj)
                    break
        return utils.GetFilteredObjects(objs, selectors.maxlimits)


RouteTableHelper = RouteTableObjectHelper()

def GetMatchingObjects(selectors):
    return RouteTableHelper.GetMatchingConfigObjects(selectors)
