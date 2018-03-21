#! /usr/bin/python3
import sys
import os
import binascii
import pdb

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

import infra.engine.comparators as crs
import infra.factory.scapyfactory as scapyfactory

from infra.penscapy.penscapy import *
from infra.common.logging import logger

logger.SetLoggingLevel(7)

rawbytes = [1]*32
dolbytes = [0xFF]*20
bnr = '-' * 20

pd = PENDOL()
crc = CRC()

#apkt = Ether()/IP()/TCP()/Raw(bytes(rawbytes))
apkt = Ether()/IP()/TCP()
apbytes = bytes(apkt)
print("%s ORIGINAL PACKET %s" % (bnr, bnr))
parsed_apkt = scapyfactory.Parse(apbytes)
parsed_apkt.Show(logger)
crc.crc = binascii.crc32(apbytes + bytes(pd))
print("%s WIRE PACKET %s" % (bnr, bnr))
#pdbytes = bytes(pd) + (crc.crc).to_bytes(4)
pdbytes = bytes(pd) + struct.pack("I", crc.crc)
(crc.crc).to_bytes(4, byteorder='little')
allbytes = apbytes + pdbytes
epkt = scapyfactory.Parse(allbytes)
epkt.Show(logger)
