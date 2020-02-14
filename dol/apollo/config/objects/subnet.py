#! /usr/bin/python3
import pdb
import ipaddress
import itertools
import copy
from infra.common.logging import logger

from apollo.config.store import client as EzAccessStoreClient
from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr
from apollo.config.agent.api import ObjectTypes as ObjectTypes

import apollo.config.agent.api as api
import apollo.config.objects.base as base
from apollo.config.objects.interface import client as InterfaceClient
import apollo.config.objects.vnic as vnic
import apollo.config.objects.rmapping as rmapping
from apollo.config.objects.policy import client as PolicyClient
import apollo.config.objects.route as route
import apollo.config.utils as utils

import subnet_pb2 as subnet_pb2

class SubnetStatus(base.StatusObjectBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.SUBNET)

class SubnetObject(base.ConfigObjectBase):
    def __init__(self, node, parent, spec, poolid):
        super().__init__(api.ObjectTypes.SUBNET, node)
        parent.AddChild(self)
        ################# PUBLIC ATTRIBUTES OF SUBNET OBJECT #####################
        if (hasattr(spec, 'id')):
            self.SubnetId = spec.id
        else:
            self.SubnetId = next(ResmgrClient[node].SubnetIdAllocator)
        self.GID('Subnet%d'%self.SubnetId)
        self.UUID = utils.PdsUuid(self.SubnetId, self.ObjType)
        self.VPC = parent
        self.PfxSel = parent.PfxSel
        self.IPPrefix = {}
        if getattr(spec, 'v6prefix', None) != None or \
                getattr(spec, 'v6prefixlen', None) != None:
            self.IPPrefix[0] = parent.AllocIPv6SubnetPrefix(poolid)
            self.IpV6Valid = True
        else:
            self.IpV6Valid = False
        if getattr(spec, 'v4prefix', None) != None:
            self.IPPrefix[1] = ipaddress.ip_network(spec.v4prefix.replace('\\', '/'))
        else:
            self.IPPrefix[1] = parent.AllocIPv4SubnetPrefix(poolid)
        self.VirtualRouterIPAddr = {}
        self.VirtualRouterMacAddr = None
        self.V4RouteTableId = route.client.GetRouteV4TableId(node, parent.VPCId)
        self.V6RouteTableId = route.client.GetRouteV6TableId(node, parent.VPCId)
        self.IngV4SecurityPolicyIds = [PolicyClient.GetIngV4SecurityPolicyId(node, parent.VPCId)]
        self.IngV6SecurityPolicyIds = [PolicyClient.GetIngV6SecurityPolicyId(node, parent.VPCId)]
        self.EgV4SecurityPolicyIds = [PolicyClient.GetEgV4SecurityPolicyId(node, parent.VPCId)]
        self.EgV6SecurityPolicyIds = [PolicyClient.GetEgV6SecurityPolicyId(node, parent.VPCId)]
        self.V4RouteTable = route.client.GetRouteV4Table(node, parent.VPCId, self.V4RouteTableId)
        self.V6RouteTable = route.client.GetRouteV6Table(node, parent.VPCId, self.V6RouteTableId)
        if getattr(spec, 'fabricencap', None) != None:
            self.FabricEncap = utils.GetEncapType(spec.fabricencap)
        if getattr(spec, 'fabricencapvalue', None) != None:
            self.Vnid = spec.fabricencapvalue
        else:
            self.Vnid = next(ResmgrClient[node].VxlanIdAllocator)
        # TODO: clean this host if logic
        if utils.IsDol():
            self.HostIf = InterfaceClient.GetHostInterface(node)
            if self.HostIf:
                self.HostIfIdx = utils.LifId2LifIfIndex(self.HostIf.lif.id)
            else:
                self.HostIfIdx = getattr(parent, 'HostIfIdx', None)
            node_uuid = None
        else:
            self.HostIf = None
            hostifidx = getattr(spec, 'hostifidx', None)
            if hostifidx:
                self.HostIfIdx = int(hostifidx)
            else:
                self.HostIfIdx = next(ResmgrClient[node].HostIfIdxAllocator)
            node_uuid = EzAccessStoreClient[node].GetNodeUuid(node)
        self.HostIfUuid = utils.PdsUuid(self.HostIfIdx, ObjectTypes.INTERFACE, node_uuid) if self.HostIfIdx else None
        self.Status = SubnetStatus()
        ################# PRIVATE ATTRIBUTES OF SUBNET OBJECT #####################
        self.__ip_address_pool = {}
        if self.IpV6Valid:
            self.__ip_address_pool[0] = Resmgr.CreateIpv6AddrPool(self.IPPrefix[0])
        self.__ip_address_pool[1] = Resmgr.CreateIpv4AddrPool(self.IPPrefix[1])

        self.__set_vrouter_attributes()
        self.__fill_default_rules_in_policy(node)
        self.DeriveOperInfo()
        self.Mutable = utils.IsUpdateSupported()
        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate VNIC and Remote Mapping configuration
        vnic.client.GenerateObjects(node, self, spec)
        rmapping.client.GenerateObjects(node, self, spec)
        return

    def __repr__(self):
        return "Subnet: %s |VPC: %s |PfxSel:%d|MAC:%s" %\
               (self.UUID, self.VPC.UUID, self.PfxSel, self.VirtualRouterMACAddr.get())

    def Show(self):
        logger.info("SUBNET object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Prefix %s VNI %d" % (self.IPPrefix, self.Vnid))
        logger.info('- HostIfIdx:%s' % (self.HostIfIdx))
        logger.info('- HostIfUuid:%s' % (self.HostIfUuid))
        logger.info("- VirtualRouter IP:%s" % (self.VirtualRouterIPAddr))
        logger.info("- VRMac:%s" % (self.VirtualRouterMACAddr))
        logger.info("- TableIds V4:%d|V6:%d" % (self.V4RouteTableId, self.V6RouteTableId))
        logger.info("- NaclIDs IngV4:%s|IngV6:%s|EgV4:%s|EgV6:%s" %\
                    (self.IngV4SecurityPolicyIds, self.IngV6SecurityPolicyIds, self.EgV4SecurityPolicyIds, self.EgV6SecurityPolicyIds))
        hostif = self.HostIf
        if hostif:
            lif = hostif.lif
            lififindex = hex(utils.LifId2LifIfIndex(lif.id))
            logger.info("- HostInterface:%s|%s|%s" %\
                (hostif.Ifname, lif.GID(), lififindex))
        if self.HostIfUuid:
            logger.info("- HostIf:%s" % self.HostIfUuid)
        self.Status.Show()
        return

    def __fill_default_rules_in_policy(self, node):
        ids = itertools.chain(self.IngV4SecurityPolicyIds, self.EgV4SecurityPolicyIds, self.IngV6SecurityPolicyIds, self.EgV6SecurityPolicyIds)
        for policyid in ids:
            if policyid is 0:
                continue
            policyobj = PolicyClient.GetPolicyObject(node, policyid)
            if policyobj.PolicyType == 'default':
                #TODO: move this to policy.py
                self.__fill_default_rules(policyobj)
            else:
                PolicyClient.ModifyPolicyRules(node, policyid, self)
        return

    def __fill_default_rules(self, policyobj):
        rules = []
        pfx = None
        if policyobj.AddrFamily == 'IPV4':
            if policyobj.PolicyType == 'default':
                pfx = utils.IPV4_DEFAULT_ROUTE
            elif policyobj.PolicyType is 'subnet':
                pfx = ipaddress.ip_network(self.IPPrefix[1])
        else:
            if not self.IpV6Valid:
                return
            if policyobj.PolicyType == 'default':
                pfx = utils.IPV6_DEFAULT_ROUTE
            elif policyobj.PolicyType is 'subnet':
                pfx = ipaddress.ip_network(self.IPPrefix[0])
        policyobj.rules = PolicyClient.Generate_Allow_All_Rules(pfx, pfx)

    def __set_vrouter_attributes(self):
        # 1st IP address of the subnet becomes the vrouter.
        if self.IpV6Valid:
            self.VirtualRouterIPAddr[0] = next(self.__ip_address_pool[0])
        self.VirtualRouterIPAddr[1] = next(self.__ip_address_pool[1])
        self.VirtualRouterMACAddr = ResmgrClient[self.Node].VirtualRouterMacAllocator.get()
        return

    def AllocIPv6Address(self):
        return next(self.__ip_address_pool[0])

    def AllocIPv4Address(self):
        return next(self.__ip_address_pool[1])

    def UpdateAttributes(self):
        self.VirtualRouterMACAddr = ResmgrClient[self.Node].VirtualRouterMacAllocator.get()
        if utils.IsDol():
            hostIf = InterfaceClient.GetHostInterface(self.Node)
            if hostIf != None:
                self.HostIf = hostIf
                self.HostIfIdx = utils.LifId2LifIfIndex(self.HostIf.lif.id)
                self.HostIfUuid = utils.PdsUuid(self.HostIfIdx, ObjectTypes.INTERFACE, None) if self.HostIfIdx else None
        return

    def RollbackAttributes(self):
        attrlist = ["VirtualRouterMACAddr", "HostIf", "HostIfIdx", "HostIfUuid"]
        self.RollbackMany(attrlist)
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.VPCId = self.VPC.GetKey()
        utils.GetRpcIPv4Prefix(self.IPPrefix[1], spec.V4Prefix)
        if self.IpV6Valid:
            utils.GetRpcIPv6Prefix(self.IPPrefix[0], spec.V6Prefix)
        spec.IPv4VirtualRouterIP = int(self.VirtualRouterIPAddr[1])
        if self.IpV6Valid:
            spec.IPv6VirtualRouterIP = self.VirtualRouterIPAddr[0].packed
        spec.VirtualRouterMac = self.VirtualRouterMACAddr.getnum()
        spec.V4RouteTableId = utils.PdsUuid.GetUUIDfromId(self.V4RouteTableId, ObjectTypes.ROUTE)
        spec.V6RouteTableId = utils.PdsUuid.GetUUIDfromId(self.V6RouteTableId, ObjectTypes.ROUTE)
        for policyid in self.IngV4SecurityPolicyIds:
            spec.IngV4SecurityPolicyId.append(utils.PdsUuid.GetUUIDfromId(policyid, ObjectTypes.POLICY))
        for policyid in self.IngV6SecurityPolicyIds:
            spec.IngV6SecurityPolicyId.append(utils.PdsUuid.GetUUIDfromId(policyid, ObjectTypes.POLICY))
        for policyid in self.EgV4SecurityPolicyIds:
            spec.EgV4SecurityPolicyId.append(utils.PdsUuid.GetUUIDfromId(policyid, ObjectTypes.POLICY))
        for policyid in self.EgV6SecurityPolicyIds:
            spec.EgV6SecurityPolicyId.append(utils.PdsUuid.GetUUIDfromId(policyid, ObjectTypes.POLICY))
        utils.GetRpcEncap(self.Node, self.Vnid, self.Vnid, spec.FabricEncap)
        if utils.IsPipelineApulu():
            if self.HostIfUuid:
                spec.HostIf = self.HostIfUuid.GetUuid()
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.VPCId != self.VPC.GetKey():
            return False
        if spec.VirtualRouterMac != self.VirtualRouterMACAddr.getnum():
            return False
        if spec.V4RouteTableId != utils.PdsUuid.GetUUIDfromId(self.V4RouteTableId, ObjectTypes.ROUTE):
            return False
        if spec.V6RouteTableId != utils.PdsUuid.GetUUIDfromId(self.V6RouteTableId, ObjectTypes.ROUTE):
            return False
        if spec.IngV4SecurityPolicyId[0] != utils.PdsUuid.GetUUIDfromId(self.IngV4SecurityPolicyIds[0], ObjectTypes.POLICY):
            return False
        if spec.IngV6SecurityPolicyId[0] != utils.PdsUuid.GetUUIDfromId(self.IngV6SecurityPolicyIds[0], ObjectTypes.POLICY):
            return False
        if spec.EgV4SecurityPolicyId[0] != utils.PdsUuid.GetUUIDfromId(self.EgV4SecurityPolicyIds[0], ObjectTypes.POLICY):
            return False
        if spec.EgV6SecurityPolicyId[0] != utils.PdsUuid.GetUUIDfromId(self.EgV6SecurityPolicyIds[0], ObjectTypes.POLICY):
            return False
        if utils.ValidateTunnelEncap(self.Node, self.Vnid, spec.FabricEncap) is False:
            return False
        if utils.IsPipelineApulu():
            if self.HostIfUuid:
                if spec.HostIf != self.HostIfUuid.GetUuid():
                    return False
        return True

    def ValidateYamlSpec(self, spec):
        if  utils.GetYamlSpecAttr(spec, ObjectTypes.SUBNET, 'id') != self.GetKey():
            return False
        return True

    def GetNaclId(self, direction, af=utils.IP_VERSION_4):
        if af == utils.IP_VERSION_4:
            if direction == 'ingress':
                return self.IngV4SecurityPolicyIds[0]
            else:
                return self.EgV4SecurityPolicyIds[0]
        elif af == utils.IP_VERSION_6:
            if direction == 'ingress':
                return self.IngV6SecurityPolicyIds[0]
            else:
                return self.EgV6SecurityPolicyIds[0]
        return None

    def GetDependees(self, node):
        """
        depender/dependent - subnet
        dependee - routetable, policy
        """
        dependees = [ self.V4RouteTable, self.V6RouteTable ]
        policyids = self.IngV4SecurityPolicyIds + self.IngV6SecurityPolicyIds
        policyids += self.EgV4SecurityPolicyIds + self.EgV6SecurityPolicyIds
        policyobjs = PolicyClient.GetObjectsByKeys(node, policyids)
        dependees.extend(policyobjs)
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
        elif cObj.ObjType == api.ObjectTypes.POLICY:
            policylist = None
            if cObj.IsV4():
                if cObj.IsIngressPolicy():
                    policylist = self.IngV4SecurityPolicyIds
                elif cObj.IsEgressPolicy():
                    policylist = self.EgV4SecurityPolicyIds
            elif cObj.IsV6():
                if cObj.IsIngressPolicy():
                    policylist = self.IngV6SecurityPolicyIds
                elif cObj.IsEgressPolicy():
                    policylist = self.EgV6SecurityPolicyIds
            if policylist is not None:
                policylist.append(cObj.PolicyId)
            else:
                logger.error(" - ERROR: %s not associated with %s" % \
                             (cObj, self))
                cObj.Show()
                assert(0)
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
        elif dObj.ObjType == api.ObjectTypes.POLICY:
            policylist = None
            if dObj.IsV4():
                if dObj.IsIngressPolicy():
                    policylist = self.IngV4SecurityPolicyIds
                elif dObj.IsEgressPolicy():
                    policylist = self.EgV4SecurityPolicyIds
            elif dObj.IsV6():
                if dObj.IsIngressPolicy():
                    policylist = self.IngV6SecurityPolicyIds
                elif dObj.IsEgressPolicy():
                    policylist = self.EgV6SecurityPolicyIds
            if policylist is not None:
                policylist.remove(dObj.PolicyId)
            else:
                logger.error(" - ERROR: %s not associated with %s" % \
                             (dObj, self))
                assert(0)
        else:
            logger.error(" - ERROR: %s not handling %s deletion" %\
                         (self.ObjType.name, dObj.ObjType))
            dObj.Show()
            assert(0)
        self.SetDirty(True)
        self.CommitUpdate()
        return

class SubnetObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.SUBNET, Resmgr.MAX_SUBNET)
        return

    def GetSubnetObject(self, node, subnetid):
        return self.GetObjectByKey(node, subnetid)

    def GenerateObjects(self, node, parent, vpc_spec_obj):
        poolid = 0
        for subnet_spec_obj in vpc_spec_obj.subnet:
            if hasattr(subnet_spec_obj, 'v6prefixlen'):
                v6prefixlen = subnet_spec_obj.v6prefixlen
            else:
                v6prefixlen = 0
            parent.InitSubnetPefixPools(poolid, v6prefixlen, subnet_spec_obj.v4prefixlen)
            for c in range(subnet_spec_obj.count):
                obj = SubnetObject(node, parent, subnet_spec_obj, poolid)
                self.Objs[node].update({obj.SubnetId: obj})
            poolid = poolid + 1
        return

    def CreateObjects(self, node):
        super().CreateObjects(node)
        # Create VNIC and Remote Mapping Objects
        vnic.client.CreateObjects(node)
        rmapping.client.CreateObjects(node)
        return True

client = SubnetObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
