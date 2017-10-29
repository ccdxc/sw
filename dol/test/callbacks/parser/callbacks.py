#! /usr/bin/python3

import random
import pdb

EOL_NAME        = 'EOL'
NOP_NAME        = 'NOP'
WSCALE_NAME     = 'WScale'
SACK_NAME       = 'SAck'
SACKOK_NAME     = 'SAckOK'
TIMESTAMP_NAME  = 'Timestamp'
MSS_NAME        = 'MSS'

'''
TCPOptions = (
    {0: ("EOL", None),
     1: ("NOP", None),
     2: ("MSS", "!H"),
     3: ("WScale", "!B"),
     4: ("SAckOK", None),
     5: ("SAck", "!"),
     8: ("Timestamp", "!II"),
     14: ("AltChkSum", "!BH"),
     15: ("AltChkSumOpt", None),
     25: ("Mood", "!p")
'''


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

TcpOption2ObjectList = {
    'TCP_EOL'      : [ TcpOptionEol ],
    'TCP_SACK'      : [ TcpOptionSack ],
    'TCP_NOP'       : [ TcpOptionNop ],
    'TCP_MSS'       : [ TcpOptionMss ],
    'TCP_WSCALE'    : [ TcpOptionWscale ],
    'TCP_SACKOK'    : [ TcpOptionSackok ],
    'TCP_TIMESTAMP'         : [ TcpOptionTimestamp ],
    'TCP_MSS_NOP_WSCALE'    : [ TcpOptionMss, TcpOptionNop, TcpOptionWscale ],
    'TCP_MSS_EOL_WSCALE'    : [ TcpOptionMss, TcpOptionEol, TcpOptionWscale ],
}

def GetTcpOptions(tc, pkt):
    opts = []
    objlist = TcpOption2ObjectList[tc.pvtdata.type]
    for obj in objlist:
        for c in range(tc.pvtdata.count):
            opts.append(obj())
    return opts
