#! /usr/bin/python3
import pdb
import copy

import infra.config.base as base
from apollo.config.resmgr import client as ResmgrClient
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.rmapping as rmapping
import apollo.config.objects.route as routetable
import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.objects.dhcprelay as dhcprelay

from infra.common.logging import logger
from apollo.config.store import EzAccessStore as EzAccessStore
from apollo.config.store import client as EzAccessStoreClient

def IsAlreadySelected(obj, objs):
    if topo.ChosenFlowObjs.use_selected_objs is True and obj in objs:
        return True
    return False

# Flow based on Local and Remote mapping
class FlowMapObject(base.ConfigObjectBase):
    def __init__(self, lobj, robj, fwdmode, routetblobj = None, tunobj = None, policyobj = None):
        super().__init__()
        self.Node = EzAccessStore.GetDUTNode()
        self.Clone(EzAccessStore.templates.Get('FLOW'))
        self.FlowMapId = next(ResmgrClient[self.Node].FlowIdAllocator)
        self.GID('FlowMap%d'%self.FlowMapId)
        self.FwdMode = fwdmode
        self.__lobj = lobj
        self.__robj = robj
        self.__routeTblObj = routetblobj
        self.__tunobj = tunobj
        self.__policyobj = policyobj
        self.__dev = EzAccessStoreClient[self.Node].GetDevice()
        self.Show()
        return

    def IsFilterMatch(self, selectors):
        filters = selectors.flow.filters
        if filters == None:
            return True
        for attr,value in filters:
            if attr == 'AddrFamily':
                if value != self.__lobj.AddrFamily:
                    return False
            elif attr == 'FwdMode':
                if value != self.FwdMode:
                    return False
        return True

    def SetupTestcaseConfig(self, obj):
        obj.root = self
        obj.localmapping = self.__lobj
        obj.remotemapping = self.__robj
        obj.route = self.__routeTblObj
        obj.tunnel = self.__tunobj
        obj.devicecfg = self.__dev
        #TODO: Handle host mode
        obj.hostport = EzAccessStoreClient[self.Node].GetHostPort()
        obj.switchport = EzAccessStoreClient[self.Node].GetSwitchPort()
        obj.dhcprelay = dhcprelay.client.GetDhcpRelayObject(EzAccessStore.GetDUTNode())
        utils.DumpTestcaseConfig(obj)
        return

    def __repr__(self):
        return "FlowMapId:%d" %(self.FlowMapId)

    def Show(self):
        def __show_flow_object(obj):
            if obj is not None:
                obj.Show()
            return

        logger.info("FlowMap Object: %s" % self)
        __show_flow_object(self.__lobj)
        __show_flow_object(self.__robj)
        __show_flow_object(self.__routeTblObj)
        __show_flow_object(self.__tunobj)
        return

class FlowMapObjectHelper:
    def __init__(self):
        return

    def __is_lmapping_valid(self, lobj):
        if not utils.IsRouteTableSupported():
            return True
        routetblobj = None
        if lobj.AddrFamily == 'IPV4':
            routetblobj = lobj.VNIC.SUBNET.V4RouteTable
        elif lobj.AddrFamily == 'IPV6':
            routetblobj = lobj.VNIC.SUBNET.V6RouteTable
        else:
            return False
        """
            For local2remote, reject if route table has any of the following
            # VPC Peering enabled
            # Has default route
            # Has Blackhole route
        """
        if routetblobj.VPCPeeringEnabled or routetblobj.HasDefaultRoute or\
           routetblobj.HasBlackHoleRoute:
           return False
        return True

    def __is_lmapping_match(self, routetblobj, lobj):
        if lobj.AddrFamily == 'IPV4':
            return lobj.AddrFamily == routetblobj.AddrFamily and\
               lobj.VNIC.SUBNET.V4RouteTableId == routetblobj.RouteTblId
        if lobj.AddrFamily == 'IPV6':
            return lobj.AddrFamily == routetblobj.AddrFamily and\
               lobj.VNIC.SUBNET.V6RouteTableId == routetblobj.RouteTblId

    def __is_lmapping_extract(self, vpcid, lobj):
        return lobj.VNIC.SUBNET.VPC.VPCId == vpcid

    def __is_rmapping_match(self, routetblobj, robj):
        # match remote mapping VPC id with route table's peer VPC for VPC Peering
        if robj.AddrFamily == 'IPV4':
            return robj.AddrFamily == routetblobj.AddrFamily and\
               robj.SUBNET.VPC.VPCId == routetblobj.PeerVPCId
        if robj.AddrFamily == 'IPV6':
            return robj.AddrFamily == routetblobj.AddrFamily and\
               robj.SUBNET.VPC.VPCId == routetblobj.PeerVPCId
        return False

    def GetMatchingConfigObjects(self, selectors):
        objs = []
        fwdmode = None
        mapsel = copy.deepcopy(selectors)
        key = 'FwdMode'

        # Consider it only if TEST is for MAPPING
        if mapsel.flow.GetValueByKey('FlType') != 'MAPPING':
            return objs

        # Get the forwarding mode, fwdmode is not applicable for local & remote
        fwdmode = mapsel.flow.GetValueByKey(key)
        mapsel.flow.filters.remove((key, fwdmode))

        # Get the local2local info.
        key = 'L2LType'
        l2l = mapsel.flow.GetValueByKey(key)
        if l2l != None:
            mapsel.flow.filters.remove((key, l2l))

        # Src and Dst check is not applicable for remote
        rmapsel = copy.deepcopy(mapsel)
        key = 'SourceGuard'
        value = rmapsel.flow.GetValueByKey(key)
        if value != None:
            rmapsel.flow.filters.remove((key, value))

        assert (fwdmode == 'L2' or fwdmode == 'L3' or fwdmode == 'IGW' or \
                fwdmode == 'IGW_NAT' or fwdmode == 'VPC_PEER' or \
                fwdmode == 'POLICY' or fwdmode == 'L2L' or fwdmode == 'IGW_NAPT') == True

        selected_objs = []
        if topo.ChosenFlowObjs.use_selected_objs == True and len(topo.ChosenFlowObjs.objs) != 0:
            matching_flows = topo.ChosenFlowObjs.GetMatchingFlowObjects(selectors)
            selected_objs = utils.GetFilteredObjects(matching_flows, topo.ChosenFlowObjs.GetMaxLimits())
            maxlimits = selectors.maxlimits - len(selected_objs)
            if maxlimits <= 0:
                return utils.GetFilteredObjects(selected_objs, selectors.maxlimits)
        else:
            maxlimits = selectors.maxlimits

        dutNode = EzAccessStore.GetDUTNode()
        if fwdmode == 'VPC_PEER':
            rmappings = rmapping.GetMatchingObjects(mapsel, dutNode)
            lmappings = lmapping.GetMatchingObjects(mapsel, dutNode)
            for routetblobj in routetable.GetAllMatchingObjects(mapsel):
                if routetblobj.IsNatEnabled():
                    # Skip IGWs with nat flag set to True
                    continue
                lmappingobjs = filter(lambda x: self.__is_lmapping_match(routetblobj, x), lmappings)
                rmappingobjs = filter(lambda x: self.__is_rmapping_match(routetblobj, x), rmappings)
                for robj in rmappingobjs:
                    for lobj in lmappingobjs:
                        obj = FlowMapObject(lobj, robj, fwdmode, routetblobj, robj.TUNNEL)
                        if IsAlreadySelected(obj, selected_objs): continue
                        objs.append(obj)
        elif fwdmode == 'IGW':
            for lobj in lmapping.GetMatchingObjects(mapsel, dutNode):
                for routetblobj in routetable.GetAllMatchingObjects(mapsel):
                    if not self.__is_lmapping_match(routetblobj, lobj):
                        continue
                    if routetblobj.IsNatEnabled():
                        # Skip IGWs with nat flag set to True
                        continue
                    obj = FlowMapObject(lobj, None, fwdmode, routetblobj, routetblobj.TUNNEL)
                    if IsAlreadySelected(obj, selected_objs): continue
                    objs.append(obj)
        elif fwdmode == 'IGW_NAT':
            for lobj in lmapping.GetMatchingObjects(mapsel, dutNode):
                if hasattr(lobj, "PublicIP") == False:
                    continue
                for routetblobj in routetable.GetAllMatchingObjects(mapsel):
                    if not self.__is_lmapping_match(routetblobj, lobj):
                        continue
                    if not routetblobj.IsNatEnabled():
                        # Skip IGWs without nat flag set to True
                        continue
                    obj = FlowMapObject(lobj, None, fwdmode, routetblobj, routetblobj.TUNNEL)
                    if IsAlreadySelected(obj, selected_objs): continue
                    objs.append(obj)
        elif fwdmode == 'IGW_NAPT':
            for lobj in lmapping.GetMatchingObjects(mapsel, dutNode):
                if hasattr(lobj, "PublicIP") == True:
                    # skip VNICs which have floating IP
                    continue
                for routetblobj in routetable.GetAllMatchingObjects(mapsel):
                    if not self.__is_lmapping_match(routetblobj, lobj):
                        continue
                    if not routetblobj.IsNatEnabled():
                        # Skip IGWs without nat flag set to True
                        continue
                    obj = FlowMapObject(lobj, None, fwdmode, routetblobj, routetblobj.TUNNEL)
                    if IsAlreadySelected(obj, selected_objs): continue
                    objs.append(obj)
        elif fwdmode == 'POLICY':
            for lobj in lmapping.GetMatchingObjects(mapsel, dutNode):
                for routetblobj in routetable.GetAllMatchingObjects(mapsel):
                    if not self.__is_lmapping_match(routetblobj, lobj):
                        continue
                    obj = FlowMapObject(lobj, None, fwdmode, routetblobj, routetblobj.TUNNEL)
                    if IsAlreadySelected(obj, selected_objs): continue
                    objs.append(obj)
        elif fwdmode == "L2L":
            lobjs = lmapping.GetMatchingObjects(mapsel, dutNode)
            if l2l == 'VPC_PEER':
                for routetblobj in routetable.GetAllMatchingObjects(mapsel):
                    smappingobjs = filter(lambda x: self.__is_lmapping_match(routetblobj, x), lobjs)
                    dmappingobjs = filter(lambda x: self.__is_lmapping_extract(routetblobj.PeerVPCId, x), lobjs)
                    for s in smappingobjs:
                        for d in dmappingobjs:
                            obj = FlowMapObject(s, d, fwdmode)
                            if IsAlreadySelected(obj, selected_objs): continue
                            objs.append(obj)
                objs = utils.GetFilteredObjects(objs, maxlimits)
                utils.MergeFilteredObjects(objs, selected_objs)
                return objs

            for s in lobjs:
                if not self.__is_lmapping_valid(s):
                    continue
                for d in lobjs:
                    if not self.__is_lmapping_valid(d):
                        continue
                    if s.MappingId == d.MappingId:
                        continue
                    if l2l == 'SAME_VNIC':
                        # Select ips from same vnic.
                        if s.VNIC.VnicId != d.VNIC.VnicId:
                            continue;
                    elif l2l == 'SAME_SUBNET':
                        # Select ips from same subnet but different vnic
                        if s.VNIC.VnicId == d.VNIC.VnicId:
                            continue
                        if s.VNIC.SUBNET.SubnetId != d.VNIC.SUBNET.SubnetId:
                            continue
                    elif l2l == 'SAME_VPC':
                        # Select ips from different subnet in a VPC
                        if s.VNIC.SUBNET.SubnetId == d.VNIC.SUBNET.SubnetId:
                            continue
                        if s.VNIC.SUBNET.VPC.VPCId != d.VNIC.SUBNET.VPC.VPCId:
                            continue

                    obj = FlowMapObject(s, d, fwdmode)
                    if IsAlreadySelected(obj, selected_objs): continue
                    objs.append(obj)
        else:
            for lobj in lmapping.GetMatchingObjects(mapsel, dutNode):
                if not self.__is_lmapping_valid(lobj):
                    continue
                for robj in rmapping.GetMatchingObjects(rmapsel, dutNode):
                    if lobj.VNIC.SUBNET.VPC.VPCId != robj.SUBNET.VPC.VPCId:
                        continue
                    # Select mappings from the same subnet if L2 is set
                    if fwdmode == 'L2':
                        if lobj.VNIC.SUBNET.SubnetId != robj.SUBNET.SubnetId:
                            continue
                    else:
                        if lobj.VNIC.SUBNET.SubnetId == robj.SUBNET.SubnetId:
                            continue
                    obj = FlowMapObject(lobj, robj, fwdmode, tunobj=robj.TUNNEL)
                    if IsAlreadySelected(obj, selected_objs): continue
                    objs.append(obj)
        objs = utils.GetFilteredObjects(objs, maxlimits)
        utils.MergeFilteredObjects(objs, selected_objs)
        return objs

FlowMapHelper = FlowMapObjectHelper()

def GetMatchingObjects(selectors):
    return FlowMapHelper.GetMatchingConfigObjects(selectors)
