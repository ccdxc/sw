#! /usr/bin/python3
import pdb
import copy

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.rmapping as rmapping
import apollo.config.objects.route as routetable
import apollo.config.utils as utils

from infra.common.logging import logger
from apollo.config.store import Store

# Flow based on Local and Remote mapping
class FlowMapObject(base.ConfigObjectBase):
    def __init__(self, lobj, robj, fwdmode, routeobj):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW'))
        self.FlowMapId = next(resmgr.FlowIdAllocator)
        self.GID('FlowMap%d'%self.FlowMapId)
        self.FwdMode = fwdmode
        self.__lobj = lobj
        self.__robj = robj
        self.__dev = Store.GetDevice()
        self.__routeobj = routeobj
        self.Show()
        return

    def SetupTestcaseConfig(self, obj):
        obj.root = self
        obj.localmapping = self.__lobj
        obj.remotemapping = self.__robj
        obj.devicecfg = self.__dev
        obj.route = self.__routeobj
        obj.hostport = utils.PortTypes.HOST
        obj.switchport = utils.PortTypes.SWITCH
        return

    def __repr__(self):
        return "FlowMapId:%d" %(self.FlowMapId)

    def Show(self):
        def __show_flow_object(obj):
            if obj is not None:
                obj.Show()
            return
        logger.info("")
        logger.info("FlowMap Object: %s" % self)
        __show_flow_object(self.__lobj)
        if self.__robj:
            __show_flow_object(self.__robj)
        elif self.__routeobj:
            __show_flow_object(self.__routeobj)
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
        rmapsel = copy.deepcopy(mapsel)

        assert (fwdmode == 'VNET' or fwdmode == 'IGW') == True

        if fwdmode == 'VNET':
            for lobj in lmapping.GetMatchingObjects(mapsel):
                for robj in rmapping.GetMatchingObjects(rmapsel):

                    # Ignore VPC-ID 1 , Reserved for substrate
                    if lobj.VNIC.SUBNET.VPC.VPCId == 1  or robj.SUBNET.VPC.VPCId == 1:
                        continue

                    if lobj.VNIC.SUBNET.VPC.VPCId != robj.SUBNET.VPC.VPCId:
                        continue

                    obj = FlowMapObject(lobj, robj, fwdmode, None)
                    objs.append(obj)

            return utils.GetFilteredObjects(objs, selectors.maxlimits)

        elif fwdmode == 'IGW':
            for lobj in lmapping.GetMatchingObjects(mapsel):
                for routetblobj in routetable.GetAllMatchingObjects(mapsel):
                    if not self.__is_lmapping_match(routetblobj, lobj):
                        continue
                    obj = FlowMapObject(lobj, None, fwdmode, routetblobj)
                    objs.append(obj)

            # Random is disabled for the initial testing. TODO Remove 
            return utils.GetFilteredObjects(objs, selectors.maxlimits, False)

        else:
            assert 0


FlowMapHelper = FlowMapObjectHelper()

def GetMatchingObjects(selectors):
    return FlowMapHelper.GetMatchingConfigObjects(selectors)
