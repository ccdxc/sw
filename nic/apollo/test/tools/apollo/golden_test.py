#!/usr/bin/python3

import sys
import socket

sys.path.insert(0, '../dol')
sys.path.insert(0, '../dol/third_party')
from infra.penscapy.penscapy import *
from scapy.contrib.mpls import MPLS

def dump_pkt(pkt):
    print('***')
    for p in range(0, len(pkt), 8):
        chunk = bytes(pkt)[p:p+8]
        print(', '.join('0x{:02X}'.format(b) for b in chunk), end=",\n")

###############################################################################
# begin golden/main.cc
###############################################################################

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
spkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
urpkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=200, s=1) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
grpkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        GRE(proto=0x8847) / MPLS(label=200, s=1) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(spkt)
dump_pkt(urpkt)

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
spkt = Ether(dst='00:AA:BB:CC:DD:EE', src='00:12:34:56:78:90') / \
        IP(dst='100.101.102.103', src='12.12.1.1', id=0, ttl=64) / \
        UDP(sport=0xC0D, dport=6635) / MPLS(label=0x12345, s=1) / \
        IP(dst='11.11.1.1', src='10.10.1.1') / \
        TCP(sport=0x5678, dport=0x1234) / payload
rpkt = Ether(dst='00:C1:C2:C3:C4:C5', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=100) / \
        IP(dst='11.11.1.1', src='10.10.1.1') / \
        TCP(sport=0x5678, dport=0x1234) / payload
dump_pkt(spkt)
dump_pkt(rpkt)

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
spkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.2', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
rpkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xC1AF, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='10.10.1.2', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(spkt)
dump_pkt(rpkt)

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
spkt = Ether(dst='00:AA:BB:CC:DD:EE', src='00:12:34:56:78:90') / \
        IP(dst='100.101.102.103', src='12.12.1.1', id=0, ttl=64) / \
        UDP(sport=0x2BA9, dport=4789, chksum=0) / VXLAN(vni=0x12345) / \
        Ether(dst='00:C1:C2:C3:C4:C5', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='11.11.1.1', src='10.10.1.2') / \
        TCP(sport=0x5678, dport=0x1234) / payload
rpkt = Ether(dst='00:C1:C2:C3:C4:C5', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=100) / \
        IP(dst='11.11.1.1', src='10.10.1.2') / \
        TCP(sport=0x5678, dport=0x1234) / payload
dump_pkt(spkt)
dump_pkt(rpkt)

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
spkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='11.11.2.2', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
rpkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C6') / \
        Dot1Q(vlan=101) / \
        IP(dst='11.11.2.2', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
mpkt = Ether(dst='00:0E:0E:0E:0E:0E', src='00:E1:E2:E3:E4:E5') / \
        Dot1Q(vlan=0xEEE) / \
        IP(dst='200.1.1.2', src='200.1.1.1', id=0, ttl=64) / \
        GRE() / \
        ERSPAN(vlan=101, sessionid=6, d=1, gra=3) / \
        rpkt
dump_pkt(spkt)
dump_pkt(rpkt)
dump_pkt(mpkt)

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
spkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=505) / \
        IP(dst='10.10.1.2', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
rpkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xC1AF, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='10.10.1.2', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
mpkt = Ether(dst='00:0E:0E:0E:0E:0E', src='00:E1:E2:E3:E4:E5') / \
        Dot1Q(vlan=0xEEE) / \
        IP(dst='200.1.1.2', src='200.1.1.1', id=0, ttl=64) / \
        GRE() / \
        ERSPAN(vlan=505, sessionid=6, d=0, gra=3) / \
        spkt
dump_pkt(spkt)
dump_pkt(rpkt)
dump_pkt(mpkt)

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
spkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.100.100.100', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
rpkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xF349, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='10.100.100.100', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(spkt)
dump_pkt(rpkt)
exit(0)

###############################################################################
# end golden/main.cc
###############################################################################
###############################################################################
# begin scale/main.cc
###############################################################################

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
spkt =  Ether(src='00:00:00:40:08:01', dst='00:00:F1:D0:D1:D0') / \
        Dot1Q(vlan=1) / \
        IP( src='2.0.0.1', dst='192.0.2.1') / \
        UDP(sport=1000, dport=10000, chksum=0) / payload

rpkt =  Ether(src='00:02:01:00:00:01', dst='00:02:0b:0a:0d:0e') / \
        IP(src='1.0.0.2', dst='1.0.0.5', id=0, ttl=64, chksum=0x7873) / \
        UDP(sport=0xC6EF, dport=0x19EB, chksum=0) / \
        MPLS(label=1, s=0) / \
        MPLS(label=0, s=1) / \
        IP( src='2.0.0.1', dst='192.0.2.1') / \
        UDP(sport=1000, dport=10000, chksum=0) / payload

dump_pkt(spkt)
dump_pkt(rpkt)

spkt =  Ether(src='00:02:0b:0a:0d:0e', dst='00:02:01:00:00:01') / \
        IP(src='1.0.0.3', dst='1.0.0.2', id=0, ttl=64, chksum=0) / \
        UDP(sport=0x92BA, dport=6635, chksum=0) / \
        MPLS(label=1025, s=0) / \
        MPLS(label=1, s=1) / \
        IP(src='2.0.0.2', dst='2.0.0.1') / \
        UDP(sport=10000, dport=1000, chksum=0) / payload

rpkt =  Ether(src='00:00:00:40:09:01', dst='00:00:00:40:08:01') / \
        Dot1Q(vlan=1) / \
        IP(src='2.0.0.2', dst='2.0.0.1') / \
        UDP(sport=10000, dport=1000, chksum=0) / payload

dump_pkt(spkt)
dump_pkt(rpkt)

spkt =  Ether(src='00:00:00:40:08:01', dst='00:00:F1:D0:D1:D0') / \
        Dot1Q(vlan=1) / \
        IP( src='2.0.0.1', dst='2.0.32.1') / \
        UDP(sport=1000, dport=10000, chksum=0) / payload

rpkt =  Ether(src='00:02:01:00:00:01', dst='00:02:0b:0a:0d:0e') / \
        IP(src='1.0.0.2', dst='1.0.0.3', id=0, ttl=64, chksum=0x7875) / \
        UDP(sport=0xF68B, dport=0x19EB, chksum=0) / \
        MPLS(label=1, s=0) / \
        MPLS(label=1025, s=1) / \
        IP( src='2.0.0.1', dst='2.0.32.1') / \
        UDP(sport=1000, dport=10000, chksum=0) / payload

dump_pkt(spkt)
dump_pkt(rpkt)

spkt =  Ether(src='00:00:00:40:08:01', dst='00:00:F1:D0:D1:D0') / \
        Dot1Q(vlan=1) / \
        IPv6( src='2019::0200:0001', dst='2021::f1d0:d1d0:c000:0201') / \
        UDP(sport=1000, dport=10000, chksum=0) / payload

rpkt =  Ether(src='00:02:01:00:00:01', dst='00:02:0b:0a:0d:0e') / \
        IP(src='1.0.0.2', dst='1.0.0.5', id=0, ttl=64, chksum=0x785F) / \
        UDP(sport=0xD96F, dport=0x19EB, chksum=0) / \
        MPLS(label=1, s=0) / \
        MPLS(label=0, s=1) / \
        IPv6( src='2019::0200:0001', dst='2021::f1d0:d1d0:c000:0201') / \
        UDP(sport=1000, dport=10000, chksum=0) / payload

dump_pkt(spkt)
dump_pkt(rpkt)
