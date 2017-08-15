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

def VerifySeqNum (tc, dummy):
    #verify seq number is 0xbabababa + 0x40 = 0xBABABAFA
    logger.info("VerifySeqNum!!!")
    return True, "Passed"

