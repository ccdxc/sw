#! /usr/bin/python3
import evpn_pb2 as evpn_pb2
import types_pb2 as types_pb2
import struct

def GetRTType(RTType):
    if RTType == 'import':
        return evpn_pb2.EVPN_RT_IMPORT
    elif RTType == 'export':
        return evpn_pb2.EVPN_RT_EXPORT
    elif RTType == 'import_export':
        return evpn_pb2.EVPN_RT_IMPORT_EXPORT
    elif RTType == 'none':
        return evpn_pb2.EVPN_RT_NONE
    else:
        return evpn_pb2.EVPN_RT_INVALID

def GetRT(RT):
    rt_array = bytearray()
    for x in RT.split(':'):
        rt_array.append(int(x,16))
    return bytes(rt_array)

def GetEVPNCfg(EVPNCfg):
    if EVPNCfg == 'auto':
        return evpn_pb2.EVPN_CFG_AUTO
    elif EVPNCfg == 'manual':
        return evpn_pb2.EVPN_CFG_MANUAL
    else:
        return evpn_pb2.EVPN_CFG_INVALID

def IPv4EndianReverse(ipaddr):
    if ipaddr.Af == types_pb2.IP_AF_INET:
       u32 = ipaddr.V4Addr
       ipaddr.V4Addr = struct.unpack("<I", struct.pack(">I", u32))[0]
