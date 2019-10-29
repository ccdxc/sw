#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from apollo.config.store import Store

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base


class PortObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF PORT OBJECT #####################
        self.PortId = next(resmgr.PortIdAllocator)
        self.Port = spec.port
        self.AdminState = spec.status
        self.Mode = spec.mode
        self.GID("Port ID:%s"%self.PortId)
        ################# PRIVATE ATTRIBUTES OF PORT OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "PortId:%d|Port:%d|AdminState:%s|Mode:%s" % \
                (self.PortId, self.Port, self.AdminState, self.Mode)

    def Show(self):
        logger.info("PortObject:")
        logger.info("- %s" % repr(self))
        return

class PortObjectClient:
    def __init__(self):
        self.__objs = dict()
        return

    def Objects(self):
        return self.__objs.values()

    def GenerateObjects(self, topospec):
        portlist = getattr(topospec, 'uplink', None)
        if portlist is None:
            return
        for spec in portlist:
            obj = PortObject(spec.entry)
            self.__objs.update({obj.PortId: obj})
            if obj.Mode == 'host':
                Store.SetHostPort(obj.Port)
            elif obj.Mode == 'switch':
                Store.SetSwitchPort(obj.Port)
        return

client = PortObjectClient()
def GetMatchingObjects(selectors):
    return client.Objects()

