#! /usr/bin/python3
import pdb
from collections import defaultdict

from infra.common.logging import logger

from apollo.config.store import client as EzAccessStoreClient

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.objects.base as base
import apollo.config.utils as utils
import apollo.config.topo as topo
import ipaddress

import service_pb2 as service_pb2
import types_pb2 as types_pb2

class LocalMappingObject(base.ConfigObjectBase):
    def __init__(self, node, parent, spec, ipversion, count):
        super().__init__(api.ObjectTypes.LMAPPING, node)
        parent.AddChild(self)
        if hasattr(spec, 'origin'):
            self.SetOrigin(spec.origin)
        elif (EzAccessStoreClient[node].IsDeviceLearningEnabled()):
            self.SetOrigin('discovered')

        self.__is_public = getattr(spec, 'public', False)
        ################# PUBLIC ATTRIBUTES OF LOCAL MAPPING OBJECT ###########
        if (hasattr(spec, 'id')):
            self.MappingId = spec.id
        else:
            self.MappingId = next(ResmgrClient[node].LocalMappingIdAllocator)
        self.GID('LocalMapping%d'%self.MappingId)
        self.UUID = utils.PdsUuid(self.MappingId, self.ObjType)
        self.VNIC = parent
        public_ip = getattr(spec, 'publicip', None)
        if public_ip:
            self.PublicIPAddr = ipaddress.IPv4Address(public_ip)
        else:
            self.PublicIPAddr = None

        self.SourceGuard = parent.SourceGuard
        self.HasDefaultRoute = False
        if ipversion == utils.IP_VERSION_6:
            self.AddrFamily = 'IPV6'
            self.IPAddr = parent.SUBNET.AllocIPv6Address();
            if not self.PublicIPAddr and self.__is_public:
                self.PublicIPAddr = next(ResmgrClient[node].PublicIpv6AddressAllocator)
            if parent.SUBNET.V6RouteTable:
                self.HasDefaultRoute = parent.SUBNET.V6RouteTable.HasDefaultRoute
            self.SvcIPAddr, self.SvcPort = EzAccessStoreClient[node].GetSvcMapping(utils.IP_VERSION_6)
        else:
            self.AddrFamily = 'IPV4'
            if getattr(spec, 'lipaddr', None) != None:
                logger.info("LocalMapping Object assigned IP address:%s" % spec.lipaddr)
                self.IPAddr = ipaddress.IPv4Address(spec.lipaddr)
            else:
                self.IPAddr = parent.SUBNET.AllocIPv4Address()
                logger.info("LocalMapping Object generated IP address:%s" %(str(self.IPAddr)))
            if not self.PublicIPAddr and self.__is_public:
                self.PublicIPAddr = next(ResmgrClient[node].PublicIpAddressAllocator)
            if parent.SUBNET.V4RouteTable:
                self.HasDefaultRoute = parent.SUBNET.V4RouteTable.HasDefaultRoute
            self.SvcIPAddr, self.SvcPort = EzAccessStoreClient[node].GetSvcMapping(utils.IP_VERSION_4)
        self.Label = 'NETWORKING'
        self.FlType = "MAPPING"
        self.IP = str(self.IPAddr) # for testspec
        # Provider IP can be v4 or v6
        self.ProviderIPAddr, self.TunFamily = EzAccessStoreClient[node].GetProviderIPAddr(count)
        self.ProviderIP = str(self.ProviderIPAddr) # for testspec
        if self.PublicIPAddr is not None:
            self.PublicIP = str(self.PublicIPAddr) # for testspec
        self.SvcIP = str(self.SvcIPAddr) # for testspec
        # We will differentiate the traffic by port (vnet vs internet) and
        # different rules will be applied
        self.AppPort = ResmgrClient[node].TransportSrcPort
        self.LBPort = ResmgrClient[node].TransportSrcLBPort
        self.UnderlayVPCId = EzAccessStoreClient[node].GetUnderlayVPCId()

        ################# PRIVATE ATTRIBUTES OF MAPPING OBJECT #####################
        self.DeriveOperInfo()
        self.Show()
        return

    def __repr__(self):
        return "LocalMapping: %s |Vnic: %s |Subnet: %s |VPC: %s |Origin:%s" %\
               (self.UUID, self.VNIC.UUID, self.VNIC.SUBNET.UUID, self.VNIC.SUBNET.VPC.UUID, self.Origin)

    def Show(self):
        logger.info("LocalMapping Object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- IPAddr:%s|PublicIP:%s|PIP:%s|VIP:%s" \
            %(str(self.IPAddr), str(self.PublicIPAddr), str(self.ProviderIPAddr), str(self.SvcIPAddr)))
        return

    def IsFilterMatch(self, selectors):
        return super().IsFilterMatch(selectors.flow.filters)

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.IPKey.VPCId = self.VNIC.SUBNET.VPC.GetKey()
        utils.GetRpcIPAddr(self.IPAddr, spec.IPKey.IPAddr)
        spec.SubnetId = self.VNIC.SUBNET.GetKey()
        spec.VnicId = self.VNIC.GetKey()
        spec.MACAddr = self.VNIC.MACAddr.getnum()
        utils.GetRpcEncap(self.Node, self.VNIC.MplsSlot, self.VNIC.Vnid, spec.Encap)
        spec.PublicIP.Af = types_pb2.IP_AF_NONE
        if self.PublicIPAddr is not None:
            utils.GetRpcIPAddr(self.PublicIPAddr, spec.PublicIP)
        if utils.IsPipelineArtemis():
            utils.GetRpcIPAddr(self.ProviderIPAddr, spec.ProviderIp)
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.IPKey.VPCId != self.VNIC.SUBNET.VPC.GetKey():
            return False
        if not utils.ValidateRpcIPAddr(self.IPAddr, spec.IPKey.IPAddr):
            return False
        return True

    def GetGrpcSvcMappingCreateMessage(self, cookie):
        grpcmsg = service_pb2.SvcMappingRequest()
        grpcmsg.BatchCtxt.BatchCookie = cookie
        spec = grpcmsg.Request.add()
        svc_uuid = utils.PdsUuid(self.MappingId, api.ObjectTypes.SVCMAPPING)
        spec.Id = svc_uuid.GetUuid()
        spec.Key.VPCId = self.VNIC.SUBNET.VPC.GetKey()
        utils.GetRpcIPAddr(self.IPAddr, spec.Key.BackendIP)
        spec.Key.BackendPort = self.LBPort
        utils.GetRpcIPAddr(self.SvcIPAddr, spec.IPAddr)
        spec.SvcPort = self.SvcPort
        utils.GetRpcIPAddr(self.ProviderIPAddr, spec.ProviderIP)
        return grpcmsg

    def GetGrpcSvcMappingReadMessage(self):
        svc_uuid = utils.PdsUuid(self.MappingId, api.ObjectTypes.SVCMAPPING)
        grpcmsg = service_pb2.SvcMappingGetRequest()
        grpcmsg.Id.append(svc_uuid.GetUuid())
        return grpcmsg

    def Destroy(self):
        if not super().Destroy():
            return True
        if not client.RemoveObjFromCache(self):
            return False
        return True

class LocalMappingObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.LMAPPING, Resmgr.MAX_LMAPPING)
        self.__epip_objs = defaultdict(dict)
        return

    def GetLocalMapObjByEpIpKey(self, node, ip, vpcid):
        return self.__epip_objs[node].get((ip, vpcid), None)

    def IsReadSupported(self):
        if utils.IsPipelineApulu():
            return True
        # TODO: reads are failing for apollo & artemis
        return False

    def VerifyLearntIpEntries(self, node, ret, cli_op):
        if utils.IsDryRun(): return True
        if not ret:
            logger.error("pdsctl show learn ip cmd failed")
            return False
        # split output per object
        ip_entries = cli_op.split("---")
        for ip in ip_entries:
            yamlOp = utils.LoadYaml(ip)
            if not yamlOp:
                continue
            ip = yamlOp['key']['ipaddr']['v4orv6']['v4addr']
            ip_str = utils.Int2IPAddrStr(ip)
            vpc_uuid = utils.GetYamlSpecAttr(yamlOp['key'], 'vpcid')
            vpc_uuid_str = utils.List2UuidStr(vpc_uuid)
            subnet_uuid = utils.GetYamlSpecAttr(yamlOp['macinfo'], 'subnetid')
            mac_str = utils.Int2MacStr(yamlOp['macinfo']['macaddr'])

            # verifying if the info learnt is expected from config
            lmap_obj = self.GetLocalMapObjByEpIpKey(node, ip_str, vpc_uuid)
            if lmap_obj == None:
                logger.error(f"lmap not found in client object store for key {vpc_uuid_str}, {ip_str}")
                return False

            # verifying if the EP is from local mapping.
            args = " | grep " + vpc_uuid_str + " | grep " + ip_str + " | grep " + mac_str
            ret, op = utils.RunPdsctlShowCmd(node, "mapping internal local", args, False)
            if not ret:
                logger.error(f"show mapping internal local failed for VPC id {vpc_uuid_str}, IP {ip_str}")
                return False

            cmdop = op.split('\n')
            if not len(cmdop):
                logger.error("No (%s) entries found in lmap show VPC id %s, IP %s "%(
                             len(cmdop), vpc_uuid_str, ip_str))
                return False

            logger.info("Found (%s) LMAP entry for learn IP:%s, VPC:%s MAC:%s, Subnet:%s "%(
                    len(cmdop), ip_str, vpc_uuid_str, mac_str, utils.List2UuidStr(subnet_uuid)))
        return True

    def ValidateLearnIPInfo(self, node):
        if not EzAccessStoreClient[node].IsDeviceLearningEnabled():
            return True
        logger.info(f"Reading LMAP & learn ip objects from {node} ")
        # verify learn db against store
        ret, cli_op = utils.RunPdsctlShowCmd(node, "learn ip", None)
        if not self.VerifyLearntIpEntries(node, ret, cli_op):
            logger.error(f"learn ip object validation failed {ret} for {node} {cli_op}")
            return False
        return True

    def GenerateObjects(self, node, parent, vnic_spec_obj):
        isV4Stack = utils.IsV4Stack(parent.SUBNET.VPC.Stack)
        isV6Stack = utils.IsV6Stack(parent.SUBNET.VPC.Stack)
        c = 0
        v6c = 0
        v4c = 0
        hasLocalMap = hasattr(vnic_spec_obj, 'lmap')
        if hasLocalMap: #iota case
            lmap_spec = vnic_spec_obj.lmap[0]
            lmap_count = len(vnic_spec_obj.lmap)
        else: #Dol case
            lmap_spec = vnic_spec_obj
            lmap_count = vnic_spec_obj.ipcount
        while c < lmap_count:
            if isV6Stack:
                obj = LocalMappingObject(node, parent, lmap_spec, utils.IP_VERSION_6, v6c)
                self.Objs[node].update({obj.MappingId: obj})
                self.__epip_objs[node].update({(obj.IP, obj.VNIC.SUBNET.VPC.UUID.GetUuid()): obj})

                c = c + 1
                if c < lmap_count and hasLocalMap:
                    lmap_spec = vnic_spec_obj.lmap[c]
                else:
                    v6c = v6c + 1
            if c < lmap_count and isV4Stack:
                obj = LocalMappingObject(node, parent, lmap_spec, utils.IP_VERSION_4, v4c)
                self.Objs[node].update({obj.MappingId: obj})
                self.__epip_objs[node].update({(obj.IP, obj.VNIC.SUBNET.VPC.UUID.GetUuid()): obj})

                c = c + 1
                if c < lmap_count and hasLocalMap:
                    lmap_spec = vnic_spec_obj.lmap[c]
                else: 
                    v4c = v4c + 1
        return

    def CreateObjects(self, node):
        # TODO: Add check to see if 32 IPs per vnic
        super().CreateObjects(node)

        if utils.IsServiceMappingSupported():
            cookie = utils.GetBatchCookie(node)
            msgs = list(map(lambda x: x.GetGrpcSvcMappingCreateMessage(cookie), self.Objects(node)))
            api.client[node].Create(api.ObjectTypes.SVCMAPPING, msgs)
            list(map(lambda x: x.SetHwHabitant(True), self.Objects(node)))
        return True

    def ReadObjects(self, node):
        # read all not supported for local mapping - so do one by one
        cfgObjects = self.Objects(node)
        logger.info(f"Reading {len(cfgObjects)} {self.ObjType.name} Objects in {node}")
        result = list(map(lambda x: x.Read(), cfgObjects))
        if not all(result):
            logger.info(f"Reading {len(cfgObjects)} {self.ObjType.name} Objects FAILED in {node}")
            return False

        if utils.IsServiceMappingSupported():
            logger.info(f"Reading {len(self.Objects(node))} SVC {self.ObjType.name} Objects in {node}")
            msgs = list(map(lambda x: x.GetGrpcSvcMappingReadMessage(), self.Objects(node)))
            api.client[node].Get(api.ObjectTypes.SVCMAPPING, msgs)
        return True

    def GetVnicAddresses(self, vnic):
        ipv4_addresses = []
        v4pfxlen = str(vnic.SUBNET.IPPrefix[1].prefixlen)
        for mapping in self.Objects(vnic.Node):
            if mapping.IsV6() or (mapping.VNIC.GID() != vnic.GID()):
                continue
            ipv4_addresses.append(mapping.IP + "/" + v4pfxlen)
        return ipv4_addresses

    def ChangeNode(self, lmap, target_node):
        logger.info(f"Changing node for {lmap}  {lmap.Node} => {target_node}")
        del self.Objs[lmap.Node][lmap.MappingId]
        del self.__epip_objs[lmap.Node][(lmap.IP, lmap.VNIC.SUBNET.VPC.UUID.GetUuid())]
        lmap.Node = target_node
        self.Objs[target_node].update({lmap.MappingId: lmap})
        self.__epip_objs[target_node].update({(lmap.IP, lmap.VNIC.SUBNET.VPC.UUID.GetUuid()): lmap})

    def RemoveObjFromCache(self, lmap):
        logger.info(f"Deleting {lmap} from EP IP object cache")
        del self.__epip_objs[lmap.Node][(lmap.IP, lmap.VNIC.SUBNET.VPC.UUID.GetUuid())]
        return super().RemoveObjFromCache(lmap)

    def GetVnicPublicAddresses(self, vnic):
        ipv4_addresses = []
        v4pfxlen = str(vnic.SUBNET.IPPrefix[1].prefixlen)
        for mapping in self.Objects(vnic.Node):
            if mapping.IsV6() or (mapping.VNIC.GID() != vnic.GID()):
                continue
            if not mapping.PublicIP:
                continue
            ipv4_addresses.append(mapping.PublicIP)
        return ipv4_addresses

client = LocalMappingObjectClient()

def GetMatchingObjects(selectors, node):
    objs = []
    for obj in client.Objects(node):
        if obj.IsFilterMatch(selectors):
            objs.append(obj)
    return objs
