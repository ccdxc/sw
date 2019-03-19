#! /usr/bin/python3
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.rmapping as rmapping

from infra.common.logging import logger
from apollo.config.store import Store

MappingId = 1

class MappingObject(base.ConfigObjectBase):
    def __init__(self, lobj, robj):
        super().__init__()
        global MappingId
        self.Clone(Store.templates.Get('MAPPING'))
        self.MappingId = MappingId
        MappingId = MappingId + 1
        self.GID('Mapping%d'%self.MappingId)
        self.__lobj = lobj
        self.__robj = robj
        # Used by packet template generator
        self.AF = lobj.af
        self.FwType = robj.fwtype
        self.Show()
        return

    def SetupTestcaseConfig(self, obj):
        obj.root = self
        #  Input packet header values
        obj.sip = str(self.__lobj.IPAddr)
        obj.dip = str(self.__robj.IPAddr)
        obj.smac = str(self.__lobj.VNIC.MacAddr)
        obj.dmac = str(self.__robj.MacAddr)
        obj.hostport = 1 # TODO Link with port object
        obj.switchport = 2 # TODO Link with port object
        obj.vlanid = int(self.__lobj.VNIC.VlanId)
        # Expected packet header values
        obj.tunsmac = str(Store.GetSwitch().GetLocalMac())
        obj.tundmac = '00:02:0b:0a:0d:0e' # Used by API TODO
        obj.tunsip = str(Store.GetSwitch().GetLocalIP())
        obj.tundip = str(self.__robj.TunIP)
        obj.mplssslot = self.__lobj.VNIC.MplsSlot
        obj.mplsdslot = self.__robj.MplsSlot
        return

    def Show(self):
        logger.info("MAPPING object:%s->%s" %(str(self.__lobj.GID), str(self.__robj.GID)))
        logger.info("- IP:%s->%s" %(str(self.__lobj.IPAddr), str(self.__robj.IPAddr)))
        return

class MappingObjectHelper:
    def __init__(self):
        return

    def GetMatchingConfigObjects(self, selectors):
        objs = []
        for lobj in lmapping.GetMatchingObjects(selectors):
            for robj in rmapping.GetMatchingObjects(selectors):
                obj = MappingObject(lobj, robj)
                objs.append(obj)
                if selectors.maxmappings is None:
                    continue
                if selectors.maxmappings >= len(objs):
                    return objs
        return objs


MappingHelper = MappingObjectHelper()

def GetMatchingObjects(selectors):
    return MappingHelper.GetMatchingConfigObjects(selectors)
