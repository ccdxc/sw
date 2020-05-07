#! /usr/bin/python3
import pdb
import json
import requests
import ipaddress

from infra.common.logging import logger
from infra.common.glopts  import GlobalOptions
import infra.common.objects as objects

from apollo.config.store import client as EzAccessStoreClient

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr
from apollo.config.objects.dhcprelay import client as DHCPRelayClient
from apollo.config.objects.dhcpproxy import client as DHCPProxyClient
from apollo.config.objects.nexthop import client as NhClient
from apollo.config.objects.nexthop_group import client as NhGroupClient
from apollo.config.objects.interface import client as InterfaceClient
from apollo.config.objects.port import client as PortClient
from apollo.config.objects.metaswitch.bgp import client as BGPClient
from apollo.config.objects.metaswitch.bgp_peer import client as BGPPeerClient
from apollo.config.objects.metaswitch.bgp_peeraf import client as BGPPeerAfClient

import apollo.config.agent.api as api
import apollo.config.objects.base as base
import apollo.config.objects.policy as policy
import apollo.config.objects.policer as policer
import apollo.config.objects.route as route
import apollo.config.objects.subnet as subnet
import apollo.config.objects.tunnel as tunnel
import apollo.config.objects.nexthop_group as nexthop_group
import apollo.config.objects.tag as tag
import apollo.config.objects.meter as meter
import artemis.config.objects.cfgjson as cfgjson
import apollo.config.utils as utils
import apollo.config.objects.nat_pb as nat_pb
import apollo.config.objects.metaswitch.evpnipvrf as evpnipvrf
import apollo.config.objects.metaswitch.evpnipvrfrt as evpnipvrfrt

import vpc_pb2 as vpc_pb2

class VpcStatus(base.StatusObjectBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.VPC)

class VpcObject(base.ConfigObjectBase):
    def __init__(self, node, spec, index, maxcount):
        super().__init__(api.ObjectTypes.VPC, node)
        if hasattr(spec, 'origin'):
            self.SetOrigin(spec.origin)
        ################# PUBLIC ATTRIBUTES OF VPC OBJECT #####################
        if (hasattr(spec, 'id')):
            self.VPCId = spec.id
        else:
            self.VPCId = next(ResmgrClient[node].VpcIdAllocator)
        self.GID('Vpc%d'%self.VPCId)
        self.UUID = utils.PdsUuid(self.VPCId, self.ObjType)
        self.IPPrefix = {}
        self.Nat46_pfx = None
        self.V4RouteTableId = 0
        self.V6RouteTableId = 0
        self.V4RouteTableName = ""
        if spec.type == 'underlay':
            self.Type = vpc_pb2.VPC_TYPE_UNDERLAY
            self.IPPrefix[0] = ResmgrClient[node].ProviderIpV6Network
            self.IPPrefix[1] = ResmgrClient[node].ProviderIpV4Network
            # Reserve one SVC port
            # Right now it does not support multiple backends for a frontend
            self.SvcPort = ResmgrClient[node].TransportSvcPort
            self.__max_svc_mapping_shared_count = 1
            self.__svc_mapping_shared_count = 0
            self.SvcMappingIPAddr  = {}
        elif spec.type == 'control':
            self.Type = vpc_pb2.VPC_TYPE_CONTROL
            self.IPPrefix[0] = ResmgrClient[node].GetVpcIPv6Prefix(self.VPCId)
            if hasattr(spec, "v4prefix"):
                self.IPPrefix[1] = ipaddress.ip_network(spec.v4prefix.replace('\\', '/'))
            else:
                self.IPPrefix[1] = ResmgrClient[node].GetVpcIPv4Prefix(self.VPCId)
        else:
            self.Type = vpc_pb2.VPC_TYPE_TENANT
            self.IPPrefix[0] = ResmgrClient[node].GetVpcIPv6Prefix(self.VPCId)
            if hasattr(spec, "v4prefix"):
                self.IPPrefix[1] = ipaddress.ip_network(spec.v4prefix.replace('\\', '/'))
            else:
                self.IPPrefix[1] = ResmgrClient[node].GetVpcIPv4Prefix(self.VPCId)
        if (hasattr(spec, 'nat46')) and spec.nat46 is True:
            self.Nat46_pfx = ResmgrClient[node].Nat46Address
        self.Stack = spec.stack
        # As currently vpc can have only type IPV4 or IPV6, we will alternate
        # the configuration
        if self.Stack == 'dual':
            self.PfxSel = index % 2
        elif self.Stack == 'ipv4':
            self.PfxSel = 1
        else:
            self.PfxSel = 0
        if getattr(spec, 'fabricencap', None) != None:
            self.FabricEncap = utils.GetEncapType(spec.fabricencap)
        if getattr(spec, 'fabricencapvalue', None) != None:
            self.Vnid = spec.fabricencapvalue
        else:
            self.Vnid = next(ResmgrClient[node].VxlanIdAllocator)

        self.VirtualRouterMACAddr = ResmgrClient[node].VirtualRouterMacAllocator.get()
        self.Tos = 0 # to start with
        self.Mutable = utils.IsUpdateSupported()
        self.Status = VpcStatus()
        self.UpdateImplicit()
        ################# PRIVATE ATTRIBUTES OF VPC OBJECT #####################
        self.__ip_subnet_prefix_pool = {}
        self.__ip_subnet_prefix_pool[0] = {}
        self.__ip_subnet_prefix_pool[1] = {}
        self.Show()

        ############### CHILDREN OBJECT GENERATION

        # Generate Port Configuration
        #PortClient.GenerateObjects(node, self, spec)

        # Generate Interface Configuration
        InterfaceClient.GenerateObjects(node, self, spec)

        if utils.IsPipelineApollo() and self.Type == vpc_pb2.VPC_TYPE_UNDERLAY:
            # Nothing to be done for underlay vpc
            return

        # Generate DHCP configuration
        if self.IsUnderlayVPC():
            DHCPRelayClient.GenerateObjects(node, self.VPCId, spec)

        # Generate NextHop configuration
        NhClient.GenerateObjects(node, self, spec)

        # Generate NextHopGroup configuration
        NhGroupClient.GenerateObjects(node, self, spec)

        # Generate Tunnel configuration
        if hasattr(spec, "tunnel"):
            tunnel.client.GenerateObjects(node, EzAccessStoreClient[node].GetDevice(), spec.tunnel)

        # Generate Tag configuration.
        if hasattr(spec, 'tagtbl'):
            tag.client.GenerateObjects(node, self, spec)

        # Generate Policy configuration.
        if hasattr(spec, 'policy'):
            policy.client.GenerateObjects(node, self, spec)

        # Generate DHCP Policy configuration.
        if hasattr(spec, 'dhcppolicy'):
           DHCPProxyClient.GenerateObjects(node, self, spec)

        # Generate Route configuration.
        if hasattr(spec, 'routetbl'):
            # find peer vpcid
            if (index + 1) == maxcount:
                vpc_peerid = self.VPCId - maxcount + 1
            else:
                vpc_peerid = self.VPCId + 1
            # For underlay-vpc, the route-table is implicitly created, so our
            # routetable object create is not needed (will fail with exists error).
            if spec.type != 'underlay':
                route.client.GenerateObjects(node, self, spec, vpc_peerid)

        # Generate Meter configuration
        meter.client.GenerateObjects(node, self, spec)

        # Generate Metaswitch configuration
        evpnipvrf.client.GenerateObjects(node, self, spec)
        evpnipvrfrt.client.GenerateObjects(node, self, spec)

        self.V4RouteTableId = route.client.GetRouteV4TableId(node, self.VPCId)
        self.V6RouteTableId = route.client.GetRouteV6TableId(node, self.VPCId)
        self.V4RouteTable = route.client.GetRouteV4Table(node, self.VPCId, self.V4RouteTableId)
        self.V6RouteTable = route.client.GetRouteV6Table(node, self.VPCId, self.V6RouteTableId)
        # Generate Subnet configuration post policy & route
        if hasattr(spec, 'subnet'):
            subnet.client.GenerateObjects(node, self, spec)
        self.DeriveOperInfo()

        # Generate NAT Port Block configuration
        if hasattr(spec, 'nat'):
            self.__nat_pool = {}
            prefix_len = getattr(spec.nat, 'prefixlen', 32)
            self.__nat_pool[utils.NAT_ADDR_TYPE_PUBLIC] = \
                iter(Resmgr.GetVpcInternetNatPoolPfx(self.VPCId).subnets(new_prefix=prefix_len))
            self.__nat_pool[utils.NAT_ADDR_TYPE_SERVICE] = \
                iter(Resmgr.GetVpcInfraNatPoolPfx(self.VPCId).subnets(new_prefix=prefix_len))
            nat_pb.client.GenerateObjects(node, self, spec)

        # Generate BGP configuration
        BGPClient.GenerateObjects(node, self, spec)
        BGPPeerClient.GenerateObjects(node, self, spec)
        return

    def __repr__(self):
        return "Vpc: %s |Type:%d|PfxSel:%d" %\
               (self.UUID, self.Type, self.PfxSel)

    def Show(self):
        logger.info("VPC Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Prefix:%s" % self.IPPrefix)
        logger.info("- Vnid:%s|VRMac:%s" %\
                    (self.Vnid, self.VirtualRouterMACAddr))
        logger.info(f"- RouteTableIds V4:{self.V4RouteTableId}|V6:{self.V6RouteTableId}")
        self.Status.Show()
        return

    def UpdateImplicit(self):
        if (GlobalOptions.dryrun):
            return
        if (not self.IsOriginImplicitlyCreated()):
            return
        # We need to read info from naples and update the DS
        resp = api.client[self.Node].GetHttp(self.ObjType)
        for vpcinst in resp:
            if (not (vpcinst['spec']['vrf-type'] == 'INFRA')):
                continue
            uuid_str = vpcinst['meta']['uuid']
            self.UUID = utils.PdsUuid(bytes.fromhex(uuid_str.replace('-','')),\
                    self.ObjType)
            ms = getattr(vpcinst['spec'], 'router-mac', '00:00:00:00:00:00')
            self.VirtualRouterMACAddr = objects.MacAddressBase(string=ms)
            self.Vnid = int(getattr(vpcinst['spec'], 'vxlan-vni', '0'))
            self.Tenant = vpcinst['meta']['tenant']
            self.Namespace = vpcinst['meta']['namespace']
            self.GID(vpcinst['meta']['name'])
        return

    def InitSubnetPefixPools(self, poolid, v6pfxlen, v4pfxlen):
        if v6pfxlen:
            self.__ip_subnet_prefix_pool[0][poolid] =  Resmgr.CreateIPv6SubnetPool(self.IPPrefix[0], v6pfxlen, poolid)
        self.__ip_subnet_prefix_pool[1][poolid] =  Resmgr.CreateIPv4SubnetPool(self.IPPrefix[1], v4pfxlen, poolid)

    def AllocIPv6SubnetPrefix(self, poolid):
        return next(self.__ip_subnet_prefix_pool[0][poolid])

    def AllocIPv4SubnetPrefix(self, poolid):
        return next(self.__ip_subnet_prefix_pool[1][poolid])

    def AllocNatPrefix(self, nat_type, sublen=1):
        return next(self.__nat_pool[nat_type])

    def GetProviderIPAddr(self, count):
        assert self.Type == vpc_pb2.VPC_TYPE_UNDERLAY
        if self.Stack == 'dual':
            paf = utils.IP_VERSION_6 if count % 2 == 0 else utils.IP_VERSION_4
        else:
            paf = utils.IP_VERSION_6 if self.Stack == 'ipv6' else utils.IP_VERSION_4
        if paf == utils.IP_VERSION_6:
            return next(ResmgrClient[self.Node].ProviderIpV6AddressAllocator), 'IPV6'
        else:
            return next(ResmgrClient[self.Node].ProviderIpV4AddressAllocator), 'IPV4'

    def GetSvcMapping(self, ipversion):
        assert self.Type == vpc_pb2.VPC_TYPE_UNDERLAY

        def __alloc():
            self.SvcMappingIPAddr[0] = next(ResmgrClient[self.Node].SvcMappingPublicIpV6AddressAllocator)
            self.SvcMappingIPAddr[1] = next(ResmgrClient[self.Node].SvcMappingPublicIpV4AddressAllocator)

        def __get():
            if ipversion ==  utils.IP_VERSION_6:
                return self.SvcMappingIPAddr[0],self.SvcPort
            else:
                return self.SvcMappingIPAddr[1],self.SvcPort

        if self.__svc_mapping_shared_count == 0:
            __alloc()
            self.__svc_mapping_shared_count = (self.__svc_mapping_shared_count + 1) % self.__max_svc_mapping_shared_count
        return __get()

    def UpdateAttributes(self, spec):
        self.VirtualRouterMACAddr = ResmgrClient[self.Node].VirtualRouterMacAllocator.get()
        self.Tos += 1

    def RollbackAttributes(self):
        attrlist = ["VirtualRouterMACAddr", "Tos", "Vnid"]
        self.RollbackMany(attrlist)

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.Type = self.Type
        spec.V4RouteTableId = utils.PdsUuid.GetUUIDfromId(self.V4RouteTableId, api.ObjectTypes.ROUTE)
        spec.V6RouteTableId = utils.PdsUuid.GetUUIDfromId(self.V6RouteTableId, api.ObjectTypes.ROUTE)
        spec.VirtualRouterMac = self.VirtualRouterMACAddr.getnum()
        spec.ToS = self.Tos
        utils.GetRpcEncap(self.Node, self.Vnid, self.Vnid, spec.FabricEncap)
        if self.Nat46_pfx is not None:
            utils.GetRpcIPv6Prefix(self.Nat46_pfx, spec.Nat46Prefix)
        return

    def PopulateAgentJson(self):
        if self.Type == vpc_pb2.VPC_TYPE_UNDERLAY:
            spec = {
                "kind": "Vrf",
                "meta": {
                    "name": self.GID(),
                    "namespace": self.Namespace,
                    "tenant": self.Tenant,
                    "uuid" : self.UUID.UuidStr,
                },
                "spec": {
                    "vrf-type": "INFRA",
                }
            }
        else:
            spec = {
                "kind": "Vrf",
                "meta": {
                    "name": self.GID(),
                    "namespace": self.Namespace,
                    "tenant": self.Tenant,
                    "uuid" : self.UUID.UuidStr,
                    "labels": {
                        "CreatedBy": "Venice"
                    },
                },
                "spec": {
                    "vrf-type": "CUSTOMER",
                    "v4-route-table": self.V4RouteTableName,
                    "router-mac": str(self.VirtualRouterMACAddr),
                    "vxlan-vni": self.Vnid,
                    "route-import-export": {
                        "address-family": "evpn",
                        "rd-auto": True,
                        "rt-export": [
                            {
                                "type": "type0",
                                "admin-value": 100,
                                "assigned-value": 2001
                            }
                        ],
                        "rt-import": [
                            {
                                "type": "type0",
                                "admin-value": 100,
                                "assigned-value": 2001
                            }
                        ]
                    }
                }
            }
        return json.dumps(spec)

    def ValidateJSONSpec(self, spec):
        if spec['kind'] != 'Vrf': return False
        if spec['meta']['name'] != self.GID(): return False
        if self.Type == vpc_pb2.VPC_TYPE_UNDERLAY:
            if spec['spec']['vrf-type'] != 'INFRA': return False
        else:
            if spec['spec']['vrf-type'] != 'CUSTOMER': return False
            if spec['spec']['vxlan-vni'] != self.Vnid: return False
            if spec['spec']['router-mac'] != str(self.VirtualRouterMACAddr):
                return False
            if spec['spec']['v4-route-table'] != self.V4RouteTableName:
                return False
        return True

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.Type != self.Type:
            return False
        if utils.ValidateTunnelEncap(self.Node, self.Vnid, spec.FabricEncap) is False:
            return False
        if spec.ToS != self.Tos:
            return False
        if utils.IsPipelineApulu():
            if spec.VirtualRouterMac != self.VirtualRouterMACAddr.getnum():
                return False
        return True

    def ValidateYamlSpec(self, spec):
        if utils.GetYamlSpecAttr(spec) != self.GetKey():
            return False
        if spec['type'] != self.Type:
            return False
        return True

    def IsUnderlayVPC(self):
        if self.Type == vpc_pb2.VPC_TYPE_UNDERLAY:
            return True
        return False

    def IsV6Stack(self):
        return utils.IsV6Stack(self.Stack)

    def GetDependees(self, node):
        """
        depender/dependent - vpc
        dependee - routetable
        """
        dependees = [ self.V4RouteTable, self.V6RouteTable ]
        return dependees

    def RestoreNotify(self, cObj):
        logger.info("Notify %s for %s creation" % (self, cObj))
        if not self.IsHwHabitant():
            logger.info(" - Skipping notification as %s already deleted" % self)
            return
        logger.info(" - Linking %s to %s " % (cObj, self))
        if cObj.ObjType == api.ObjectTypes.ROUTE:
            if cObj.IsV4():
                self.V4RouteTableId = cObj.RouteTblId
            elif cObj.IsV6():
                self.V6RouteTableId = cObj.RouteTblId
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
        if dObj.ObjType == api.ObjectTypes.ROUTE:
            if self.V4RouteTableId == dObj.RouteTblId:
                self.V4RouteTableId = 0
            elif self.V6RouteTableId == dObj.RouteTblId:
                self.V6RouteTableId = 0
            else:
                logger.error(" - ERROR: %s not associated with %s" % \
                             (dObj, self))
                assert(0)
        else:
            logger.error(" - ERROR: %s not handling %s deletion" %\
                         (self.ObjType.name, dObj.ObjType))
            assert(0)
        self.SetDirty(True)
        self.CommitUpdate()
        return


class VpcObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.VPC, Resmgr.MAX_VPC)
        return

    # TODO: move to GetObjectByKey
    def GetVpcObject(self, node, vpcid):
        return self.GetObjectByKey(node, vpcid)

    def IsReadSupported(self):
        if utils.IsNetAgentMode():
            # TODO: Fix validation & remove this
            return False
        return True

    def __write_cfg(self, vpc_count):
        nh = NhClient.GetNumNextHopPerVPC()
        mtr = meter.client.GetNumMeterPerVPC()
        CfgJsonHelper = cfgjson.CfgJsonObjectHelper()
        CfgJsonHelper.SetNumNexthopPerVPC(nh)
        CfgJsonHelper.SetNumMeterPerVPC(mtr[0], mtr[1])
        CfgJsonHelper.SetVPCCount(vpc_count)
        CfgJsonHelper.WriteConfig()

    def GenerateObjects(self, node, topospec):
        vpc_count = 0
        for p in topospec.vpc:
            vpc_count += p.count
            for c in range(p.count):
                if hasattr(p, "nat46"):
                    if p.nat46 is True and not utils.IsPipelineArtemis():
                        continue
                obj = VpcObject(node, p, c, p.count)
                self.Objs[node].update({obj.VPCId: obj})
                if obj.IsUnderlayVPC():
                    EzAccessStoreClient[node].SetUnderlayVPC(obj)
        # Write the flow and nexthop config to agent hook file
        if utils.IsFlowInstallationNeeded():
            self.__write_cfg(vpc_count)
        if utils.IsPipelineApulu():
            # Associate Nexthop objects
            NhGroupClient.CreateAllocator(node)
            NhClient.AssociateObjects(node)
            NhGroupClient.AssociateObjects(node)
            tunnel.client.FillUnderlayNhGroups(node)
            route.client.FillNhGroups(node)
        return

    def CreateObjects(self, node):
        if GlobalOptions.netagent:
            route.client.CreateObjects(node)

        # netagent requires route table before vpc
        super().CreateObjects(node)

        # Create DHCP Relay Objects
        DHCPRelayClient.CreateObjects(node)

        # Create DHCP Relay Objects
        DHCPProxyClient.CreateObjects(node)

        # Create Nexthop object
        NhClient.CreateObjects(node)

        # Create Tag object.
        tag.client.CreateObjects(node)

        # Create Policy object.
        policy.client.CreateObjects(node)

        # Create Policer object.
        policer.client.CreateObjects(node)

        # Create Route object.
        if not GlobalOptions.netagent:
            route.client.CreateObjects(node)

        # Create Meter Objects
        meter.client.CreateObjects(node)

        # Create BGP Objects
        BGPClient.CreateObjects(node)
        BGPPeerClient.CreateObjects(node)
        BGPPeerAfClient.CreateObjects(node)

        # Create Metaswitch objects
        evpnipvrf.client.CreateObjects(node)
        evpnipvrfrt.client.CreateObjects(node)

        # Create Subnet Objects after policy & route
        subnet.client.CreateObjects(node)

        # Create NAT Port Block Objects
        nat_pb.client.CreateObjects(node)
        return

client = VpcObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()

