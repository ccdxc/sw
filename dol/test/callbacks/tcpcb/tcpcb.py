#! /usr/bin/python3

import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs

def GetSeqNum (tc, pkt):
    if tc.config.flow.IsIflow():
        return 0x1abababa
    else:
        return 0x2abababa

def GetAckNum (tc, pkt):
    if tc.config.flow.IsIflow():
        return 0x1fefefef
    else:
        return 0x2fefefef

def GetNxtPktSeqNum (tc, pkt):
    if tc.config.flow.IsIflow():
        return 0x1ababb0e
    else:
        return 0x2ababb0e

def GetNxtPktAckNum (tc, pkt):
    if tc.config.flow.IsIflow():
        return 0x1fefefef
    else:
        return 0x2fefefef

def GetDstIp (tc, pkt):
    return "54.0.0.2"
