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

urpkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64, chksum=0) / \
        UDP(sport=0xEE08, dport=6635, chksum=0) / MPLS(label=200, s=1) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload

grpkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64, chksum=0) / \
        GRE(proto=0x8847) / MPLS(label=200, s=1) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload

dump_pkt(pkt)
dump_pkt(urpkt)


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
pkt = Ether(dst='00:AA:BB:CC:DD:EE', src='00:12:34:56:78:90') / \
        IP(dst='100.101.102.103', src='12.12.1.1', id=0, ttl=64) / \
        UDP(sport=0xC0D, dport=6635) / MPLS(label=0x12345, s=1) / \
        IP(dst='11.11.1.1', src='10.10.1.1') / \
        TCP(sport=0x5678, dport=0x1234) / payload

rpkt = Ether(dst='00:C1:C2:C3:C4:C5', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=100) / \
        IP(dst='11.11.1.1', src='10.10.1.1') / \
        TCP(sport=0x5678, dport=0x1234) / payload

dump_pkt(pkt)
dump_pkt(rpkt)
