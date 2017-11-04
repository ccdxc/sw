#! /usr/bin/python3

import random
import pdb

import infra.penscapy.penscapy as penscapy

EOL_NAME        = 'EOL'
NOP_NAME        = 'NOP'
WSCALE_NAME     = 'WScale'
SACK_NAME       = 'SAck'
SACKOK_NAME     = 'SAckOK'
TIMESTAMP_NAME  = 'Timestamp'
MSS_NAME        = 'MSS'
AO_NAME         = 'AO'
EXP1_NAME       = 'EXP1'

class TestspecTcpOption:
    def __init__(self, k, d):
        self.kind = k
        self.data = d
        return

def TcpOptionEol():
    return TestspecTcpOption(EOL_NAME, None)

def TcpOptionNop():
    return TestspecTcpOption(NOP_NAME, None)

def TcpOptionSack():
    a = random.randint(1, 32768)
    b = random.randint(1, 32768)
    data = '%s %s' % (str(a), str(b))
    return TestspecTcpOption(SACK_NAME, data)

def TcpOptionMss():
    return TestspecTcpOption(MSS_NAME, '16384')

def TcpOptionWscale():
    return TestspecTcpOption(WSCALE_NAME, '3')

def TcpOptionSackok():
    return TestspecTcpOption(SACKOK_NAME, '0')

def TcpOptionTimestamp():
    return TestspecTcpOption(TIMESTAMP_NAME, '1234 1234')

def TcpOptionAO():
    return TestspecTcpOption(AO_NAME,
                             '12 34 12345678 12345678 12345678 12345678')

def TcpOptionExp1():
    a = '12345678'
    datastr = ''
    for i in range(9):
        datastr = a + ' ' + datastr
    datastr = datastr[:-1]
    return TestspecTcpOption(EXP1_NAME, datastr)


TcpOption2ObjectList = {
    'TCP_EOL'               : [ TcpOptionEol ],
    'TCP_SACK'              : [ TcpOptionSack ],
    'TCP_NOP'               : [ TcpOptionNop ],
    'TCP_MSS'               : [ TcpOptionMss ],
    'TCP_WSCALE'            : [ TcpOptionWscale ],
    'TCP_SACKOK'            : [ TcpOptionSackok ],
    'TCP_TIMESTAMP'         : [ TcpOptionTimestamp ],
    'TCP_MSS_NOP_WSCALE'    : [ TcpOptionMss, TcpOptionNop, TcpOptionWscale ],
    'TCP_MSS_EOL_WSCALE'    : [ TcpOptionMss, TcpOptionEol, TcpOptionWscale ],
    'TCP_AO'                : [ TcpOptionAO ],
    'TCP_EXP1'              : [ TcpOptionExp1 ],
    'TCP_AO_NOP'            : [ TcpOptionAO, TcpOptionNop ],
}

def GetTcpOptions(tc, pkt):
    opts = []
    objlist = TcpOption2ObjectList[tc.pvtdata.type]
    for obj in objlist:
        for c in range(tc.pvtdata.count):
            opts.append(obj())
    return opts

def __get_checksum_value(tc):
    if 'BAD_CHECKSUM' in tc.pvtdata.scenario:
        return 0xFFFF
    elif 'ZERO_CHECKSUM' in tc.pvtdata.scenario:
        return 0
    return None

def GetIpv4Checksum(tc, pkt):
    if tc.pvtdata.scenario is None:
        return None
    if 'IPV4' in tc.pvtdata.scenario:
        return __get_checksum_value(tc)
    return None

def GetTcpChecksum(tc, pkt):
    if tc.pvtdata.scenario is None:
        return None
    if 'TCP' in tc.pvtdata.scenario:
        return __get_checksum_value(tc)
    return None

def GetUdpChecksum(tc, pkt):
    if tc.pvtdata.scenario is None:
        return None
    if 'UDP' in tc.pvtdata.scenario:
        return __get_checksum_value(tc)
    return None

def GetOuterIpv4Checksum(tc, pkt):
    if tc.pvtdata.scenario is None:
        return None
    if 'OUTER_IPV4' in tc.pvtdata.scenario:
        return __get_checksum_value(tc)
    return None

def GetOuterUdpChecksum(tc, pkt):
    if tc.pvtdata.scenario is None:
        return None
    if 'OUTER_UDP' in tc.pvtdata.scenario:
        return __get_checksum_value(tc)
    return None

def GetIpv4Flags(tc, pkt):
    if tc.pvtdata.scenario is None:
        return None
    if 'FIRST_FRAGMENT' in tc.pvtdata.scenario or\
       'MIDDLE_FRAGMENT' in tc.pvtdata.scenario:
        return 'MF'
    return None

def GetIpv4FragOffset(tc, pkt):
    if tc.pvtdata.scenario is None:
        return None
    if 'MIDDLE_FRAGMENT' in tc.pvtdata.scenario or\
       'LAST_FRAGMENT' in tc.pvtdata.scenario:
        return 256
    return 0


IpOption2ObjectList = {
    'EOL'           : penscapy.IPOption_EOL,
    'NOP'           : penscapy.IPOption_NOP,
    'SECURITY'      : penscapy.IPOption_Security,
    'LSRR'          : penscapy.IPOption_LSRR,
    'RR'            : penscapy.IPOption_RR,
    'SSRR'          : penscapy.IPOption_SSRR,
    'STREAMID'      : penscapy.IPOption_Stream_Id,
    'MTUPROBE'      : penscapy.IPOption_MTU_Probe,
    'MTUREPLY'      : penscapy.IPOption_MTU_Reply,
    'TRACEROUTE'    : penscapy.IPOption_Traceroute,
    'ADDREXT'       : penscapy.IPOption_Address_Extension,
    'ROUTERALERT'   : penscapy.IPOption_Router_Alert,
    'SDBM'          : penscapy.IPOption_SDBM,
}

Ipv6ExtHeaders2ObjectList = {
    'HOP_BY_HOP'    : penscapy.IPv6ExtHdrHopByHop,
    'DEST_OPT'      : penscapy.IPv6ExtHdrDestOpt,
    'ROUTING'       : penscapy.IPv6ExtHdrRouting,
    'FRAGMENT'      : penscapy.IPv6ExtHdrFragment,
}

def __get_ipopts_common(tc, pkt, optsdb):
    if tc.pvtdata.ipopts is None:
        return None
    opts = []
    for opt in tc.pvtdata.ipopts:
        objcls = optsdb[opt]
        for c in range(tc.pvtdata.count):
            opts.append(objcls())
    return opts
   
def GetIpv4Options(tc, pkt):
    return __get_ipopts_common(tc, pkt, IpOption2ObjectList)
def GetIpv6ExtHeaders(tc, pkt):
    return __get_ipopts_common(tc, pkt, Ipv6ExtHeaders2ObjectList)
