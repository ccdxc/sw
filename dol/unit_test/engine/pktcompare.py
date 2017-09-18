#! /usr/bin/python3
import sys
import os

paths = [
    '',
    '/nic',
    '/dol/third_party/'
]

ws_top = os.path.dirname(sys.argv[0]) + '/../../'
ws_top = os.path.abspath(ws_top)
for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

from infra.penscapy.penscapy import *
import infra.engine.comparators as crs
from infra.common.logging import logger

logger.set_level(7)

rawbytes = [1]*64
bnr = '-' * 20

print("%s Received Pkt %s" % (bnr, bnr))
apkt = Ether()/IP()/UDP()/Raw(bytes(rawbytes))
apkt[Ether].src = '00:11:22:33:44:55'
hexdump(apkt)

print("%s Expected Pkt %s" % (bnr, bnr))
epkt = Ether()/Dot1Q()/IP()/UDP()/IP()/UDP()/IP()/UDP()/Raw(bytes(rawbytes))
hexdump(epkt)
elen = len(epkt)

pcr = crs.PacketComparator()
pcr.AddExpected(bytes(epkt), [1], 'PKT1')
pcr.AddReceived(bytes([0] * elen), [1])
pcr.Compare()
pcr.ShowResults()

print("Match = %s" % pcr.IsMatch())
