#! /usr/bin/python3

import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs

def GetSpi (tc, pkt):
    return 0 

def GetSeqNo (tc, pkt):
    return 1 

def GetLt63SeqNo (tc, pkt):
    return 35

def GetGt63SeqNo (tc, pkt):
    return 75

def GetIv (tc, pkt):
    return "0xaaaaaaaaaaaaaaaa"

def GetIcmpIv (tc, pkt):
    return "0xaaaaaaaaaaaaaaab"

def GetUdpIv (tc, pkt):
    return "0xaaaaaaaaaaaaaaab"

def GetSMac (tc, pkt):
    return "00:ee:ff:00:00:02"

def GetDMac (tc, pkt):
    return "00:ee:ff:00:00:03"

def GetEncapVlan (tc, pkt):
    return 2

def GetTos (tc, pkt):
    return 0

def GetLen (tc, pkt):
    return 166

def GetId (tc, pkt):
    return 0

def GetTtl (tc, pkt):
    return 255

def GetCksum (tc, pkt):
    return 0

def GetSIp (tc, pkt):
    return "10.1.0.1"

def GetDIp (tc, pkt):
    return "10.1.0.2"

def GetSIp6 (tc, pkt):
    return "1000::1"

def GetDIp6 (tc, pkt):
    return "1000::2"
