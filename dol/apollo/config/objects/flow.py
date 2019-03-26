#! /usr/bin/python3
import pdb
import ipaddress
import copy

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.rmapping as rmapping

from infra.common.logging import logger
from apollo.config.store import Store

# Flow based on Local and Remote mapping
class FlowMapObject(base.ConfigObjectBase):
    def __init__(self, lobj, robj, fwdmode):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW'))
        self.FlowMapId = next(resmgr.FlowIdAllocator)
        self.GID('FlowMap%d'%self.FlowMapId)
        self.FwdMode = fwdmode
        self.__lobj = lobj
        self.__robj = robj
        self.__dev = Store.GetDevice()
        self.Show()
        return

    def SetupTestcaseConfig(self, obj):
        obj.root = self
        obj.localmapping = self.__lobj
        obj.remotemapping = self.__robj
        obj.devicecfg = self.__dev
        obj.hostport = 1
        obj.switchport = 2
        return

    def __repr__(self):
        return "FlowMapId:%d" %(self.FlowMapId)

    def Show(self):
        logger.info("FlowMap Object: %s" % self)
        logger.info("Local  Id:%d|IPAddr:%s|SubnetId:%d|VPCId:%d|VnicId:%d"\
             %(self.__lobj.MappingId, self.__lobj.IPAddr, self.__lobj.VNIC.SUBNET.SubnetId,\
              self.__lobj.VNIC.SUBNET.VPC.VPCId, self.__lobj.VNIC.VnicId))
        logger.info("Remote Id:%d|IPAddr:%s|SubnetId:%d|VPCId:%d"\
             %(self.__robj.MappingId, self.__robj.IPAddr,\
             self.__robj.SUBNET.SubnetId, self.__robj.SUBNET.VPC.VPCId))
        return

class FlowMapObjectHelper:
    def __init__(self):
        return

    def GetMatchingConfigObjects(self, selectors):
        objs = []
        fwdmode = None
        mapsel = selectors
        key = 'FwdMode'
        skip = 0

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

        for lobj in lmapping.GetMatchingObjects(mapsel):
            for robj in rmapping.GetMatchingObjects(rmapsel):
                # Select mappings from the same subnet if L2 is set
                if fwdmode == 'L2':
                    if lobj.VNIC.SUBNET.SubnetId != robj.SUBNET.SubnetId:
                        continue
                else:
                    if lobj.VNIC.SUBNET.SubnetId == robj.SUBNET.SubnetId:
                        continue
                if selectors.maxlimits is None:
                    if skip < 100:  # TODO
                        skip = skip + 1
                        continue
                    skip = 0
                elif selectors.maxlimits <= len(objs):
                    return objs
                obj = FlowMapObject(lobj, robj, fwdmode)
                objs.append(obj)

        return objs


FlowMapHelper = FlowMapObjectHelper()

def GetMatchingObjects(selectors):
    return FlowMapHelper.GetMatchingConfigObjects(selectors)
