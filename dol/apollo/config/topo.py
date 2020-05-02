#! /usr/bin/python3
import enum
from infra.common.logging  import logger as logger

MTU = 1500
KEEPALIVE_INTERVAL = 60
HOLD_TIME = 3 * KEEPALIVE_INTERVAL

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
    LOOPBACK = 9
    CONTROL = 10

class Ports(enum.IntEnum):
    HOST=0
    UPLINK_0 = 1
    UPLINK_1 = 2

class PortTypes(enum.IntEnum):
    NONE = 0
    HOST = 1
    SWITCH = 2

"""
    # Eth1/1 0x11010001 ==> 1 Hostport
    # Eth2/1 0x11020001 ==> 2 Switchport
"""
INTF2PORT_TBL = { 0x11010001: PortTypes.HOST,
                  0x11020001: PortTypes.SWITCH }
MODE2INTF_TBL = { 'host' : InterfaceTypes.ETH,
                  'switch': InterfaceTypes.UPLINK,
                  'uplink': InterfaceTypes.UPLINK,
                  'l3': InterfaceTypes.L3,
                  'loopback': InterfaceTypes.LOOPBACK,
                  'control': InterfaceTypes.CONTROL}

ETH_IF_PARENT_PORT_SHIFT = 16
ETH_IF_PARENT_PORT_MASK = 0xFF

def PortToEthIfIdx(port):
    ifidx = InterfaceTypes.ETH << 28
    ifidx = ifidx | (1 << 24)
    ifidx = ifidx | (port << 16)
    ifidx = ifidx | 1
    return ifidx

def EthIfIdx2Port(ethifindex):
    return ((ethifindex >> ETH_IF_PARENT_PORT_SHIFT) & ETH_IF_PARENT_PORT_MASK)

DEFAULT_ROUTE_PRIORITY = 0
MIN_ROUTE_PRIORITY = 1023
MAX_ROUTE_PRIORITY = 1

class NhType(enum.IntEnum):
    NONE = 0
    OVERLAY = 2
    UNDERLAY = 4
    UNDERLAY_ECMP = 5
    IP = 8

class NatActionTypes(enum.IntEnum):
    NONE = 0
    STATIC = 1
    NAPT = 2
    NAPT_SERVICE = 3

class L3MatchType(enum.IntEnum):
    PFX = 0
    PFXRANGE = 1
    TAG = 2

class OriginTypes(enum.IntEnum):
     FIXED = 0
     DISCOVERED = 1
     IMPLICITLY_CREATED = 2

class SecurityRuleActionType(enum.IntEnum):
    NONE = 0
    ALLOW = 1
    DENY = 2

class ChosenFlowObjs:
    def __init__(self):
        self.select_objs = False
        self.use_selected_objs = False
        self.objs = []

    def Add(self, obj):
        self.objs.extend(obj)

    def Reset(self):
        self.__init__()

    def SetMaxLimits(self, maxlimits):
        self.maxlimits = maxlimits

    def GetMaxLimits(self):
        return self.maxlimits

    def GetMatchingFlowObjects(self, selectors):
        matching_objs = []
        for obj in self.objs:
            if obj.IsFilterMatch(selectors):
                matching_objs.append(obj)
        return matching_objs


ChosenFlowObjs = ChosenFlowObjs()
