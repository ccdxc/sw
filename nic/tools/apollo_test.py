#!/usr/bin/python3

import sys
import socket

sys.path.insert(0, '../dol')
sys.path.insert(0, '../dol/third_party')
from infra.penscapy.penscapy import *
from infra.factory.scapyfactory import IcrcHeaderBuilder
from scapy.contrib.mpls import MPLS

def dump_pkt(pkt):
    print('***')
    for p in range(0, len(pkt), 8):
        chunk = bytes(pkt)[p:p+8]
        print(', '.join('0x{:02X}'.format(b) for b in chunk), end=",\n")

###############################################################################

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
pkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload

rpkt = Ether(dst='00:00:00:00:00:00', src='00:00:00:00:00:00') / \
        IP(dst='0.0.0.0', src='0.0.0.0', id=0, ttl=0, len=0, chksum=0) / \
        GRE(proto=0x8847) / MPLS(label=0, s=1) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload

dump_pkt(pkt)
dump_pkt(rpkt)
