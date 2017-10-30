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
}

def GetTcpOptions(tc, pkt):
    opts = []
    objlist = TcpOption2ObjectList[tc.pvtdata.type]
    for obj in objlist:
        for c in range(tc.pvtdata.count):
            opts.append(obj())
    return opts
