#! /usr/bin/python3
import pdb
import ipaddress
import random
from collections import OrderedDict

from infra.common.logging import logger

from apollo.config.store import Store

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.objects.base as base
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.nexthop as nexthop

import route_pb2 as route_pb2
import tunnel_pb2 as tunnel_pb2
import types_pb2 as types_pb2

DEFAULT_ROUTE_PRIORITY = 0
MIN_ROUTE_PRIORITY = 65535
MAX_ROUTE_PRIORITY = 1

class RouteObject():
    def __init__(self, ipaddress, priority=0, nh_type="", nhid=0, nhgid=0, vpcid=0, tunnelid=0):
        super().__init__()
        self.Id = next(resmgr.RouteIdAllocator)
        self.ipaddr = ipaddress
        self.Priority = priority
        self.NextHopType = nh_type
        if self.NextHopType == "vpcpeer":
            self.PeerVPCId = vpcid
        elif self.NextHopType == "tep":
            self.TunnelId = tunnelid
        elif self.NextHopType == "nh":
            self.NexthopId = nhid
        elif self.NextHopType == "nhg":
            self.NexthopGroupId = nhgid
        self.Show()

    def __repr__(self):
        return "RouteID:%d|ip:%s|priority:%d|type:%s"\
                %(self.Id, self.ipaddr, self.Priority, self.NextHopType)

    def Show(self):
        logger.info("Route object:", self)
        nh = self.NextHopType
        if self.NextHopType == "vpcpeer":
            nh = "vpc %d" % (self.PeerVPCId)
        elif self.NextHopType == "tep":
            nh = "tunnel %d" % (self.TunnelId)
        elif self.NextHopType == "nh":
            nh = "nh %d" % (self.NexthopId)
        elif self.NextHopType == "nhg":
            nh = "nhg %d" % (self.NexthopGroupId)
        logger.info(nh)

class RouteTableObject(base.ConfigObjectBase):
    def __init__(self, parent, af, routes, routetype, tunobj, vpcpeerid, spec):
        super().__init__(api.ObjectTypes.ROUTE)
        ################# PUBLIC ATTRIBUTES OF ROUTE TABLE OBJECT #####################
        if af == utils.IP_VERSION_6:
            self.RouteTblId = next(resmgr.V6RouteTableIdAllocator)
            self.AddrFamily = 'IPV6'
            self.NEXTHOP = nexthop.client.GetV6Nexthop(parent.VPCId)
        else:
            self.RouteTblId = next(resmgr.V4RouteTableIdAllocator)
            self.AddrFamily = 'IPV4'
            self.NEXTHOP = nexthop.client.GetV4Nexthop(parent.VPCId)
        self.GID('RouteTable%d' %self.RouteTblId)
        self.routes = routes
        #resmgr.ResetRouteIdAllocator() #TODO fix bug while generating multiple route tables
        self.TUNNEL = tunobj
        self.NhGroup = None
        self.DualEcmp = utils.IsDualEcmp(spec)
        self.PriorityType = getattr(spec, "priority", None)
        if self.TUNNEL:
            self.TunnelId = self.TUNNEL.Id
            self.TunIPAddr = tunobj.RemoteIPAddr
            self.TunIP = str(self.TunIPAddr)
            self.TunEncap = tunobj.EncapValue
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
        for route in self.routes.values():
            route.Show()
        return

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.route.filters)

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.RouteTblId)
        return

    def PopulateNh(self, rtspec, route):
        if route.NextHopType == "vpcpeer":
            rtspec.VPCId = route.PeerVPCId
        elif route.NextHopType == "tep":
            rtspec.TunnelId = route.TunnelId
        elif route.NextHopType == "nh":
            rtspec.NexthopId = route.NexthopId
        elif route.NextHopType == "nhg":
            rtspec.NexthopGroupId = route.NexthopGroupId

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.RouteTblId
        spec.Af = utils.GetRpcIPAddrFamily(self.AddrFamily)
        for route in self.routes.values():
            rtspec = spec.Routes.add()
            utils.GetRpcIPPrefix(route.ipaddr, rtspec.Prefix)
            if self.PriorityType:
                rtspec.Priority = route.Priority
                self.PopulateNh(rtspec, route)
            else:
                self.PopulateNh(rtspec, self)
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
        utils.DumpTestcaseConfig(obj)
        return

class RouteObjectClient(base.ConfigClientBase):
    def __init__(self):
        def __isObjSupported():
            return utils.IsRouteTableSupported()
        super().__init__(api.ObjectTypes.ROUTE, resmgr.MAX_ROUTE_TABLE)
        self.__v4objs = {}
        self.__v6objs = {}
        self.__v4iter = {}
        self.__v6iter = {}
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
            return resmgr.RemoteInternetNonNatTunAllocator.rrnext()
        else:
            return resmgr.RemoteInternetNatTunAllocator.rrnext()

    def GetRouteTableObject(self, routetableid):
        return self.GetObjectByKey(routetableid)

    def GetRouteV4Tables(self, vpcid):
        return self.__v4objs.get(vpcid, None)

    def GetRouteV6Tables(self, vpcid):
        return self.__v6objs.get(vpcid, None)

    def IsValidConfig(self):
        count = sum(list(map(lambda x: len(x.values()), self.__v4objs.values())))
        if  count > self.Maxlimit:
            return False, "V4 Route Table count %d exceeds allowed limit of %d" %\
                          (count, self.Maxlimit)
        count = sum(list(map(lambda x: len(x.values()), self.__v6objs.values())))
        if  count > self.Maxlimit:
            return False, "V6 Route Table count %d exceeds allowed limit of %d" %\
                          (count, self.Maxlimit)
        #TODO: check route table count equals subnet count in that VPC
        #TODO: check scale of routes per route table
        return True, ""

    def GetRouteV4Table(self, vpcid, routetblid):
        v4tables = self.GetRouteV4Tables(vpcid)
        if not v4tables:
            return None
        return v4tables.get(routetblid, None)

    def GetRouteV6Table(self, vpcid, routetblid):
        v6tables = self.GetRouteV6Tables(vpcid)
        if not v6tables:
            return None
        return v6tables.get(routetblid, None)

    def GetRouteV4TableId(self, vpcid):
        if self.GetRouteV4Tables(vpcid):
            return self.__v4iter[vpcid].rrnext().RouteTblId
        return 0

    def GetRouteV6TableId(self, vpcid):
        if self.GetRouteV6Tables(vpcid):
            return self.__v6iter[vpcid].rrnext().RouteTblId
        return 0

    def FillNhGroups(self):
        for robj in client.Objects():
            if "overlay-ecmp" in robj.TepType:
                if robj.DualEcmp is True:
                    robj.NhGroup = resmgr.DualEcmpNhGroupAllocator.rrnext()
                else:
                    robj.NhGroup = resmgr.OverlayNhGroupAllocator.rrnext()
                robj.NexthopGroupId = robj.NhGroup.Id
                logger.info("Filling NexthopGroup%d in RouteTable%d" % \
                    (robj.NhGroup.Id, robj.RouteTblId))

    def GenerateObjects(self, parent, vpc_spec_obj, vpcpeerid):
        if not self.__supported:
            return


        vpcid = parent.VPCId
        isV4Stack = utils.IsV4Stack(parent.Stack)
        isV6Stack = utils.IsV6Stack(parent.Stack)
        self.__v4objs[vpcid] = dict()
        self.__v6objs[vpcid] = dict()
        self.__v4iter[vpcid] = None
        self.__v6iter[vpcid] = None

        if utils.IsNatSupported():
            if resmgr.RemoteInternetNonNatTunAllocator == None and \
                resmgr.RemoteInternetNatTunAllocator == None:
                logger.info("Skipping route creation as there are no Internet tunnels")
                return

        def __get_adjacent_routes(base, count):
            routes = OrderedDict()
            ipaddr = ipaddress.ip_network(base)
            af = ipaddr.version
            spec = routetbl_spec_obj
            priority = DEFAULT_ROUTE_PRIORITY
            priorityType = getattr(spec, "priority", None)
            if priorityType:
                priority = __get_priority(priorityType, True)
            nh_type, nh_id, nhgid, vpcid, tunnelid = __get_route_attribs(spec, af)
            obj = RouteObject(ipaddr, priority, nh_type, nh_id, nhgid, vpcid, tunnelid)
            routes.update({obj.Id: obj})
            c = 1
            while c < count:
                ipaddr = utils.GetNextSubnet(ipaddr)
                if priorityType:
                    priority = __get_priority(priorityType, False, priority)
                nh_type, nh_id, nhgid, vpcid, tunnelid = __get_route_attribs(spec, af)
                obj = RouteObject(ipaddr, priority, nh_type, nh_id, nhgid, vpcid, tunnelid)
                routes.update({obj.Id: obj})
                c += 1
            return routes

        def __get_overlap(basepfx, base, count):
            # for overlap, add user specified base prefix with original prefixlen
            routes = __get_user_specified_routes([basepfx])
            routes = utils.MergeDicts(routes, __get_adjacent_routes(base, count))
            return routes

        def __get_first_subnet(ip, prefixlen):
            for ip in ip.subnets(new_prefix=prefixlen):
                return (ip)
            return

        def __add_v4routetable(v4routes, spec):
            obj = RouteTableObject(parent, utils.IP_VERSION_4, v4routes, spec.routetype, tunobj, vpcpeerid, spec)
            self.__v4objs[vpcid].update({obj.RouteTblId: obj})
            self.Objs.update({obj.RouteTblId: obj})

        def __add_v6routetable(v6routes, spec):
            obj = RouteTableObject(parent, utils.IP_VERSION_6, v6routes, spec.routetype, tunobj, vpcpeerid, spec)
            self.__v6objs[vpcid].update({obj.RouteTblId: obj})
            self.Objs.update({obj.RouteTblId: obj})

        def __derive_nh_type_info(spec):
            routetype = spec.routetype
            if 'vpc_peer' in routetype:
                return 'vpcpeer'
            elif 'blackhole' in routetype:
                return 'blackhole'
            elif utils.IsPipelineArtemis():
                # handle service tunnel case, return tep
                return "nh"
            elif utils.IsPipelineApulu():
                if 'overlay-ecmp' in spec.teptype:
                    return "nhg"
                else:
                    return "tep"
            else:
                return "tep"

        def __get_tunnel(spec):
            routetype = spec.routetype
            nat, teptype = __get_nat_teptype_from_spec(spec)
            tunobj = self.__internet_tunnel_get(nat, teptype)
            return tunobj

        def __get_nexthop(af):
            nh = None
            if af == utils.IP_VERSION_4:
                nh = nexthop.client.GetV4Nexthop(parent.VPCId)
            else:
                nh = nexthop.client.GetV6Nexthop(parent.VPCId)
            return nh

        def __get_route_attribs(spec, af=utils.IP_VERSION_4):
            nhid = 0
            nhgid = 0
            vpcid = 0
            tunnelid = 0
            nh_type = __derive_nh_type_info(spec)
            if nh_type == "vpcpeer":
                vpcid = vpcpeerid
            elif nh_type == "tep":
                tunobj = __get_tunnel(spec)
                if utils.IsPipelineArtemis() and tunobj.Type == tunnel_pb2.TUNNEL_TYPE_SERVICE:
                    # service tunnel case
                    nh_type = "nh"
                    nexthop = __get_nexthop(af)
                    nh_id = nexthop.NexthopId
                else:
                    tunnelid = tunobj.Id
            elif nh_type == "nh":
                nexthop = __get_nexthop(af)
                if nexthop:
                    nhid = nexthop.NexthopId
            elif nh_type == "nhg":
                nhgid = 1 # fill later
            return nh_type, nhid, nhgid, vpcid, tunnelid

        def __get_priority(priotype, firstVal=False, priority=0):
            if priotype ==  "increasing":
                if firstVal: return MIN_ROUTE_PRIORITY
                return (priority - 1)
            elif priotype == "decreasing":
                if firstVal: return MAX_ROUTE_PRIORITY
                return (priority + 1)
            elif priotype == "random":
                return (random.randint(MAX_ROUTE_PRIORITY, MIN_ROUTE_PRIORITY))
            else:
                logger.error("Unknown priority type", priotype)
                return (random.randint(MAX_ROUTE_PRIORITY, MIN_ROUTE_PRIORITY))

        def __get_user_specified_routes(routespec):
            routes = OrderedDict()
            spec = routetbl_spec_obj
            priorityType = getattr(spec, "priority", None)
            priority = DEFAULT_ROUTE_PRIORITY
            if priorityType:
                priority = __get_priority(spec.priority, True)
            if routespec:
                for route in routespec:
                    if priorityType:
                        priority = __get_priority(spec.priority, False, priority)
                    nh_type, nh_id, nhgid, vpcid, tunnelid = __get_route_attribs(spec)
                    obj = RouteObject(ipaddress.ip_network(route.replace('\\', '/')),\
                                          priority, nh_type, nh_id, nhgid, vpcid, tunnelid)
                    routes.update({obj.Id: obj})
            return routes

        def __add_user_specified_routetable(spec):
            if isV4Stack:
                __add_v4routetable(__get_user_specified_routes(spec.v4routes), spec)

            if isV6Stack:
                __add_v6routetable(__get_user_specified_routes(spec.v6routes), spec)

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
                __add_user_specified_routetable(routetbl_spec_obj)
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
                        routes = utils.MergeDicts(user_specified_v4routes, \
                                    __get_adjacent_routes(v4base, v4routecount-1))
                        __add_v4routetable(routes, routetbl_spec_obj)
                        v4base = utils.GetNextSubnet(routes.get(list(routes)[-1]).ipaddr)
                    if isV6Stack:
                        routes = utils.MergeDicts(user_specified_v6routes, \
                                    __get_adjacent_routes(v6base, v6routecount-1))
                        __add_v6routetable(routes, routetbl_spec_obj)
                        v6base = utils.GetNextSubnet(routes.get(list(routes)[-1]).ipaddr)

                elif 'overlap' in routetype:
                    if isV4Stack:
                        routes = utils.MergeDicts(user_specified_v4routes, \
                                    __get_overlap(routetbl_spec_obj.v4base, v4base, v4routecount))
                        __add_v4routetable(routes, routetbl_spec_obj)
                        v4base = utils.GetNextSubnet(routes.get(list(routes)[-1]).ipaddr)
                    if isV6Stack:
                        routes = utils.MergeDicts(user_specified_v6routes, \
                                    __get_overlap(routetbl_spec_obj.v6base, v6base, v6routecount))
                        __add_v6routetable(routes, routetbl_spec_obj)
                        v6base = utils.GetNextSubnet(routes.get(list(routes)[-1]).ipaddr)

        if self.__v6objs[vpcid]:
            self.__v6iter[vpcid] = topo.rrobiniter(self.__v6objs[vpcid].values())

        if self.__v4objs[vpcid]:
            self.__v4iter[vpcid] = topo.rrobiniter(self.__v4objs[vpcid].values())

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
