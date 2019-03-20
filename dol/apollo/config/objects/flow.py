#! /usr/bin/python3
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.rmapping as rmapping

from infra.common.logging import logger
from apollo.config.store import Store

# Flow based on Local and Remote mapping
class FlowMapObject(base.ConfigObjectBase):
    def __init__(self, lobj, robj):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW'))
        self.FlowMapId = next(resmgr.FlowIdAllocator)
        self.GID('FlowMap%d'%self.FlowMapId)
        self.__lobj = lobj
        self.__robj = robj
        self.__dev = Store.GetDevice()
        # Used by packet template generator
        self.AddrFamily = lobj.AddrFamily
        self.FwType = robj.FwType
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
        logger.info("Local  Id:%d|IPAddr:%s|SubnetId:%d|PCNId:%d|VnicId:%d"\
             %(self.__lobj.MappingId, self.__lobj.IPAddr, self.__lobj.VNIC.SUBNET.SubnetId,\
              self.__lobj.VNIC.SUBNET.PCN.PCNId, self.__lobj.VNIC.VnicId))
        logger.info("Remote Id:%d|IPAddr:%s|SubnetId:%d|PCNId:%d"\
             %(self.__robj.MappingId, self.__robj.IPAddr,\
             self.__robj.SUBNET.SubnetId, self.__robj.SUBNET.PCN.PCNId))
        return

class FlowMapObjectHelper:
    def __init__(self):
        return

    def GetMatchingConfigObjects(self, selectors):
        objs = []
        fwdmode = None
        mapsel = selectors

        # Get the forwarding mode
        for f in mapsel.flow.filters:
            if f[0] == 'FwdMode':
                fwdmode = f[1]
                break
        if fwdmode != None:
            mapsel.flow.filters.remove((f[0], f[1]))

        assert (fwdmode == 'L2' or fwdmode == 'L3') == True

        for lobj in lmapping.GetMatchingObjects(mapsel):
            for robj in rmapping.GetMatchingObjects(mapsel):
                # Select mappings from the same subnet if L2 is set
                if fwdmode == 'L2':
                    if lobj.VNIC.SUBNET.SubnetId != robj.SUBNET.SubnetId:
                        continue
                else:
                    if lobj.IPPrefix.compare_networks(robj.IPPrefix) == 0:
                        continue
                obj = FlowMapObject(lobj, robj)
                objs.append(obj)
                if selectors.maxlimits is None:
                    continue
                if selectors.maxlimits <= len(objs):
                    return objs
        return objs


FlowMapHelper = FlowMapObjectHelper()

def GetMatchingObjects(selectors):
    return FlowMapHelper.GetMatchingConfigObjects(selectors)
