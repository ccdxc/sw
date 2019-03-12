#! /usr/bin/python3
import enum
import infra.config.base as base

from infra.common.logging import logger
from apollo.config.store import Store

class PortTypes(enum.IntEnum):
    NONE = 0
    HOST = 1
    SWITCH = 2

class PortObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__()
        self.GID(spec.port)
        
        ################# PUBLIC ATTRIBUTES OF PORT OBJECT #####################
        self.Type = PortTypes.HOST
        self.Port = spec.port
        
        ################# PRIVATE ATTRIBUTES OF PORT OBJECT #####################
        return

    def __repr__(self):
        return "Port:%d/Type:%s" % (self.port, self.type)
    
class PortObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self):
        return

    def CreateObjects(self):
        return

client = PortObjectClient()
def GetMatchingObjects(selectors):
    return client.Objects()

