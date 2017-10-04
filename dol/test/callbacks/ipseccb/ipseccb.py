#! /usr/bin/python3

import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs

def GetESPPktSeqNum(tc, pkt):
    return 0 

def GetESPPktSPI(tc, pkt):
    return 0 

def GetIPSecTunnelDstIp (tc, pkt):
    return "10.1.0.2"
def GetIPSecTunnelSrcIp (tc, pkt):
    return "10.1.0.1"
