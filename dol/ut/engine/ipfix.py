#! /usr/bin/python3
import sys
import os
import binascii
import pdb

paths = [
    '',
    '/nic',
    '/dol/',
    '/dol/third_party'
]

ws_top = os.path.dirname(sys.argv[0]) + '/../../../'
ws_top = os.path.abspath(ws_top)
for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

import infra.engine.comparators as crs
import infra.factory.scapyfactory as scapyfactory

from infra.penscapy.penscapy import *
from infra.common.logging import logger

logger.set_level(7)

a = Ether()/IP()/UDP(sport=49000)
ipf = Ipfix()
v4rec = IpfixRecord()/IpfixRecordIpv4Data()
v6rec = IpfixRecord()/IpfixRecordIpv6Data()
ethrec = IpfixRecord()/IpfixRecordNonIpData()

records = []
records.append(IpfixRecord(bytes(ethrec)))
records.append(IpfixRecord(bytes(v6rec)))
records.append(IpfixRecord(bytes(v4rec)))

ipf[Ipfix].records = records

a = a/ipf
a.show2(indent = 0)
hexdump(a)

newpkt = Ether(bytes(a))
newpkt.show(indent = 0)
