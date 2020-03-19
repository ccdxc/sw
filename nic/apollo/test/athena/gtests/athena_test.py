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
# begin Athena Switch-to-Host UDP
###############################################################################
print('S2H UDP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_ipv4_udp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789a, s=1) / IP(dst='2.0.0.1', src='192.0.2.1') / \
        UDP(sport=0x2710, dport=0x03e8) / payload
dump_pkt(udpompls_ipv4_udp)
###############################################################################
# end Athena Switch-to-Host UDP
###############################################################################

###############################################################################
# begin Athena Host-to-Switch UDP
###############################################################################

print('H2S UDP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv4_udp = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=1) / \
        IP(dst='192.0.2.1', src='2.0.0.1', id=0, ttl=64) / \
        UDP(sport=0x03e8, dport=0x2710) / payload
dump_pkt(ipv4_udp)
###############################################################################
# end Athena Host-to-Switch UDP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host TCP
###############################################################################
print('S2H TCP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_ipv4_tcp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789a, s=1) / IP(dst='2.0.0.1', src='192.0.2.1', proto=6) / \
        TCP(sport=0x2710, dport=0x03e8) / payload
dump_pkt(udpompls_ipv4_tcp)
###############################################################################
# end Athena Switch-to-Host UDP
###############################################################################

###############################################################################
# begin Athena Host-to-Switch TCP
###############################################################################

print('H2S TCP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv4_tcp = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=1) / \
        IP(dst='192.0.2.1', src='2.0.0.1', id=0, ttl=64) / \
        TCP(sport=0x03e8, dport=0x2710) / payload
dump_pkt(ipv4_tcp)
###############################################################################
# end Athena Host-to-Switch UDP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host ICMP
###############################################################################
print('S2H ICMP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_ipv4_tcp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789d, s=1) / IP(dst='2.0.0.1', src='192.0.2.1') / \
        ICMP(type="echo-request", code=0, id=0x1234) / payload
dump_pkt(udpompls_ipv4_tcp)
###############################################################################
# end Athena Switch-to-Host UDP
###############################################################################

###############################################################################
# begin Athena Host-to-Switch ICMP
###############################################################################

print('H2S ICMP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv4_icmp = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=1) / \
        IP(dst='192.0.2.1', src='2.0.0.1', id=0, ttl=64) / \
        ICMP(type="echo-request", code=0, id=0x1234) / payload
dump_pkt(ipv4_icmp)
###############################################################################
# end Athena Host-to-Switch UDP
###############################################################################
