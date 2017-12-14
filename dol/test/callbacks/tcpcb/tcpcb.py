#! /usr/bin/python3

import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs
from infra.common.objects import MacAddressBase

def GetSeqNum (tc, pkt):
    fin = 0
    if 'F' in pkt.headers.tcp.fields.flags or 'fin' in pkt.headers.tcp.fields.flags:
        fin = 1
    # HACK: Account for RXed bytes here
    # to be used for next TCP segment
    tc.pvtdata.flow1_bytes_rxed += pkt.payloadsize + fin
    return tc.pvtdata.flow1_rcv_nxt

def GetAckNum (tc, pkt):
    return tc.pvtdata.flow1_snd_una

def GetReverseFlowSeqNum (tc, pkt):
    fin = 0
    if 'F' in pkt.headers.tcp.fields.flags or 'fin' in pkt.headers.tcp.fields.flags:
        fin = 1
    tc.pvtdata.flow2_bytes_rxed += pkt.payloadsize + fin
    return tc.pvtdata.flow2_rcv_nxt

def GetReverseFlowAckNum (tc, pkt):
    return tc.pvtdata.flow2_snd_una

def GetReverseFlowAckNumAckOnePkt (tc, pkt):
    fin = 0
    if 'F' in tc.packets.Get('PKT1').headers.tcp.fields.flags:
        fin = 1
    return tc.pvtdata.flow2_snd_una + tc.packets.Get('PKT1').payloadsize + fin

def GetReverseFlowAckNumAckTwoPkts (tc, pkt):
    return tc.pvtdata.flow2_snd_una + tc.packets.Get('PKT1').payloadsize + \
            tc.packets.Get('PKT2').payloadsize

def GetNxtPktSeqNum (tc, pkt):
    pktSeqNum = tc.pvtdata.flow1_rcv_nxt + tc.pvtdata.flow1_bytes_rxed
    tc.pvtdata.flow1_bytes_rxed += pkt.payloadsize
    return pktSeqNum

def GetNxtPktAckNum (tc, pkt):
    return tc.pvtdata.flow1_snd_una

def GetPktOutSeqNum (tc, pkt):
    fin = 0
    if 'F' in tc.packets.Get('PKT1').headers.tcp.fields.flags:
        fin = 1
    if tc.pvtdata.same_flow:
        return tc.pvtdata.flow1_snd_nxt + fin
    else:
        return tc.pvtdata.flow2_snd_nxt + fin

def GetPktOutAckNum (tc, pkt):
    if tc.pvtdata.same_flow:
        return tc.pvtdata.flow1_rcv_nxt + pkt.payloadsize
    else:
        return tc.pvtdata.flow2_rcv_nxt + tc.pvtdata.flow2_bytes_rxed

def GetNxtPktOutSeqNum (tc, pkt):
    return GetPktOutSeqNum(tc, pkt) + pkt.payloadsize

def GetNxtPktOutAckNum (tc, pkt):
    return GetPktOutAckNum(tc, pkt)

def GetReverseFlowPktOutSeqNum (tc, pkt):
    return tc.pvtdata.flow1_snd_nxt

def GetReverseFlowPktOutAckNum (tc, pkt):
    return tc.pvtdata.flow1_rcv_nxt + pkt.payloadsize

def GetAckPktSeqNum (tc, pkt):
    return tc.pvtdata.flow1_snd_nxt

def GetAckPktAckNum (tc, pkt):
    fin = 0
    if 'F' in tc.packets.Get('PKT1').headers.tcp.fields.flags:
        fin = 1
    return tc.pvtdata.flow1_rcv_nxt + tc.packets.Get('PKT1').payloadsize + fin

def GetReverseFlowAckPktSeqNum (tc, pkt):
    return tc.pvtdata.flow2_snd_nxt + tc.packets.Get('PKT1').payloadsize

def GetReverseFlowAckPktAckNum (tc, pkt):
    return tc.pvtdata.flow2_rcv_nxt + tc.packets.Get('PKT2').payloadsize

def GetDstIp (tc, pkt):
    return "54.0.0.2"

def GetOOOSeqNum (tc, pkt):
    return tc.pvtdata.flow1_rcv_nxt + tc.pvtdata.ooo_seq_delta

def GetQuiesceSrcMac(tc, pkt):
    return MacAddressBase(integer=(0x00eeff000004))

def GetQuiesceDstMac(tc, pkt):
    return MacAddressBase(integer=(0x00eeff000005))
