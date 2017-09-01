#! /usr/bin/python3

import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs

def GetSeqNum (tc, pkt):
    return 0xbabababa

def GetAckNum (tc, pkt):
    return 0xefefefef

def GetNxtPktSeqNum (tc, pkt):
    return 0xbababb0e

def GetNxtPktAckNum (tc, pkt):
    return 0xefeff043
