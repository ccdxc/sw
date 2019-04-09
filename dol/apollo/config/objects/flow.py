#! /usr/bin/python3
import pdb
import ipaddress
import copy

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.rmapping as rmapping
import apollo.config.objects.route as routetable
import apollo.config.utils as utils

from infra.common.logging import logger
from apollo.config.store import Store

def IsNatEnabled(routetblobj):
    tunnel = routetblobj.Tunnel
    if tunnel is not None:
        return tunnel.Nat
    return False

# Flow based on Local and Remote mapping
class FlowMapObject(base.ConfigObjectBase):
    def __init__(self, lobj, robj, fwdmode, routetblobj = None, tunobj = None):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW'))
        self.FlowMapId = next(resmgr.FlowIdAllocator)
        self.GID('FlowMap%d'%self.FlowMapId)
        self.FwdMode = fwdmode
        self.__lobj = lobj
        self.__robj = robj
        self.__routeTblObj = routetblobj
        self.__tunobj = tunobj
        self.__dev = Store.GetDevice()
        self.Show()
        return

    def SetupTestcaseConfig(self, obj):
        obj.root = self
        obj.localmapping = self.__lobj
        obj.remotemapping = self.__robj
        obj.route = self.__routeTblObj
        obj.tunnel = self.__tunobj
        obj.devicecfg = self.__dev
        obj.hostport = 1
        obj.switchport = 2
        return

    def __repr__(self):
        return "FlowMapId:%d" %(self.FlowMapId)

    def Show(self):
        logger.info("FlowMap Object: %s" % self)
        logger.info("Local Id:%d|IPAddr:%s|SubnetId:%d|VPCId:%d|VnicId:%d"\
             %(self.__lobj.MappingId, self.__lobj.IPAddr, self.__lobj.VNIC.SUBNET.SubnetId,\
              self.__lobj.VNIC.SUBNET.VPC.VPCId, self.__lobj.VNIC.VnicId))
        if self.__robj is None:
            logger.info("No Remote mapping object")
            self.__routeTblObj.Show()
            return
        logger.info("Remote Id:%d|IPAddr:%s|SubnetId:%d|VPCId:%d"\
             %(self.__robj.MappingId, self.__robj.IPAddr,\
             self.__robj.SUBNET.SubnetId, self.__robj.SUBNET.VPC.VPCId))
        return

class FlowMapObjectHelper:
    def __init__(self):
        return

    def __is_lmapping_match(self, routetblobj, lobj):
        if lobj.AddrFamily == 'IPV4':
            return lobj.AddrFamily == routetblobj.AddrFamily and\
               lobj.VNIC.SUBNET.V4RouteTableId == routetblobj.RouteTblId
        if lobj.AddrFamily == 'IPV6':
            return lobj.AddrFamily == routetblobj.AddrFamily and\
               lobj.VNIC.SUBNET.V6RouteTableId == routetblobj.RouteTblId

    def GetMatchingConfigObjects(self, selectors):
        objs = []
        fwdmode = None
        mapsel = selectors
        key = 'FwdMode'

        # Consider it only if TEST is for MAPPING
        if mapsel.flow.GetValueByKey('FlType') != 'MAPPING':
            return objs

        # Get the forwarding mode, fwdmode is not applicable for local & remote
        fwdmode = mapsel.flow.GetValueByKey(key)
        mapsel.flow.filters.remove((key, fwdmode))

        # Src and Dst check is not applicable for remote
        rmapsel = copy.deepcopy(mapsel)
        key = 'SourceGuard'
        value = rmapsel.flow.GetValueByKey(key)
        if value != None:
            rmapsel.flow.filters.remove((key, value))

        assert (fwdmode == 'L2' or fwdmode == 'L3' or fwdmode == 'IGW' or fwdmode == 'IGW_NAT') == True

        if fwdmode == 'IGW':
            for lobj in lmapping.GetMatchingObjects(mapsel):
                for routetblobj in routetable.GetAllMatchingObjects(mapsel):
                    if not self.__is_lmapping_match(routetblobj, lobj):
                        continue
                    if IsNatEnabled(routetblobj) is True:
                    # Skip IGWs with nat flag set to True
                        continue
                    obj = FlowMapObject(lobj, None, fwdmode, routetblobj, routetblobj.Tunnel)
                    objs.append(obj)
        elif fwdmode == 'IGW_NAT':
            for lobj in lmapping.GetMatchingObjects(mapsel):
                if hasattr(lobj, "PublicIP") == False:
                    continue
                for routetblobj in routetable.GetAllMatchingObjects(mapsel):
                    if not self.__is_lmapping_match(routetblobj, lobj):
                        continue
                    if IsNatEnabled(routetblobj) is False:
                    # Skip IGWs without nat flag set to True
                        continue
                    obj = FlowMapObject(lobj, None, fwdmode, routetblobj, routetblobj.Tunnel)
                    objs.append(obj)
        else:
            for lobj in lmapping.GetMatchingObjects(mapsel):
                for robj in rmapping.GetMatchingObjects(rmapsel):
                    # Select mappings from the same subnet if L2 is set
                    if fwdmode == 'L2':
                        if lobj.VNIC.SUBNET.SubnetId != robj.SUBNET.SubnetId:
                            continue
                    else:
                        if lobj.VNIC.SUBNET.SubnetId == robj.SUBNET.SubnetId:
                            continue
                    obj = FlowMapObject(lobj, robj, fwdmode, None, robj.TunObj)
                    objs.append(obj)
        return utils.GetFilteredObjects(objs, selectors.maxlimits)

FlowMapHelper = FlowMapObjectHelper()

def GetMatchingObjects(selectors):
    return FlowMapHelper.GetMatchingConfigObjects(selectors)
