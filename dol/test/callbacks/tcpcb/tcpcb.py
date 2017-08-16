#! /usr/bin/python3

import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs

def GetSMac (tc, pkt):
    return "00:22:22:22:22:22"

def GetDMac (tc, pkt):
    return "00:33:33:33:33:33"

def GetVlanId (tc, pkt):
    return 2

def GetSip (tc, pkt):
    return "64.2.0.4"

def GetDip (tc, pkt):
    return "64.2.0.5"

def GetDPort (tc, pkt):
    return 80

def GetSPort (tc, pkt):
    return 47273

def GetSeqNum (tc, pkt):
    return 0xbabababa

def GetAckNum (tc, pkt):
    return 0xefefefef

def VerifySeqNum (tc, dummy):
    #verify seq number is 0xbabababa + 0x40 = 0xBABABAFA
    logger.info("VerifySeqNum!!!")
    return True, "Passed"

