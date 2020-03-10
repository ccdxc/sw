#! /usr/bin/python3
import pdb
import ipaddress
import random
import copy
import json
from collections import OrderedDict, defaultdict

from infra.common.logging import logger
from infra.common.glopts  import GlobalOptions

from apollo.config.store import EzAccessStore
from apollo.config.store import client as EzAccessStoreClient

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.objects.base as base
import apollo.config.objects.lmapping as lmapping
from apollo.config.objects.nexthop import client as NexthopClient
from apollo.config.objects.nexthop_group import client as NexthopGroupClient
from apollo.config.objects.tunnel import client as TunnelClient
from apollo.config.agent.api import ObjectTypes as ObjectTypes

class RouteObject():
    def __init__(self, node, ipaddress, priority=0, nh_type="", nhid=0, nhgid=0, vpcid=0, tunnelid=0, nat_type=None):
        super().__init__()
        self.Id = next(ResmgrClient[node].RouteIdAllocator)
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
        if nat_type == "static":
            self.SNatAction = topo.NatActionTypes.STATIC
        elif nat_type == "napt":
            self.SNatAction = topo.NatActionTypes.NAPT
        else:
            self.SNatAction = topo.NatActionTypes.NONE

    def __repr__(self):
        return "RouteID:%d|ip:%s|priority:%d|type:%s"\
                %(self.Id, self.ipaddr, self.Priority, self.NextHopType)

    def Show(self):
        logger.info("  - Route object:", self)
        nh = self.NextHopType
        if self.NextHopType == "vpcpeer":
            nh = "vpc %d" % (self.PeerVPCId)
        elif self.NextHopType == "tep":
            nh = "tunnel %d" % (self.TunnelId)
        elif self.NextHopType == "nh":
            nh = "nh %d" % (self.NexthopId)
        elif self.NextHopType == "nhg":
            nh = "nhg %d" % (self.NexthopGroupId)
        logger.info("     NH info: %s" % (nh))
        logger.info("     SNAT action %s" % (self.SNatAction.name))

class RouteTableObject(base.ConfigObjectBase):
    def __init__(self, node, parent, af, routes, routetype, tunobj, vpcpeerid, spec):
        super().__init__(api.ObjectTypes.ROUTE, node)
        if hasattr(spec, 'origin'):
            self.SetOrigin(spec.origin)
        elif (EzAccessStoreClient[node].IsDeviceOverlayRoutingEnabled()):
            self.SetOrigin('discovered')
        ################# PUBLIC ATTRIBUTES OF ROUTE TABLE OBJECT #####################
        if af == utils.IP_VERSION_6:
            self.RouteTblId = next(ResmgrClient[node].V6RouteTableIdAllocator)
            self.AddrFamily = 'IPV6'
            self.NEXTHOP = NexthopClient.GetV6Nexthop(node, parent.VPCId)
        else:
            self.RouteTblId = next(ResmgrClient[node].V4RouteTableIdAllocator)
            self.AddrFamily = 'IPV4'
            self.NEXTHOP = NexthopClient.GetV4Nexthop(node, parent.VPCId)
        self.GID('RouteTable%d' %self.RouteTblId)
        if af == utils.IP_VERSION_4:
            parent.V4RouteTableName = self.GID()
        self.UUID = utils.PdsUuid(self.RouteTblId, self.ObjType)
        self.routes = routes
        self.TUNNEL = tunobj
        self.NhGroup = None
        self.DualEcmp = utils.IsDualEcmp(spec)
        self.PriorityType = getattr(spec, "priority", None)
        if self.TUNNEL:
            self.TunnelId = self.TUNNEL.Id
            self.TunEncap = tunobj.EncapValue
        else:
            self.TunnelId = 0
        self.NexthopId = self.NEXTHOP.NexthopId if self.NEXTHOP else 0
        self.VPCId = parent.VPCId
        self.VPC = parent
        self.Label = 'NETWORKING'
        self.RouteType = routetype # used for lpm route cases
        self.PeerVPCId = vpcpeerid
        self.AppPort = ResmgrClient[node].TransportDstPort
        self.Mutable = utils.IsUpdateSupported()
        ##########################################################################
        self.DeriveOperInfo(spec)
        self.Show()
        return

    def __repr__(self):
        return "RouteTable: %s |VPCId:%d|AddrFamily:%s|NumRoutes:%d|RouteType:%s"\
               % (self.UUID, self.VPCId, self.AddrFamily,\
                 len(self.routes), self.RouteType)

    def Show(self):
        logger.info("RouteTable object:", self)
        logger.info("- %s" % repr(self))
        logger.info(f"- PriorityEnabled:{self.PriorityType != None}")
        logger.info("- HasDefaultRoute:%s|HasBlackHoleRoute:%s"\
                    %(self.HasDefaultRoute, self.HasBlackHoleRoute))
        logger.info("- VPCPeering:%s Peer Vpc%d" %(self.VPCPeeringEnabled, self.PeerVPCId))
        logger.info("- NH : Nexthop%d|Type:%s" %(self.NexthopId, self.NextHopType))
        if utils.IsPipelineApulu():
            logger.info("- Tep type:%s" % (self.TepType))
            if self.DualEcmp:
                logger.info("- Dual ecmp:%s" % (self.DualEcmp))
        if self.TUNNEL:
            logger.info("- TEP: Tunnel%d|IP:%s" %(self.TunnelId, str(self.TUNNEL.RemoteIPAddr)))
        elif self.NhGroup:
            logger.info("- TEP: None")
            logger.info("- NexthopGroup%d" % (self.NhGroup.Id))
        for route in self.routes.values():
            route.Show()
        return

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.route.filters)

    def CopyObject(self):
        clone = copy.copy(self)
        clone.routes = self.routes
        self.routes = copy.deepcopy(clone.routes)
        return clone

    def UpdateAttributes(self):
        ipaddr = utils.GetNextSubnet(self.routes.get(list(self.routes)[-1]).ipaddr)
        for route in self.routes.values():
            ipaddr = utils.GetNextSubnet(ipaddr)
            route.ipaddr = ipaddr
            if route.NextHopType == "nh":
                if af == utils.IP_VERSION_4:
                    nh = nexthop.client.GetV4Nexthop(self.Node, self.VPCId)
                else:
                    nh = nexthop.client.GetV6Nexthop(self.Node, self.VPCId)
                route.NexthopId = nh.NexthopId
        return

    def RollbackAttributes(self):
        self.routes = self.GetPrecedent().routes
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateNh(self, rtspec, route):
        if route.NextHopType == "vpcpeer":
            rtspec.VPCId = utils.PdsUuid.GetUUIDfromId(route.PeerVPCId, ObjectTypes.VPC)
        elif route.NextHopType == "tep":
            rtspec.TunnelId = utils.PdsUuid.GetUUIDfromId(route.TunnelId, ObjectTypes.TUNNEL)
        elif route.NextHopType == "nh":
            rtspec.NexthopId = utils.PdsUuid.GetUUIDfromId(route.NexthopId, ObjectTypes.NEXTHOP)
        elif route.NextHopType == "nhg":
            rtspec.NexthopGroupId = utils.PdsUuid.GetUUIDfromId(route.NexthopGroupId, ObjectTypes.NEXTHOPGROUP)

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.Af = utils.GetRpcIPAddrFamily(self.AddrFamily)
        spec.EnablePBR = (self.PriorityType != None)
        for route in self.routes.values():
            rtspec = spec.Routes.add()
            rtspec.NatAction.SrcNatAction = route.SNatAction
            utils.GetRpcIPPrefix(route.ipaddr, rtspec.Prefix)
            if self.PriorityType:
                rtspec.Priority = route.Priority
                self.PopulateNh(rtspec, route)
            else:
                #TODO move to per route populate nh eventually
                self.PopulateNh(rtspec, self)
        return

    def PopulateAgentJson(self):
        # TBD: the actual route table spec
        spec = {
            "kind": "RouteTable",
            "meta": {
                "name": self.GID(),
                "namespace": "default",
                "tenant": self.VPC.GID(),
                "uuid" : self.UUID.UuidStr,
                "labels": {
                    "CreatedBy": "Venice"
                },
            },
            "spec": {
            }
        }
        return json.dumps(spec)

    def ValidateJSONSpec(self, spec):
        if spec['kind'] != 'RouteTable': return False
        if spec['meta']['name'] != self.GID(): return False
        return True

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.Af != utils.GetRpcIPAddrFamily(self.AddrFamily):
            return False
        return True

    def GetDependees(self, node):
        """
        depender/dependent - route table
        dependee - vpc, tunnel, nexthop, & nexthop_group
        """
        # TODO: get vpc
        dependees = [ ]
        return dependees #updating routes in routetable for dependees is yet to be done
        if self.TUNNEL:
            dependees.append(self.TUNNEL)
        for route in self.routes.values():
            if route.NextHopType == "vpcpeer":
                # TODO: get VPC object
                pass
            elif route.NextHopType == "tep":
                tunnelObj = TunnelClient.GetObjectByKey(node, route.TunnelId)
                dependees.append(tunnelObj)
            elif route.NextHopType == "nh":
                nhObj = NexthopClient.GetObjectByKey(node, route.NexthopId)
                dependees.append(nhObj)
            elif route.NextHopType == "nhg":
                nhgObj = NexthopGroupClient.GetObjectByKey(node, route.NexthopGroupId)
                dependees.append(nhgObj)
        return dependees

    def DeriveOperInfo(self, spec):
        # operational info useful for testspec
        routetype = self.RouteType
        self.HasDefaultRoute = True if 'default' in routetype else False
        self.VPCPeeringEnabled = True if 'vpc_peer' in routetype else False
        self.HasBlackHoleRoute = True if 'blackhole' in routetype else False
        self.HasNexthop = True if self.NEXTHOP else False
        self.TepType = getattr(spec, 'teptype', None)
        self.HasServiceTunnel = False
        if utils.IsPipelineArtemis():
            if self.TUNNEL and self.TUNNEL.IsSvc():
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
        self.NatLevel = None
        natspec = getattr(spec, 'nat', None)
        if natspec:
            self.NatLevel = getattr(natspec, 'level', None)
        super().DeriveOperInfo()
        return

    def RestoreNotify(self, cObj):
        logger.info("Notify %s for %s creation" % (self, cObj))
        if not self.IsHwHabitant():
            logger.info(" - Skipping notification as %s already deleted" % self)
            return
        logger.info(" - Linking %s to %s " % (cObj, self))
        if cObj.ObjType == api.ObjectTypes.TUNNEL:
            self.TunnelId = cObj.Id
        elif cObj.ObjType == api.ObjectTypes.NEXTHOP or\
             cObj.ObjType == api.ObjectTypes.NEXTHOPGROUP:
            logger.info(" - Nh or Nhg updated, ignoring for now")
        else:
            logger.error(" - ERROR: %s not handling %s restoration" %\
                         (self.ObjType.name, cObj.ObjType))
            assert(0)
        self.SetDirty(True)
        self.CommitUpdate()
        return

    def DeleteNotify(self, dObj):
        logger.info("Notify %s for %s deletion" % (self, dObj))
        if not self.IsHwHabitant():
            logger.info(" - Skipping notification as %s already deleted" % self)
            return
        logger.info(" - Unlinking %s from %s " % (dObj, self))
        if dObj.ObjType == api.ObjectTypes.TUNNEL:
            self.TunnelId = dObj.Duplicate.Id 
        elif dObj.ObjType == api.ObjectTypes.NEXTHOP or\
             dObj.ObjType == api.ObjectTypes.NEXTHOPGROUP:
            logger.info(" - Nh or Nhg updated, ignoring for now")
        else:
            logger.error(" - ERROR: %s not handling %s deletion" %\
                         (self.ObjType.name, dObj.ObjType))
            assert(0)
        self.SetDirty(True)
        self.CommitUpdate()
        return

    def IsNatEnabled(self):
        natlevel = self.NatLevel
        if natlevel == 'tunnel':
            tunnel = self.TUNNEL
            if tunnel is not None:
                return tunnel.Nat
            return False
        elif natlevel == 'route':
            # all routes in route table has same NAT action in DOL
            # TODO: make it random / rrobin
            return True
        return False

    def SetupTestcaseConfig(self, obj):
        obj.localmapping = self.l_obj
        obj.route = self
        obj.tunnel = self.TUNNEL
        obj.hostport = EzAccessStoreClient[self.Node].GetHostPort()
        obj.switchport = EzAccessStoreClient[self.Node].GetSwitchPort()
        obj.devicecfg = EzAccessStoreClient[self.Node].GetDevice()
        obj.vpc = self.VPC
        utils.DumpTestcaseConfig(obj)
        return

class RouteObjectClient(base.ConfigClientBase):
    def __init__(self):
        def __isObjSupported():
            return utils.IsRouteTableSupported()
        def __isIPv6RouteTableSupported():
            if utils.IsPipelineApulu():
                return False
            return True
        super().__init__(api.ObjectTypes.ROUTE, Resmgr.MAX_ROUTE_TABLE)
        self.__v4objs = defaultdict(dict)
        self.__v6objs = defaultdict(dict)
        self.__v4iter = defaultdict(dict)
        self.__v6iter = defaultdict(dict)
        self.__supported = __isObjSupported()
        self.__v6supported = __isIPv6RouteTableSupported()
        return

    def PdsctlRead(self, node):
        # pdsctl show not supported for route table
        return True

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
        logger.info(f"Generated {count} IPv4 {self.ObjType.name}TABLE Objects in {node}")
        count = sum(list(map(lambda x: len(x.values()), self.__v6objs[node].values())))
        if  count > self.Maxlimit:
            return False, "V6 Route Table count %d exceeds allowed limit of %d" %\
                          (count, self.Maxlimit)
        #TODO: check route table count equals subnet count in that VPC
        #TODO: check scale of routes per route table
        logger.info(f"Generated {count} IPv6 {self.ObjType.name}TABLE Objects in {node}")
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
                    robj.NhGroup = ResmgrClient[node].DualEcmpNhGroupAllocator.rrnext()
                else:
                    robj.NhGroup = ResmgrClient[node].OverlayNhGroupAllocator.rrnext()
                robj.NexthopGroupId = robj.NhGroup.Id
                logger.info("Filling NexthopGroup%d in RouteTable%d" % \
                    (robj.NhGroup.Id, robj.RouteTblId))

    def GenerateObjects(self, node, parent, vpc_spec_obj, vpcpeerid):
        if not self.__supported:
            return


        vpcid = parent.VPCId
        isV4Stack = utils.IsV4Stack(parent.Stack)
        isV6Stack = utils.IsV6Stack(parent.Stack) and self.__v6supported

        self.__v4objs[node][vpcid] = dict()
        self.__v6objs[node][vpcid] = dict()
        self.__v4iter[node][vpcid] = None
        self.__v6iter[node][vpcid] = None

        if utils.IsNatSupported():
            if ResmgrClient[node].RemoteInternetNonNatTunAllocator == None and \
                ResmgrClient[node].RemoteInternetNatTunAllocator == None:
                logger.info("Skipping route creation as there are no Internet tunnels")
                return

        def __get_adjacent_routes(base, count):
            routes = OrderedDict()
            ipaddr = ipaddress.ip_network(base)
            af = ipaddr.version
            spec = routetbl_spec_obj
            priority = topo.DEFAULT_ROUTE_PRIORITY
            priorityType = getattr(spec, "priority", None)
            if priorityType:
                priority = __get_priority(priorityType, True)
            nh_type, nh_id, nhgid, vpcid, tunnelid, nat_type = __get_route_attribs(spec, af)
            obj = RouteObject(node, ipaddr, priority, nh_type, nh_id, nhgid, vpcid, tunnelid, nat_type)
            routes.update({obj.Id: obj})
            c = 1
            while c < count:
                ipaddr = utils.GetNextSubnet(ipaddr)
                if priorityType:
                    priority = __get_priority(priorityType, False, priority)
                nh_type, nh_id, nhgid, vpcid, tunnelid, nat_type = __get_route_attribs(spec, af)
                obj = RouteObject(node, ipaddr, priority, nh_type, nh_id, nhgid, vpcid, tunnelid, nat_type)
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
            obj = RouteTableObject(node, parent, utils.IP_VERSION_4, v4routes, spec.routetype, tunobj, vpcpeerid, spec)
            self.__v4objs[node][vpcid].update({obj.RouteTblId: obj})
            self.Objs[node].update({obj.RouteTblId: obj})

        def __add_v6routetable(v6routes, spec):
            obj = RouteTableObject(node, parent, utils.IP_VERSION_6, v6routes, spec.routetype, tunobj, vpcpeerid, spec)
            self.__v6objs[node][vpcid].update({obj.RouteTblId: obj})
            self.Objs[node].update({obj.RouteTblId: obj})

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

        def __internet_tunnel_get(nat, teptype=None):
            if teptype is not None:
                if "service" in teptype:
                    if "remoteservice" == teptype:
                        return ResmgrClient[node].RemoteSvcTunAllocator.rrnext()
                    return ResmgrClient[node].SvcTunAllocator.rrnext()
                if "underlay" in teptype:
                    if "underlay-ecmp" == teptype:
                        return ResmgrClient[node].UnderlayECMPTunAllocator.rrnext()
                    return ResmgrClient[node].UnderlayTunAllocator.rrnext()
                if "overlay-ecmp" in teptype:
                    # Fill NhGroup later
                    return None
            if nat is False:
                return ResmgrClient[node].RemoteInternetNonNatTunAllocator.rrnext()
            else:
                return ResmgrClient[node].RemoteInternetNatTunAllocator.rrnext()

        def __get_tunnel(spec):
            routetype = spec.routetype
            nat, teptype = __get_nat_teptype_from_spec(spec)
            tunobj = __internet_tunnel_get(nat, teptype)
            return tunobj

        def __get_nexthop(af):
            nh = None
            if af == utils.IP_VERSION_4:
                nh = NexthopClient.GetV4Nexthop(node, parent.VPCId)
            else:
                nh = NexthopClient.GetV6Nexthop(node, parent.VPCId)
            return nh

        def __get_route_attribs(spec, af=utils.IP_VERSION_4):
            nhid = 0
            nhgid = 0
            vpcid = 0
            tunnelid = 0
            nat_type = None
            natspec = getattr(spec, 'nat', None)
            if natspec:
                nat_type = getattr(natspec, 'type', None)
                nat_level = getattr(natspec, 'level', None)
            nh_type = __derive_nh_type_info(spec)
            if nh_type == "vpcpeer":
                vpcid = vpcpeerid
            elif nh_type == "tep":
                tunobj = __get_tunnel(spec)
                if utils.IsPipelineArtemis() and tunobj.IsSvc():
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
            return nh_type, nhid, nhgid, vpcid, tunnelid, nat_type

        def __get_priority(priotype, firstVal=False, priority=0):
            if priotype ==  "increasing":
                if firstVal: return topo.MIN_ROUTE_PRIORITY
                return (priority - 1)
            elif priotype == "decreasing":
                if firstVal: return topo.MAX_ROUTE_PRIORITY
                return (priority + 1)
            elif priotype == "random":
                return (random.randint(topo.MAX_ROUTE_PRIORITY, topo.MIN_ROUTE_PRIORITY))
            else:
                logger.error("Unknown priority type", priotype)
                return (random.randint(topo.MAX_ROUTE_PRIORITY, topo.MIN_ROUTE_PRIORITY))

        def __get_user_specified_routes(routespec):
            routes = OrderedDict()
            spec = routetbl_spec_obj
            priorityType = getattr(spec, "priority", None)
            priority = topo.DEFAULT_ROUTE_PRIORITY
            if priorityType:
                priority = __get_priority(spec.priority, True)
            if routespec:
                for route in routespec:
                    if priorityType:
                        priority = __get_priority(spec.priority, False, priority)
                    nh_type, nh_id, nhgid, vpcid, tunnelid, nat_type = __get_route_attribs(spec)
                    obj = RouteObject(node, ipaddress.ip_network(route.replace('\\', '/')),\
                                          priority, nh_type, nh_id, nhgid, vpcid, tunnelid, nat_type)
                    routes.update({obj.Id: obj})
            return routes

        def __add_user_specified_routetable(spec):
            if isV4Stack:
                __add_v4routetable(__get_user_specified_routes(spec.v4routes), spec)

            if isV6Stack:
                __add_v6routetable(__get_user_specified_routes(spec.v6routes), spec)

        def __get_valid_route_count_per_route_table(count):
            if count > Resmgr.MAX_ROUTES_PER_ROUTE_TBL:
                return Resmgr.MAX_ROUTES_PER_ROUTE_TBL
            return count

        def __get_nat_teptype_from_spec(routetbl_spec_obj):
            nat = False
            natSpec = getattr(routetbl_spec_obj, 'nat', None)
            if natSpec:
                level = getattr(natSpec, 'level')
                if level == 'tunnel' and not utils.IsPipelineApulu():
                    nat = True
            teptype = getattr(routetbl_spec_obj, 'teptype', None)
            return nat, teptype

        for routetbl_spec_obj in vpc_spec_obj.routetbl:
            routetbltype = routetbl_spec_obj.type
            routetype = routetbl_spec_obj.routetype
            nat, teptype = __get_nat_teptype_from_spec(routetbl_spec_obj)
            tunobj = __internet_tunnel_get(nat, teptype)
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
        dutNode = EzAccessStore.GetDUTNode()
        for route_obj in client.Objects(dutNode):
            if not route_obj.IsFilterMatch(selectors):
                continue
            if rtype != 'empty' and 0 == len(route_obj.routes):
                # skip route tables with no routes
                continue
            for lobj in lmapping.GetMatchingObjects(selectors, dutNode):
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
