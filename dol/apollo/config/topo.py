#! /usr/bin/python3
import enum

class InterfaceTypes(enum.IntEnum):
    NONE = 0
    ETH = 1
    ETH_PC = 2
    TUNNEL = 3
    MGMT = 4
    UPLINK = 5
    UPLINKPC = 6
    L3 = 7
    LIF = 8

class PortTypes(enum.IntEnum):
    NONE = 0
    HOST = 1
    SWITCH = 2

"""
    # Eth1/1 0x11010001 ==> 1 Hostport
    # Eth2/1 0x11020001 ==> 2 Switchport
"""
INTF2PORT_TBL = { 0x11010001: PortTypes.HOST, 0x11020001: PortTypes.SWITCH }
MODE2INTF_TBL = { 'host' : InterfaceTypes.ETH, 'switch': InterfaceTypes.UPLINK,
        'uplink': InterfaceTypes.UPLINK, 'l3': InterfaceTypes.L3 }

class NhType(enum.IntEnum):
    NONE = 0
    OVERLAY = 2
    UNDERLAY = 4
    UNDERLAY_ECMP = 5
    IP = 8

class L3MatchType(enum.IntEnum):
    PFX = 0
    PFXRANGE = 1
    TAG = 2

class OriginTypes(enum.IntEnum):
     FIXED = 0
     DISCOVERED = 1

class rrobiniter:
    def __init__(self, objs):
        assert len(objs) != 0
        self.objs = objs
        self.iterator = iter(objs)
        self.size = len(objs)
    def rrnext(self):
        while True:
            try:
                return next(self.iterator)
            except:
                self.iterator = iter(self.objs)
                continue
    def size(self):
        return self.size

class CachedObjs:
    def __init__(self):
        self.select_objs = False
        self.use_selected_objs = False
        self.objs = []

    def add(self, obj):
        self.objs.extend(obj)

    def reset(self):
        self.__init__()

    def setMaxLimits(self, maxlimits):
        self.maxlimits = maxlimits

    def getMaxLimits(self):
        return self.maxlimits

CachedObjs = CachedObjs()

