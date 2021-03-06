#!/usr/bin/python3

import sys
import socket

sys.path.insert(0, '../dol')
sys.path.insert(0, '../dol/third_party')
from infra.penscapy.penscapy import *

def dump_pkt(pkt, sname):
    print('uint8_t %s[] = {' % sname)
    for p in range(0, len(pkt), 8):
        chunk = bytes(pkt)[p:p+8]
        print('    ' + ', '.join('0x{:02X}'.format(b) for b in chunk), end=",\n")
    print('};\n')

###############################################################################
# begin golden/artemis.cc
###############################################################################

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
opkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xEA63, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(ipkt, 'g_snd_pkt1')
dump_pkt(opkt, 'g_rcv_pkt1')
