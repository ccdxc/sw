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

def IsNatEnabled(routetblobj):
    tunnel = routetblobj.Tunnel
    if tunnel is not None:
        return tunnel.Nat
    return False

# Flow based on Local and Remote mapping
class FlowMapObject(base.ConfigObjectBase):
    def __init__(self, lobj, robj, fwdmode, tunobj = None):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW'))
        self.FlowMapId = next(resmgr.FlowIdAllocator)
        self.GID('FlowMap%d'%self.FlowMapId)
        self.FwdMode = fwdmode
        self.__lobj = lobj
        self.__robj = robj
        self.__dev = Store.GetDevice()
        self.__tunobj = tunobj
        self.Show()
        return

    def SetupTestcaseConfig(self, obj):
        obj.root = self
        obj.localmapping = self.__lobj
        obj.remotemapping = self.__robj
        obj.devicecfg = self.__dev
        obj.tunnel = self.__tunobj
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

        logger.info("FlowMap Object: %s" % self)
        __show_flow_object(self.__lobj)
        __show_flow_object(self.__robj)
        return

class FlowMapObjectHelper:
    def __init__(self):
        return

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

        assert (fwdmode == 'L3') == True

        if True:
            for lobj in lmapping.GetMatchingObjects(mapsel):
                for robj in rmapping.GetMatchingObjects(rmapsel):

                    # Ignore VPC-ID 1 now as it does not work. TODO.
                    if lobj.VNIC.SUBNET.VPC.VPCId == 1  or robj.SUBNET.VPC.VPCId == 1:
                        continue

                    if lobj.VNIC.SUBNET.VPC.VPCId != robj.SUBNET.VPC.VPCId:
                        continue

                    obj = FlowMapObject(lobj, robj, fwdmode, robj.TUNNEL)
                    objs.append(obj)
        return utils.GetFilteredObjects(objs, selectors.maxlimits)

FlowMapHelper = FlowMapObjectHelper()

def GetMatchingObjects(selectors):
    return FlowMapHelper.GetMatchingConfigObjects(selectors)
