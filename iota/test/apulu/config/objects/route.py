#! /usr/bin/python3
import pdb
import ipaddress
from collections import defaultdict

from infra.common.logging import logger

from iota.test.apulu.config.store import Store

import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.agent.api as api
import iota.test.apulu.config.utils as utils
import iota.test.apulu.config.objects.base as base
import iota.test.apulu.config.objects.lmapping as lmapping
import iota.test.apulu.config.objects.nexthop as nexthop

import route_pb2 as route_pb2
import tunnel_pb2 as tunnel_pb2
import types_pb2 as types_pb2

class RouteObject(base.ConfigObjectBase):
    def __init__(self, parent, af, routes, routetype, tunobj, vpcpeerid, spec):
        super().__init__(api.ObjectTypes.ROUTE)
        ################# PUBLIC ATTRIBUTES OF ROUTE TABLE OBJECT #####################
        if af == utils.IP_VERSION_6:
            self.RouteTblId = next(resmgr.V6RouteTableIdAllocator)
            self.AddrFamily = 'IPV6'
            self.NEXTHOP = nexthop.client.GetV6Nexthop(parent.Node, parent.VPCId)
        else:
            self.RouteTblId = next(resmgr.V4RouteTableIdAllocator)
            self.AddrFamily = 'IPV4'
            self.NEXTHOP = nexthop.client.GetV4Nexthop(parent.Node, parent.VPCId)
        self.GID('RouteTable%d' %self.RouteTblId)
        self.routes = routes
        self.TUNNEL = tunobj
        self.NhGroup = None
        self.DualEcmp = utils.IsDualEcmp(spec)
        if self.TUNNEL:
            self.TunnelId = self.TUNNEL.Id
            self.TunIPAddr = tunobj.RemoteIPAddr
            self.TunIP = str(self.TunIPAddr)
        else:
            self.TunnelId = 0
        self.NexthopId = self.NEXTHOP.NexthopId if self.NEXTHOP else 0
        self.VPCId = parent.VPCId
        self.VPC = parent
        self.Label = 'NETWORKING'
        self.RouteType = routetype # used for lpm route cases
        self.PeerVPCId = vpcpeerid
        self.AppPort = resmgr.TransportDstPort
        ##########################################################################
        self._derive_oper_info(spec)
        self.Show()
        return

    def _derive_oper_info(self, spec):
        # operational info useful for testspec
        routetype = self.RouteType
        self.HasDefaultRoute = True if 'default' in routetype else False
        self.VPCPeeringEnabled = True if 'vpc_peer' in routetype else False
        self.HasBlackHoleRoute = True if 'blackhole' in routetype else False
        self.HasNexthop = True if self.NEXTHOP else False
        self.TepType = getattr(spec, 'teptype', None)
        self.HasServiceTunnel = False
        if utils.IsPipelineArtemis():
            if self.TUNNEL and self.TUNNEL.Type == tunnel_pb2.TUNNEL_TYPE_SERVICE:
                self.HasServiceTunnel = True
        if self.HasBlackHoleRoute:
            self.NextHopType = "blackhole"
        elif self.VPCPeeringEnabled:
            self.NextHopType = "vpcpeer"
        else:
            if utils.IsPipelineArtemis() and \
               self.HasServiceTunnel == False and self.HasNexthop:
                self.NextHopType = "nh"
            elif utils.IsPipelineApulu() and \
                self.TepType == "overlay-ecmp":
                self.NextHopType = "nhg"
            else:
                self.NextHopType = "tep"
        return

    def __repr__(self):
        return "RouteTableID:%d|VPCId:%d|AddrFamily:%s|NumRoutes:%d|RouteType:%s"\
               %(self.RouteTblId, self.VPCId, self.AddrFamily,\
                 len(self.routes), self.RouteType)

    def Show(self):
        logger.info("RouteTable object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- HasDefaultRoute:%s|HasBlackHoleRoute:%s"\
                    %(self.HasDefaultRoute, self.HasBlackHoleRoute))
        logger.info("- VPCPeering:%s Peer Vpc%d" %(self.VPCPeeringEnabled, self.PeerVPCId))
        logger.info("- NH : Nexthop%d|Type:%s" %(self.NexthopId, self.NextHopType))
        if utils.IsPipelineApulu():
            logger.info("- Tep type:%s" % (self.TepType))
            if self.DualEcmp:
                logger.info("- Dual ecmp:%s" % (self.DualEcmp))
        if self.TUNNEL:
            logger.info("- TEP: Tunnel%d|IP:%s" %(self.TunnelId, self.TunIP))
        elif self.NhGroup:
            logger.info("- TEP: None")
            logger.info("- NexthopGroup%d" % (self.NhGroup.Id))
        for route in self.routes:
            logger.info(" -- %s" % str(route))
        return

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.route.filters)

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.RouteTblId)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.RouteTblId
        spec.Af = utils.GetRpcIPAddrFamily(self.AddrFamily)
        for route in self.routes:
            rtspec = spec.Routes.add()
            utils.GetRpcIPPrefix(route, rtspec.Prefix)
            if self.NextHopType == "vpcpeer":
                rtspec.VPCId = str.encode(str(self.PeerVPCId))
            elif self.NextHopType == "tep":
                rtspec.TunnelId = str.encode(str(self.TunnelId))
            elif self.NextHopType == "nh":
                rtspec.NexthopId = str.encode(str(self.NexthopId))
            elif self.NextHopType == "nhg":
                rtspec.NexthopGroupId = str.encode(str(self.NexthopGroupId))
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.RouteTblId:
            return False
        if spec.Af != utils.GetRpcIPAddrFamily(self.AddrFamily):
            return False
        return True

    def SetupTestcaseConfig(self, obj):
        obj.localmapping = self.l_obj
        obj.route = self
        obj.tunnel = self.TUNNEL
        obj.hostport = Store.GetHostPort()
        obj.switchport = Store.GetSwitchPort()
        obj.devicecfg = Store.GetDevice()
        obj.vpc = self.VPC
        return

class RouteObjectClient(base.ConfigClientBase):
    def __init__(self):
        def __isObjSupported():
            return utils.IsRouteTableSupported()
        super().__init__(api.ObjectTypes.ROUTE, resmgr.MAX_ROUTE_TABLE)
        self.__v4objs = defaultdict(dict)
        self.__v6objs = defaultdict(dict)
        self.__v4iter = defaultdict(dict)
        self.__v6iter = defaultdict(dict)
        self.__supported = __isObjSupported()
        return

    def PdsctlRead(self):
        # pdsctl show not supported for route table
        return

    def __internet_tunnel_get(self, nat, teptype=None):
        if teptype is not None:
            if "service" in teptype:
                if "remoteservice" == teptype:
                    return resmgr.RemoteSvcTunAllocator.rrnext()
                return resmgr.SvcTunAllocator.rrnext()
            if "underlay" in teptype:
                if "underlay-ecmp" == teptype:
                    return resmgr.UnderlayECMPTunAllocator.rrnext()
                return resmgr.UnderlayTunAllocator.rrnext()
            if "overlay-ecmp" in teptype:
                # Fill NhGroup later
                return None
        if nat is False:
            if resmgr.RemoteInternetNonNatTunAllocator != None:
                return resmgr.RemoteInternetNonNatTunAllocator.rrnext()
            else:
                return None
        else:
            return resmgr.RemoteInternetNatTunAllocator.rrnext()

    def GetRouteTableObject(self, node, routetableid):
        return self.GetObjectByKey(node, routetableid)

    def GetRouteV4Tables(self, node, vpcid):
        return self.__v4objs[node].get(vpcid, None)

    def GetRouteV6Tables(self, node, vpcid):
        return self.__v6objs[node].get(vpcid, None)

    def IsValidConfig(self, node):
        count = sum(list(map(lambda x: len(x.values()), self.__v4objs[node].values())))
        if  count > self.Maxlimit:
            return False, "V4 Route Table count %d exceeds allowed limit of %d" %\
                          (count, self.Maxlimit)
        count = sum(list(map(lambda x: len(x.values()), self.__v6objs[node].values())))
        if  count > self.Maxlimit:
            return False, "V6 Route Table count %d exceeds allowed limit of %d" %\
                          (count, self.Maxlimit)
        #TODO: check route table count equals subnet count in that VPC
        #TODO: check scale of routes per route table
        return True, ""

    def GetRouteV4Table(self, node, vpcid, routetblid):
        v4tables = self.GetRouteV4Tables(node, vpcid)
        if not v4tables:
            return None
        return v4tables.get(routetblid, None)

    def GetRouteV6Table(self, node, vpcid, routetblid):
        v6tables = self.GetRouteV6Tables(node, vpcid)
        if not v6tables:
            return None
        return v6tables.get(routetblid, None)

    def GetRouteV4TableId(self, node, vpcid):
        if self.GetRouteV4Tables(node, vpcid):
            return self.__v4iter[node][vpcid].rrnext().RouteTblId
        return 0

    def GetRouteV6TableId(self, node, vpcid):
        if self.GetRouteV6Tables(node, vpcid):
            return self.__v6iter[node][vpcid].rrnext().RouteTblId
        return 0

    def FillNhGroups(self, node):
        for robj in client.Objects(node):
            if "overlay-ecmp" in robj.TepType:
                if robj.DualEcmp is True:
                    robj.NhGroup = resmgr.DualEcmpNhGroupAllocator.rrnext()
                else:
                    robj.NhGroup = resmgr.OverlayNhGroupAllocator.rrnext()
                robj.NexthopGroupId = robj.NhGroup.Id
                logger.info("Filling NexthopGroup%d in RouteTable%d" % \
                    (robj.NhGroup.Id, robj.RouteTblId))

    def GenerateObjects(self, node, parent, vpc_spec_obj, vpcpeerid):
        if not self.__supported:
            return


        vpcid = parent.VPCId
        isV4Stack = utils.IsV4Stack(parent.Stack)
        isV6Stack = utils.IsV6Stack(parent.Stack)
        self.__v4objs[node][vpcid] = dict()
        self.__v6objs[node][vpcid] = dict()
        self.__v4iter[node][vpcid] = None
        self.__v6iter[node][vpcid] = None

        if utils.IsNatSupported():
            if resmgr.RemoteInternetNonNatTunAllocator == None and \
                resmgr.RemoteInternetNatTunAllocator == None:
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

        def __add_v4routetable(v4routes, routetype, spec):
            obj = RouteObject(parent, utils.IP_VERSION_4, v4routes, routetype, tunobj, vpcpeerid, spec)
            self.__v4objs[node][vpcid].update({obj.RouteTblId: obj})
            self.Objs[node].update({obj.RouteTblId: obj})

        def __add_v6routetable(v6routes, routetype, spec):
            obj = RouteObject(parent, utils.IP_VERSION_6, v6routes, routetype, tunobj, vpcpeerid, spec)
            self.__v6objs[node][vpcid].update({obj.RouteTblId: obj})
            self.Objs[node].update({obj.RouteTblId: obj})

        def __get_user_specified_routes(routespec):
            routes = []
            if routespec:
                for route in routespec:
                    routes.append(ipaddress.ip_network(route.replace('\\', '/')))
            return routes

        def __add_user_specified_routetable(spec, routetype):
            if isV4Stack:
                __add_v4routetable(__get_user_specified_routes(spec.v4routes), routetype, spec)

            if isV6Stack:
                __add_v6routetable(__get_user_specified_routes(spec.v6routes), routetype, spec)

        def __get_valid_route_count_per_route_table(count):
            if count > resmgr.MAX_ROUTES_PER_ROUTE_TBL:
                return resmgr.MAX_ROUTES_PER_ROUTE_TBL
            return count

        def __get_nat_teptype_from_spec(routetbl_spec_obj):
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
                    if isV4Stack:
                        routes = user_specified_v4routes + __get_adjacent_routes(v4base, v4routecount)
                        __add_v4routetable(routes, routetype, routetbl_spec_obj)
                        v4base = utils.GetNextSubnet(routes[-1])
                    if isV6Stack:
                        routes = user_specified_v6routes + __get_adjacent_routes(v6base, v6routecount)
                        __add_v6routetable(routes, routetype, routetbl_spec_obj)
                        v6base = utils.GetNextSubnet(routes[-1])

                elif 'overlap' in routetype:
                    if isV4Stack:
                        routes = user_specified_v4routes + __get_overlap(routetbl_spec_obj.v4base, v4base, v4routecount)
                        __add_v4routetable(routes, routetype, routetbl_spec_obj)
                        v4base = utils.GetNextSubnet(routes[-1])
                    if isV6Stack:
                        routes = user_specified_v6routes + __get_overlap(routetbl_spec_obj.v6base, v6base, v6routecount)
                        __add_v6routetable(routes, routetype, routetbl_spec_obj)
                        v6base = utils.GetNextSubnet(routes[-1])

        if self.__v6objs[node][vpcid]:
            self.__v6iter[node][vpcid] = utils.rrobiniter(self.__v6objs[node][vpcid].values())

        if self.__v4objs[node][vpcid]:
            self.__v4iter[node][vpcid] = utils.rrobiniter(self.__v4objs[node][vpcid].values())

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
