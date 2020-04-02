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
# begin Athena Switch-to-Host IPv4 UDP
###############################################################################
print('S2H IPv4 UDP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_ipv4_udp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789a, s=1) / \
        IP(dst='2.0.0.1', src='192.0.2.1') / \
        UDP(sport=0x2710, dport=0x03e8) / payload
dump_pkt(udpompls_ipv4_udp)
###############################################################################
# end Athena Switch-to-Host IPv4 UDP
###############################################################################

###############################################################################
# begin Athena Host-to-Switch IPv4 UDP
###############################################################################

print('H2S IPv4 UDP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv4_udp = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=1) / \
        IP(dst='192.0.2.1', src='2.0.0.1', id=0, ttl=64) / \
        UDP(sport=0x03e8, dport=0x2710) / payload
dump_pkt(ipv4_udp)


print('Expected: H2S IPv4 UDP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
expected_ipv4_udp = Ether(dst='00:06:07:08:09:0a', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=2) / \
        IP(dst='1.2.3.4', src='4.3.2.1', id=0, ttl=64) / \
        UDP(sport=0, dport=6635, chksum=0) / \
        MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789a, s=1) / \
        IP(dst='192.0.2.1', src='2.0.0.1', id=0, ttl=64) / \
        UDP(sport=0x03e8, dport=0x2710) / payload
dump_pkt(expected_ipv4_udp)


###############################################################################
# end Athena Host-to-Switch IPv4 UDP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host NAT IPv4 UDP
###############################################################################
print('S2H NAT IPv4 UDP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_nat_ipv4_udp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789e, s=1) / \
        IP(dst='3.0.0.1', src='192.0.2.1') / \
        UDP(sport=0x2710, dport=0x03e8) / payload
dump_pkt(udpompls_nat_ipv4_udp)
###############################################################################
# end Athena Switch-to-Host NAT IPv4 UDP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host IPv6 UDP
###############################################################################
print('S2H IPv6 UDP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_ipv6_udp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789a, s=1) / \
        IPv6(src='c00::201', dst='200::1') / \
        UDP(sport=0x2710, dport=0x03e8) / payload
dump_pkt(udpompls_ipv6_udp)
###############################################################################
# end Athena Switch-to-Host IPv6 UDP
###############################################################################

###############################################################################
# begin Athena Host-to-Switch IPv6 UDP
###############################################################################

print('H2S IPv6 UDP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv4_udp = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=1) / \
        IPv6(dst='c00::201', src='200::1') / \
        UDP(sport=0x03e8, dport=0x2710) / payload
dump_pkt(ipv4_udp)
###############################################################################
# end Athena Host-to-Switch IPv6 UDP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host NAT IPv6 UDP
###############################################################################
print('S2H NAT IPv6 UDP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_nat_ipv6_udp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789e, s=1) / \
        IPv6(src='c00::201', dst='300::1') / \
        UDP(sport=0x2710, dport=0x03e8) / payload
dump_pkt(udpompls_nat_ipv6_udp)
###############################################################################
# end Athena Switch-to-Host IPv6 UDP
###############################################################################


###############################################################################
# begin Athena Switch-to-Host IPv4 TCP
###############################################################################
print('S2H IPv4 TCP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_ipv4_tcp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789b, s=1) / IP(dst='2.0.0.1', src='192.0.2.1', proto=6) / \
        TCP(sport=0x2710, dport=0x03e8) / payload
dump_pkt(udpompls_ipv4_tcp)
###############################################################################
# end Athena Switch-to-Host IPv4 TCP
###############################################################################

###############################################################################
# begin Athena Host-to-Switch IPv4 TCP
###############################################################################

print('H2S IPv4 TCP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv4_tcp = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=2) / \
        IP(dst='192.0.2.1', src='2.0.0.1', id=0, ttl=64) / \
        TCP(sport=0x03e8, dport=0x2710) / payload
dump_pkt(ipv4_tcp)
###############################################################################
# end Athena Host-to-Switch IPv4 TCP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host NAT IPv4 TCP
###############################################################################
print('S2H NAT IPv4 TCP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_nat_ipv4_tcp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789e, s=1) / IP(dst='3.0.0.1', src='192.0.2.1', proto=6) / \
        TCP(sport=0x2710, dport=0x03e8) / payload
dump_pkt(udpompls_nat_ipv4_tcp)
###############################################################################
# end Athena Switch-to-Host IPv4 TCP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host IPv6 TCP
###############################################################################
print('S2H IPv6 TCP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_ipv6_tcp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789b, s=1) / \
        IPv6(src='c00::201', dst='200::1') / \
        TCP(sport=0x2710, dport=0x03e8) / payload
dump_pkt(udpompls_ipv6_tcp)
###############################################################################
# end Athena Switch-to-Host IPv6 TCP
###############################################################################

###############################################################################
# begin Athena Host-to-Switch IPv6 TCP
###############################################################################

print('H2S IPv6 TCP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv6_tcp = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=2) / \
        IPv6(dst='c00::201', src='200::1') / \
        TCP(sport=0x03e8, dport=0x2710) / payload
dump_pkt(ipv6_tcp)
###############################################################################
# end Athena Host-to-Switch UDP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host NAT IPv6 TCP
###############################################################################
print('S2H NAT IPv6 TCP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_nat_ipv6_tcp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789e, s=1) / \
        IPv6(src='c00::201', dst='300::1') / \
        TCP(sport=0x2710, dport=0x03e8) / payload
dump_pkt(udpompls_nat_ipv6_tcp)
###############################################################################
# end Athena Switch-to-Host IPv6 TCP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host ICMP
###############################################################################
print('S2H IPv4 ICMP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_ipv4_icmp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789d, s=1) / IP(dst='2.0.0.1', src='192.0.2.1') / \
        ICMP(type="echo-request", code=0, id=0x1234) / payload
dump_pkt(udpompls_ipv4_icmp)
###############################################################################
# end Athena Switch-to-Host ICMP
###############################################################################

###############################################################################
# begin Athena Host-to-Switch ICMP
###############################################################################

print('H2S IPv4 ICMP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv4_icmp = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=4) / \
        IP(dst='192.0.2.1', src='2.0.0.1', id=0, ttl=64) / \
        ICMP(type="echo-request", code=0, id=0x1234) / payload
dump_pkt(ipv4_icmp)
###############################################################################
# end Athena Host-to-Switch ICMP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host NAT ICMP
###############################################################################
print('S2H NAT IPv4 ICMP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_nat_ipv4_icmp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789e, s=1) / IP(dst='3.0.0.1', src='192.0.2.1') / \
        ICMP(type="echo-request", code=0, id=0x1234) / payload
dump_pkt(udpompls_nat_ipv4_icmp)
###############################################################################
# end Athena Switch-to-Host ICMP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host ICMPv6
###############################################################################
print('S2H IPv6 ICMP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_ipv6_tcp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789d, s=1) / \
        IPv6(src='c00::201', dst='200::1') / \
        ICMPv6EchoRequest(id=0x1234) / payload
dump_pkt(udpompls_ipv6_tcp)
###############################################################################
# end Athena Switch-to-Host ICMPv6
###############################################################################

###############################################################################
# begin Athena Host-to-Switch ICMPv6
###############################################################################

print('H2S IPv6 ICMP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv6_icmp = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=4) / \
        IPv6(dst='c00::201', src='200::1') / \
        ICMPv6EchoRequest(id=0x1234) / payload
dump_pkt(ipv6_icmp)
###############################################################################
# end Athena Host-to-Switch ICMPv6
###############################################################################

###############################################################################
# begin Athena Switch-to-Host NAT ICMPv6
###############################################################################
print('S2H NAT IPv6 ICMP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_nat_ipv6_icmp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789e, s=1) / \
        IPv6(src='c00::201', dst='300::1') / \
        ICMPv6EchoRequest(id=0x1234) / payload
dump_pkt(udpompls_nat_ipv6_icmp)
###############################################################################
# end Athena Switch-to-Host ICMPv6
###############################################################################
