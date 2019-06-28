#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.nexthop as nexthop
import route_pb2 as route_pb2
import tunnel_pb2 as tunnel_pb2
import types_pb2 as types_pb2
import ipaddress

from infra.common.logging import logger
from apollo.config.store import Store

class RouteObject(base.ConfigObjectBase):
    def __init__(self, parent, af, routes, routetype, tunobj, vpcpeerid):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF ROUTE TABLE OBJECT #####################
        if af == utils.IP_VERSION_6:
            self.RouteTblId = next(resmgr.V6RouteTableIdAllocator)
            self.AddrFamily = 'IPV6'
            self.NEXTHOP = nexthop.client.GetV6Nexthop(parent.VPCId)
        else:
            self.RouteTblId = next(resmgr.V4RouteTableIdAllocator)
            self.AddrFamily = 'IPV4'
            self.NEXTHOP = nexthop.client.GetV4Nexthop(parent.VPCId)
        self.GID('RouteTbl%d'%self.RouteTblId)
        self.routes = routes
        if hasattr(self, "TUNNEL") and self.TUNNEL is not None:
        #if self.TUNNEL:
            self.TunnelId = self.TUNNEL.Id
        else:
            self.TunnelId = 0
        if self.NEXTHOP:
            self.NexthopId = self.NEXTHOP.NexthopId
        else:
            self.NexthopId = 0
        self.TUNNEL = tunobj
        self.TunIPAddr = tunobj.RemoteIPAddr
        self.TunIP = str(self.TunIPAddr)
        self.VPCId = parent.VPCId
        self.Label = 'NETWORKING'
        self.RouteType = routetype # used for lpm route cases
        self.PeerVPCId = vpcpeerid
        self.AppPort = resmgr.TransportDstPort
        ##########################################################################
        self._derive_oper_info()
        self.Show()
        return

    def _derive_oper_info(self):
        # operational info useful for testspec
        routetype = self.RouteType
        self.HasDefaultRoute = True if 'default' in routetype else False
        self.VPCPeeringEnabled = True if 'vpc_peer' in routetype else False
        self.HasBlackHoleRoute = True if 'blackhole' in routetype else False
        self.HasServiceTunnel = False
        self.HasNexthop = False
        if hasattr(self, "TUNNEL"):
            if self.TUNNEL.Type == tunnel_pb2.TUNNEL_TYPE_SERVICE and \
                    utils.IsPipelineArtemis():
                self.HasServiceTunnel = True
        if hasattr(self, "NEXTHOP") and self.NEXTHOP is not None:
            self.HasNexthop = True
        if self.HasBlackHoleRoute:
            self.NextHopType = "blackhole"
        elif self.VPCPeeringEnabled:
            self.NextHopType = "vpcpeer"
        else:
            if utils.IsPipelineArtemis() and self.TUNNEL.Type is not \
                 tunnel_pb2.TUNNEL_TYPE_SERVICE:
                self.NextHopType = "nh"
            else:
                self.NextHopType = "tep"
        return

    def __repr__(self):
        return "RouteTblID:%d|VPCId:%d|AddrFamily:%s|NumRoutes:%d|RouteType:%s|"\
               "HasDefaultRoute:%s|HasBlackHoleRoute:%s|VPCPeering:%s ID:%d|"\
               "NextHop: ID:%d Type:%s IP:%s|TEP: ID:%s"\
               %(self.RouteTblId, self.VPCId, self.AddrFamily, len(self.routes),\
                 self.RouteType, self.HasDefaultRoute, self.HasBlackHoleRoute,\
                 self.VPCPeeringEnabled, self.PeerVPCId,\
                 self.NexthopId, self.NextHopType, str(self.TunIPAddr), self.TunnelId)

    def GetGrpcCreateMessage(self):
        grpcmsg = route_pb2.RouteTableRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.RouteTblId
        spec.Af = utils.GetRpcIPAddrFamily(self.AddrFamily)
        for route in self.routes:
            rtspec = spec.Routes.add()
            utils.GetRpcIPPrefix(route, rtspec.Prefix)
            if self.NextHopType == "vpcpeer":
                rtspec.VPCId = self.PeerVPCId
            elif self.NextHopType == "tep":
                utils.GetRpcIPAddr(self.TunIPAddr, rtspec.NextHop)
            elif self.NextHopType == "nh":
                rtspec.NexthopId = self.NexthopId
        return grpcmsg

    def Show(self):
        logger.info("RouteTbl object:", self)
        logger.info("- %s" % repr(self))
        for route in self.routes:
            logger.info("-- %s" % str(route))
        return

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.route.filters)

    def SetupTestcaseConfig(self, obj):
        obj.localmapping = self.l_obj
        obj.route = self
        obj.tunnel = self.TUNNEL
        obj.hostport = utils.PortTypes.HOST
        obj.switchport = utils.PortTypes.SWITCH
        obj.devicecfg = Store.GetDevice()
        return

class RouteObjectClient:
    def __init__(self):
        self.__objs = dict()
        self.__v4objs = {}
        self.__v6objs = {}
        self.__v4iter = {}
        self.__v6iter = {}
        return

    def __internet_tunnel_get(self, nat, teptype=None):
        if teptype is not None and "service" in teptype:
            if "remote" in teptype:
                return resmgr.RemoteSvcTunAllocator.rrnext()
            return resmgr.SvcTunAllocator.rrnext()
        if nat is False:
            return resmgr.RemoteInternetNonNatTunAllocator.rrnext()
        else:
            return resmgr.RemoteInternetNatTunAllocator.rrnext()

    def Objects(self):
        return self.__objs.values()

    def IsValidConfig(self):
        count = sum(list(map(lambda x: len(x.values()), self.__v4objs.values())))
        if  count > resmgr.MAX_ROUTE_TABLE:
            return False, "V4 Route Table count %d exceeds allowed limit of %d" %\
                          (count, resmgr.MAX_ROUTE_TABLE)
        count = sum(list(map(lambda x: len(x.values()), self.__v6objs.values())))
        if  count > resmgr.MAX_ROUTE_TABLE:
            return False, "V6 Route Table count %d exceeds allowed limit of %d" %\
                          (count, resmgr.MAX_ROUTE_TABLE)
        #TODO: check route table count equals subnet count in that VPC
        #TODO: check scale of routes per route table
        return True, ""

    def GetRouteV4Table(self, vpcid, routetblid):
        return self.__v4objs[vpcid].get(routetblid, None)

    def GetRouteV6Table(self, vpcid, routetblid):
        return self.__v6objs[vpcid].get(routetblid, None)

    def GetRouteV4TableId(self, vpcid):
        if self.__v4objs[vpcid]:
            return self.__v4iter[vpcid].rrnext().RouteTblId
        return 0

    def GetRouteV6TableId(self, vpcid):
        if self.__v6objs[vpcid]:
            return self.__v6iter[vpcid].rrnext().RouteTblId
        return 0

    def GenerateObjects(self, parent, vpc_spec_obj, vpcpeerid):
        vpcid = parent.VPCId
        stack = parent.Stack
        self.__v4objs[vpcid] = dict()
        self.__v6objs[vpcid] = dict()
        self.__v4iter[vpcid] = None
        self.__v6iter[vpcid] = None

        if resmgr.RemoteInternetNonNatTunAllocator == None and resmgr.RemoteInternetNatTunAllocator == None:
            logger.info("Skipping route creation as there are no Internet tunnels")
            return

        def __get_adjacent_routes(base, count):
            routes = []
            c = 1
            routes.append(ipaddress.ip_network(base))
            while c < count:
                routes.append(utils.GetNextSubnet(routes[c-1]))
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
            obj = RouteObject(parent, utils.IP_VERSION_4, v4routes, routetype, tunobj, vpcpeerid)
            self.__v4objs[vpcid].update({obj.RouteTblId: obj})
            self.__objs.update({obj.RouteTblId: obj})

        def __add_v6routetable(v6routes, routetype):
            obj = RouteObject(parent, utils.IP_VERSION_6, v6routes, routetype, tunobj, vpcpeerid)
            self.__v6objs[vpcid].update({obj.RouteTblId: obj})
            self.__objs.update({obj.RouteTblId: obj})

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

        def __get_valid_route_count_per_route_table(count):
            if count > resmgr.MAX_ROUTES_PER_ROUTE_TBL:
                return resmgr.MAX_ROUTES_PER_ROUTE_TBL
            return count

        def __get_nat_teptype_from_spec(routetablespec):
            nat = getattr(routetbl_spec_obj, 'nat', False)
            teptype = getattr(routetbl_spec_obj, 'teptype', None)
            return nat, teptype

        for routetbl_spec_obj in vpc_spec_obj.routetbl:
            routetbltype = routetbl_spec_obj.type
            routetype = routetbl_spec_obj.routetype
            nat, teptype = __get_nat_teptype_from_spec(routetbl_spec_obj)
            tunobj = self.__internet_tunnel_get(nat, teptype)
            if routetbltype == "specific":
                __add_user_specified_routetable(routetbl_spec_obj, routetype)
                continue
            routetablecount = routetbl_spec_obj.count
            v4routecount = __get_valid_route_count_per_route_table(routetbl_spec_obj.nv4routes)
            v6routecount = __get_valid_route_count_per_route_table(routetbl_spec_obj.nv6routes)
            v4prefixlen = routetbl_spec_obj.v4prefixlen
            v6prefixlen = routetbl_spec_obj.v6prefixlen
            v4base = __get_first_subnet(ipaddress.ip_network(routetbl_spec_obj.v4base.replace('\\', '/')), v4prefixlen)
            v6base = __get_first_subnet(ipaddress.ip_network(routetbl_spec_obj.v6base.replace('\\', '/')), v6prefixlen)
            # get user specified routes if any for 'base' routetbltype
            user_specified_v4routes = __get_user_specified_routes(routetbl_spec_obj.v4routes)
            user_specified_v6routes = __get_user_specified_routes(routetbl_spec_obj.v6routes)
            v4routecount -= len(user_specified_v4routes)
            v6routecount -= len(user_specified_v6routes)
            if 'overlap' in routetype:
                v4routecount -= 1
                v6routecount -= 1
            for i in range(routetablecount):
                if 'adjacent' in routetype:
                    if __is_v4stack():
                        routes = user_specified_v4routes + __get_adjacent_routes(v4base, v4routecount)
                        __add_v4routetable(routes, routetype)
                        v4base = utils.GetNextSubnet(routes[-1])
                    if __is_v6stack():
                        routes = user_specified_v6routes + __get_adjacent_routes(v6base, v6routecount)
                        __add_v6routetable(routes, routetype)
                        v6base = utils.GetNextSubnet(routes[-1])

                elif 'overlap' in routetype:
                    if __is_v4stack():
                        routes = user_specified_v4routes + __get_overlap(routetbl_spec_obj.v4base, v4base, v4routecount)
                        __add_v4routetable(routes, routetype)
                        v4base = utils.GetNextSubnet(routes[-1])
                    if __is_v6stack():
                        routes = user_specified_v6routes + __get_overlap(routetbl_spec_obj.v6base, v6base, v6routecount)
                        __add_v6routetable(routes, routetype)
                        v6base = utils.GetNextSubnet(routes[-1])

        if self.__v6objs[vpcid]:
            self.__v6iter[vpcid] = utils.rrobiniter(self.__v6objs[vpcid].values())

        if self.__v4objs[vpcid]:
            self.__v4iter[vpcid] = utils.rrobiniter(self.__v4objs[vpcid].values())


    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs.values()))
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

    def GetMatchingConfigObjects(self, selectors, all_objs):
        objs = []
        rtype = selectors.route.GetValueByKey('RouteType')
        for route_obj in client.Objects():
            if not route_obj.IsFilterMatch(selectors):
                continue
            if rtype != 'empty' and 0 == len(route_obj.routes):
                # skip route tables with no routes
                continue
            for lobj in lmapping.GetMatchingObjects(selectors):
                if self.__is_lmapping_match(route_obj, lobj):
                    route_obj.l_obj = lobj
                    objs.append(route_obj)
                    break
        if all_objs is True:
            maxlimits = 0
        else:
            maxlimits = selectors.maxlimits
        return utils.GetFilteredObjects(objs, maxlimits)


RouteTableHelper = RouteTableObjectHelper()

def GetMatchingObjects(selectors):
    return RouteTableHelper.GetMatchingConfigObjects(selectors, False)

def GetAllMatchingObjects(selectors):
    return RouteTableHelper.GetMatchingConfigObjects(selectors, True)
