#! /usr/bin/python3

import random
import pdb

NOP_NAME        = 'NOP'
WSCALE_NAME     = 'WScale'
SACK_NAME       = 'SAck'
SACKOK_NAME     = 'SAckOK'
TIMESTAMP_NAME  = 'Timestamp'

class TestspecTcpOption:
    def __init__(self, k, d):
        self.kind = k
        self.data = d
        return

def TcpOptionNop():
    return TestspecTcpOption(NOP_NAME, None)

def TcpOptionSack():
    a = random.randint(1, 32768)
    b = random.randint(1, 32768)
    data = '%s %s' % (str(a), str(b))
    return TestspecTcpOption(SACK_NAME, data)

def GetTcpOptions(tc, pkt):
    opts = []
    if tc.pvtdata.opttype == 'TCP_SACK':
        for c in range(tc.pvtdata.optcount):
            opts.append(TcpOptionSack())
    elif tc.pvtdata.opttype == 'TCP_NOP':
        for c in range(tc.pvtdata.optcount):
            opts.append(TcpOptionNop())

    return opts
