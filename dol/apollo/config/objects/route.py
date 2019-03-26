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
    def __init__(self, parent, prefix, tunobj):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF SUBNET OBJECT #####################
        #self.RouteTblId = next(resmgr.RouteTableIdAllocator) ## TODO - Randum routetable id needs to be revisited 
        self.RouteTblId = prefix.version
        self.GID('RouteTbl%d'%self.RouteTblId)
        if prefix.version == 6:
            self.Prefix = prefix
            self.AddrFamily = 'IPV6'
        else:
            self.Prefix = prefix
            self.AddrFamily = 'IPV4'
        self.Tunnel = tunobj
        self.TunIPAddr = tunobj.RemoteIPAddr
        self.TunIP = str(self.TunIPAddr)
        self.VPCId = parent.VPCId
        self.Label = 'NETWORKING'
        #assert nroutes == 1 # TODO -- Walking thru count moved to client code
        self.TestDestination = str(next(self.Prefix.hosts()))
        ##########################################################################

        self.Show()
        return

    def __repr__(self):
        return "RouteTblID:%d|VPCId:%d|Prefix:%s|AddrFamily:%s|NextHop:%s" %\
               (self.RouteTblId, self.VPCId, str(self.Prefix),
                self.AddrFamily, str(self.TunIPAddr))

    def GetGrpcCreateMessage(self):
        grpcmsg = route_pb2.RouteTableRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.RouteTblId
        rtspec = spec.Routes.add()
        utils.GetRpcIPPrefix(self.Prefix, rtspec.Prefix)
        spec.AF = rtspec.Prefix.Addr.Af
        rtspec.NextHop.Af = types_pb2.IP_AF_INET
        rtspec.NextHop.V4Addr = int(self.TunIPAddr)
        rtspec.VPCId = self.VPCId
        return grpcmsg

    def Show(self):
        logger.info("RouteTbl object:", self)
        logger.info("- %s" % repr(self))
        return

    def __is_lmapping_match(self, lmapping):
        if lmapping.AddrFamily == 'IPV4':
            return lmapping.AddrFamily == self.AddrFamily and\
               lmapping.VNIC.SUBNET.V4RouteTableId == self.RouteTblId
        if lmapping.AddrFamily == 'IPV6':
            return lmapping.AddrFamily == self.AddrFamily and\
               lmapping.VNIC.SUBNET.V6RouteTableId == self.RouteTblId


    def SetupTestcaseConfig(self, obj):
        print(self.TestDestination)
        lobjs = list(filter(lambda x: self.__is_lmapping_match(x), 
                            lmapping.client.Objects()))
        obj.local_mapping = lobjs[0]
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
        self.__v4objs[vpcid] = []
        self.__v6objs[vpcid] = []
        self.__v4iter[vpcid] = None
        self.__v6iter[vpcid] = None

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

        def __get_overlap(base, count):
            # Keeping this function in case need to have different logic
            return  __get_adjacent_routes(base, count)

        def __get_first_subnet(ip, prefixlen):
            for ip in ip.subnets(new_prefix=prefixlen):
                return (ip)
            return

        def __is_v4stack(stack):
            if stack == "dual" or stack == 'ipv4':
                return True
            return False

        def __is_v6stack(stack):
            if stack == "dual" or stack == 'ipv6':
                return True
            return False

        if resmgr.RemoteMplsInternetTunAllocator == None:
            logger.info("Skipping route creation as there are no Internet tunnels")
            return

        routes = []
        for route_spec_obj in vpc_spec_obj.routetbl:
            stack = parent.Stack
            count = route_spec_obj.count
            if route_spec_obj.type == 'adjacent':
                if __is_v4stack(stack):
                    v4base = __get_first_subnet(ipaddress.ip_network(route_spec_obj.v4base.replace('\\', '/')), route_spec_obj.v4prefixlen)
                    routes += __get_adjacent_routes(v4base, count)
                if __is_v6stack(stack):
                    v6base = __get_first_subnet(ipaddress.ip_network(route_spec_obj.v6base.replace('\\','/')), route_spec_obj.v6prefixlen)
                    routes += __get_adjacent_routes(v6base, count)
            if route_spec_obj.type == 'overlap':
                if __is_v4stack(stack):
                    v4base = __get_first_subnet(ipaddress.ip_network(route_spec_obj.v4base.replace('\\','/')), route_spec_obj.v4prefixlen)
                    routes += __get_overlap(v4base, count)
                if __is_v6stack(stack):
                    v6base = __get_first_subnet(ipaddress.ip_network(route_spec_obj.v6base.replace('\\','/')), route_spec_obj.v6prefixlen)
                    routes += __get_overlap(v6base, count)

        tunobj = self.__internet_tunnel_get()
        for route in routes:
            obj = RouteObject(parent, route, tunobj)
            if route.version == 6:
                self.__v6objs[vpcid].append(obj)
            if route.version == 4:
                self.__v4objs[vpcid].append(obj)
            self.__objs.append(obj)

        if len(self.__v6objs[vpcid]) != 0:
            self.__v6iter[vpcid] = utils.rrobiniter(self.__v6objs[vpcid])

        if len(self.__v4objs[vpcid]) != 0:
            self.__v4iter[vpcid] = utils.rrobiniter(self.__v4objs[vpcid])

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.ROUTE, msgs)
        return

client = RouteObjectClient()

def GetMatchingObjects(selectors):
    mobjs = list(filter(lambda x: x.IsFilterMatch(selectors.route.filters), client.Objects()))
    return mobjs
