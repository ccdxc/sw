#! /usr/bin/python3

import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
from infra.common.logging       import logger
import infra.common.defs as defs
from infra.common.objects import MacAddressBase

class TcpOptions:
    def __init__(self, kind, data):
        self.kind = kind
        self.data = data
        return

def GetSeqNum (tc, pkt):
    fin = 0
    if 'F' in pkt.headers.tcp.fields.flags or 'fin' in pkt.headers.tcp.fields.flags:
        fin = 1
    if hasattr(tc.pvtdata, 'tcb1'):
        # for multi-flow cases get values from tcb
        # TODO: Later move all logic to use cb. To prevent
        # too many changes, both ways are maintained
        tcb = tc.pvtdata.tcb1
        pktSeqNum = tcb.rcv_nxt
        tcb.rcv_nxt += pkt.payloadsize + fin
    else:
        pktSeqNum = tc.pvtdata.flow1_rcv_nxt
    tc.pvtdata.flow1_bytes_rxed += pkt.payloadsize + fin
    return pktSeqNum

def GetAckNum (tc, pkt):
    if hasattr(tc.pvtdata, 'tcb1'):
        # for multi-flow cases get values from tcb
        # TODO: Later move all logic to use cb. To prevent
        # too many changes, both ways are maintained
        return tc.pvtdata.tcb1.snd_una
    else:
        return tc.pvtdata.flow1_snd_una + tc.pvtdata.flow1_bytes_txed

def GetReverseFlowSeqNum (tc, pkt):
    fin = 0
    if 'F' in pkt.headers.tcp.fields.flags or 'fin' in pkt.headers.tcp.fields.flags:
        fin = 1
    ack_seq_num = tc.pvtdata.flow2_rcv_nxt
    if hasattr(tc.pvtdata, 'tcb1'):
        tc.pvtdata.tcb2.rcv_nxt += pkt.payloadsize + fin
    tc.pvtdata.flow2_bytes_rxed += pkt.payloadsize + fin
    return ack_seq_num

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
    if hasattr(tc.pvtdata, 'tcb1'):
        # for multi-flow cases get values from tcb
        # TODO: Later move all logic to use cb. To prevent
        # too many changes, both ways are maintained
        tcb = tc.pvtdata.tcb1
        pktSeqNum = tcb.rcv_nxt
        tcb.rcv_nxt += pkt.payloadsize
    else:
        pktSeqNum = tc.pvtdata.flow1_rcv_nxt + tc.pvtdata.flow1_bytes_rxed
    tc.pvtdata.flow1_bytes_rxed += pkt.payloadsize
    return pktSeqNum

def GetNxtPktAckNum (tc, pkt):
    return tc.pvtdata.flow1_snd_una

def GetPktOutSeqNum (tc, pkt):
    fin = 0
    if 'F' in pkt.headers.tcp.fields.flags or 'fin' in pkt.headers.tcp.fields.flags:
        fin = 1
    if tc.pvtdata.same_flow:
        pktSeqNum = tc.pvtdata.flow1_snd_nxt
    elif hasattr(tc.pvtdata, 'tcb2'):
        pktSeqNum = tc.pvtdata.tcb2.snd_nxt
        tc.pvtdata.tcb2.snd_nxt += pkt.payloadsize + fin
    else:
        pktSeqNum = tc.pvtdata.flow2_snd_nxt
    tc.pvtdata.flow2_bytes_txed += pkt.payloadsize + fin
    return pktSeqNum

def GetPktOutAckNum (tc, pkt):
    if tc.pvtdata.same_flow:
        return tc.pvtdata.flow1_rcv_nxt + pkt.payloadsize
    else:
        if hasattr(tc.pvtdata, 'tcb2'):
            # for multi-flow cases get values from tcb
            # TODO: Later move all logic to use cb. To prevent
            # too many changes, both ways are maintained
            return tc.pvtdata.tcb2.rcv_nxt
        else:
            return tc.pvtdata.flow2_rcv_nxt + tc.pvtdata.flow2_bytes_rxed

def GetNxtPktOutSeqNum (tc, pkt):
    fin = 0
    if 'F' in pkt.headers.tcp.fields.flags or 'fin' in pkt.headers.tcp.fields.flags:
        fin = 1

    if tc.pvtdata.same_flow:
        pktSeqNum = tc.pvtdata.flow1_snd_nxt + \
                tc.pvtdata.flow1_bytes_txed
    elif hasattr(tc.pvtdata, 'tcb2'):
        # for multi-flow cases get values from tcb
        # TODO: Later move all logic to use cb. To prevent
        # too many changes, both ways are maintained
        pktSeqNum = tc.pvtdata.tcb2.snd_nxt
        tc.pvtdata.tcb2.snd_nxt += pkt.payloadsize + fin
    else:
        pktSeqNum = tc.pvtdata.flow2_snd_nxt + \
                tc.pvtdata.flow2_bytes_txed
    tc.pvtdata.flow2_bytes_txed += pkt.payloadsize + fin
    return pktSeqNum

def GetNxtPktOutAckNum (tc, pkt):
    return GetPktOutAckNum(tc, pkt)

def GetReverseFlowPktOutSeqNum (tc, pkt):
    return tc.pvtdata.flow1_snd_nxt

def GetReverseFlowPktOutAckNum (tc, pkt):
    return tc.pvtdata.flow1_rcv_nxt + tc.pvtdata.flow1_bytes_rxed

def GetAckPktSeqNum (tc, pkt):
    fin = 0
    if 'F' in pkt.headers.tcp.fields.flags or 'fin' in pkt.headers.tcp.fields.flags:
        fin += 1

    if hasattr(tc.pvtdata, 'tcb1'):
        pktSeqNum = tc.pvtdata.tcb1.snd_nxt
        tc.pvtdata.tcb1.snd_nxt += pkt.payloadsize + fin
        tc.pvtdata.tcb1.snd_una += pkt.payloadsize + fin
    else:
        pktSeqNum = tc.pvtdata.flow1_snd_nxt + tc.pvtdata.flow1_bytes_txed
    tc.pvtdata.flow1_bytes_txed += pkt.payloadsize + fin
    return pktSeqNum

def GetAckPktAckNum (tc, pkt):
    return tc.pvtdata.flow1_rcv_nxt + tc.pvtdata.flow1_bytes_rxed

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

def GetOpposingFlowTxCos(tc, pkt):
    flow = tc.config.flow
    oflow = flow.GetOpposingFlow()
    return oflow.txqos.cos

def GetOpposingFlowTxDscp(tc, pkt):
    flow = tc.config.flow
    oflow = flow.GetOpposingFlow()
    return oflow.txqos.dscp

def GetPktInTcpOptions(tc, pkt):
    return [] # Disable timestamps for now

    echo_ts = []
    ts_str = str(hex(tc.pvtdata.timestamp)) + ' 0x0'
    echo_ts.append(TcpOptions('Timestamp', ts_str))
    echo_ts.append(TcpOptions('NOP', None))
    echo_ts.append(TcpOptions('EOL', None))
    return echo_ts

def GetPktOutTcpOptions(tc, pkt):
    return [] # Disable timestamps for now

    echo_ts = []
    echo_ts.append(TcpOptions('Timestamp', '0x0 0x0'))
    echo_ts.append(TcpOptions('NOP', None))
    echo_ts.append(TcpOptions('EOL', None))
    return echo_ts

def GetPktAckOutTcpOptions(tc, pkt):
    return [] # Disable timestamps for now

    echo_ts = []
    ts_str = '0x0 ' + str(hex(tc.pvtdata.timestamp))
    echo_ts.append(TcpOptions('Timestamp', ts_str))
    echo_ts.append(TcpOptions('NOP', None))
    echo_ts.append(TcpOptions('EOL', None))
    return echo_ts
