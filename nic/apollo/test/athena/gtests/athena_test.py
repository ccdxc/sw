#!/usr/bin/python3

import sys
import socket

sys.path.insert(0, '../dol')
sys.path.insert(0, '../dol/third_party')
from infra.penscapy.penscapy import *
from scapy.contrib.mpls import MPLS
from scapy.contrib.geneve import GENEVE

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

###############################################################################
# begin Athena Switch-to-Host NAT ICMPv6
###############################################################################
print('H2S ARP')
arp_pkt = Ether(dst='ff:ff:ff:ff:ff:ff', src='00:AA:BB:CC:DD:EE') / \
        Dot1Q(vlan=1) / \
        ARP(hwtype=1, ptype=0x0800, hwlen=6, plen=4, op=1, hwsrc='00:AA:BB:CC:DD:EE', psrc='2.0.0.1', pdst='192.0.2.1')
dump_pkt(arp_pkt)
###############################################################################
# end Athena Switch-to-Host ICMPv6
###############################################################################
###############################################################################
# begin Athena Switch-to-Host L2 UDP
###############################################################################
print('H2S L2 UDP')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x11\x22\x33'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4a'

geneve_options=opt_src_slot_id + opt_dst_slot_id

geneve_ipv4_udp = Ether(dst='00:06:07:08:09:0A', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=2) / \
        IP(dst='1.2.3.4', src='4.3.2.1', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xAABBCC, options=geneve_options) / \
        Ether(dst='00:00:F1:D0:D1:D1', src='00:00:00:40:08:02') / \
        IP(dst='192.0.2.1', src='2.0.0.1') / \
        UDP(sport=0x2710, dport=0x03e8) / payload

dump_pkt(geneve_ipv4_udp)
###############################################################################
# end Athena Switch-to-Host L2 UDP
###############################################################################
###############################################################################
# begin Athena Host-to-Switch L2 TCP expected packet
###############################################################################
print('H2S L2 TCP EXP')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x10\x20\x30'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4b'

geneve_options=opt_src_slot_id + opt_dst_slot_id

geneve_ipv4_tcp_exp = Ether(dst='00:06:07:08:09:0A', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=2) / \
        IP(dst='1.2.3.4', src='4.3.2.1', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xA0B0C0, options=geneve_options) / \
        Ether(dst='00:00:F1:D0:D1:D0', src='00:00:00:40:08:01') / \
        IP(dst='192.0.2.1', src='2.0.0.1', id=0, ttl=64) / \
        TCP(sport=0x03e8, dport=0x2710) / payload

dump_pkt(geneve_ipv4_tcp_exp)
###############################################################################
# end Athena Switch-to-Host L2 TCP expected
###############################################################################
###############################################################################
# begin Athena Switch-to-Host L2 TCP injected
###############################################################################
print('S2H L2 TCP INJ')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x10\x20\x30'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4b'

geneve_options=opt_src_slot_id + opt_dst_slot_id

geneve_ipv4_tcp_inj = Ether(dst='00:01:02:03:04:05', src='00:06:07:08:09:0A') / \
        Dot1Q(vlan=2) / \
        IP(dst='4.3.2.1', src='1.2.3.4', id=0, ttl=64) / \
        UDP(sport=0xE4E7, chksum=0) / \
        GENEVE(vni=0xA0B0C0, options=geneve_options) / \
        Ether(dst='00:00:00:40:08:01', src='00:00:F1:D0:D1:D0') / \
        IP(dst='2.0.0.1', src='192.0.2.1', id=0, ttl=64) / \
        TCP(sport=0x2710, dport=0x03e8) / payload

dump_pkt(geneve_ipv4_tcp_inj)
###############################################################################
# end Athena Switch-to-Host L2 TCP Injected
###############################################################################
###############################################################################
# begin Athena Switch-to-Host L2 TCP Expected
###############################################################################
print('S2H L2 TCP EXP')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

geneve_ipv4_s2h_tcp_exp = Ether(dst='00:00:00:40:08:01', src='00:00:F1:D0:D1:D0') / \
        Dot1Q(vlan=7) / \
        IP(dst='2.0.0.1', src='192.0.2.1', id=0, ttl=64) / \
        TCP(sport=0x2710, dport=0x03e8) / payload

dump_pkt(geneve_ipv4_s2h_tcp_exp)
###############################################################################
# end Athena Switch-to-Host L2 TCP Expected
###############################################################################
###############################################################################
# begin Athena Host-to-Switch L2 TCP IPV6 Injected
###############################################################################
print('H2S L2 TCP IPV6 INJ')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

geneve_h2s_ipv6_tcp_inj = Ether(dst='00:00:F1:D0:D1:D0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=7) / \
        IPv6(src='200::1', dst='c00::201') / \
        TCP(sport=0x03e8, dport=0x2710) / payload

dump_pkt(geneve_h2s_ipv6_tcp_inj)
###############################################################################
# end Athena Switch-to-Host L2 TCP V6 INJ
###############################################################################
###############################################################################
# begin Athena Host-to-Switch L2 TCP IPV6 Exp
###############################################################################
print('H2S L2 TCP IPV6 EXP')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x10\x20\x30'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4b'

geneve_options=opt_src_slot_id + opt_dst_slot_id

geneve_h2s_ipv6_tcp_exp = Ether(dst='00:06:07:08:09:0A', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=2) / \
        IP(dst='1.2.3.4', src='4.3.2.1', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xA0B0C0, options=geneve_options) / \
        Ether(dst='00:00:F1:D0:D1:D0', src='00:00:00:40:08:01') / \
        IPv6(src='200::1', dst='c00::201') / \
        TCP(sport=0x03e8, dport=0x2710) / payload

dump_pkt(geneve_h2s_ipv6_tcp_exp)
###############################################################################
# end Athena Host-to-Switch L2 TCP V6 EXP
###############################################################################
###############################################################################
# begin Athena Switch-to-Host L2 TCP IPV6 INJ
###############################################################################
print('S2H L2 TCP IPV6 INJ')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x10\x20\x30'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4b'

geneve_options=opt_src_slot_id + opt_dst_slot_id

geneve_s2h_ipv6_tcp_inj = Ether(dst='00:01:02:03:04:05', src='00:06:07:08:09:0A') / \
        Dot1Q(vlan=2) / \
        IP(dst='4.3.2.1', src='1.2.3.4', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xA0B0C0, options=geneve_options) / \
        Ether(dst='00:00:00:40:08:01', src='00:00:F1:D0:D1:D0') / \
        IPv6(src='c00::201', dst='200::1') / \
        TCP(sport=0x2710, dport=0x03e8) / payload

dump_pkt(geneve_s2h_ipv6_tcp_inj)
###############################################################################
# end Athena Switch-to-Host L2 TCP V6 INJ
###############################################################################
###############################################################################
# begin Athena Switch-to-Host L2 TCP IPV6 EXP
###############################################################################
print('S2H L2 TCP IPV6 EXP')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

geneve_s2h_ipv6_tcp_exp =  Ether(dst='00:00:00:40:08:01', src='00:00:F1:D0:D1:D0') / \
        Dot1Q(vlan=7) / \
        IPv6(src='c00::201', dst='200::1') / \
        TCP(sport=0x2710, dport=0x03e8) / payload

dump_pkt(geneve_s2h_ipv6_tcp_exp)
###############################################################################
# end Athena Switch-to-Host L2 TCP V6 EXP
###############################################################################
###############################################################################
# begin Athena Host-to-Switch L2 ICMP V4 INJ
###############################################################################
print('H2S L2 ICMP IPV6 INJ')


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv4_l2_icmp = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=9) / \
        IP(dst='192.0.2.1', src='2.0.0.1', id=0, ttl=64) / \
        ICMP(type="echo-request", code=0, id=0x1234) / payload

dump_pkt(ipv4_l2_icmp)
###############################################################################
# end Athena Host-to-Switch L2 ICMP V4 INJ
###############################################################################
###############################################################################
# begin Athena Host-To_Switch L2 ICMP exp
###############################################################################
print('H2S L2 ICMP EXP')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x11\x22\x33'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4d'

geneve_options=opt_src_slot_id + opt_dst_slot_id

geneve_ipv4_icmp_exp = Ether(dst='00:06:07:08:09:0A', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=2) / \
        IP(dst='1.2.3.4', src='4.3.2.1', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xAABBCC, options=geneve_options) / \
        Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        IP(dst='192.0.2.1', src='2.0.0.1', id=0, ttl=64) / \
        ICMP(type="echo-request", code=0, id=0x1234) / payload

dump_pkt(geneve_ipv4_icmp_exp)
###############################################################################
# end Athena Host-to-Switch L2 ICMP EXP
###############################################################################
###############################################################################
# begin Athena Switch-To-Host L2 ICMP inj
###############################################################################
print('S2H L2 ICMP INJ')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x11\x22\x33'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4d'

geneve_options=opt_src_slot_id + opt_dst_slot_id

geneve_ipv4_icmp_inj = Ether(dst='00:01:02:03:04:05', src='00:06:07:08:09:0A') / \
        Dot1Q(vlan=2) / \
        IP(dst='4.3.2.1', src='1.2.3.4', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xAABBCC, options=geneve_options) / \
        Ether(dst='00:00:00:40:08:01', src='00:00:f1:d0:d1:d0') / \
        IP(dst='2.0.0.1', src='192.0.2.1', id=0, ttl=64) / \
        ICMP(type="echo-request", code=0, id=0x1234) / payload

dump_pkt(geneve_ipv4_icmp_inj)
###############################################################################
# end Athena Switch-to-Host L2 ICMP INJ
###############################################################################

###############################################################################
# begin Athena Switch-To-Host L2 ICMP EXP
###############################################################################
print('S2H L2 ICMP EXP')


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv4_l2_icmp_exp = Ether(dst='00:00:00:40:08:01', src='00:00:f1:d0:d1:d0') / \
        Dot1Q(vlan=9) / \
        IP(dst='2.0.0.1', src='192.0.2.1', id=0, ttl=64) / \
        ICMP(type="echo-request", code=0, id=0x1234) / payload

dump_pkt(ipv4_l2_icmp_exp)
###############################################################################
# end Athena Switch-To-Host L2 ICMP V4 EXP
###############################################################################
###############################################################################
# begin Athena Host-to-Switch L2 ICMP V6 INJ
###############################################################################
print('H2S L2 ICMP IPV6 INJ')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv6_l2_icmp_inj = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=9) / \
        IPv6(dst='c00::201', src='200::1') / \
        ICMPv6EchoRequest(id=0x1234) / payload

dump_pkt(ipv6_l2_icmp_inj)
###############################################################################
# end Athena Host-to-Switch L2 ICMP V4 INJ
###############################################################################

###############################################################################
# begin Athena Host-To_Switch L2 ICMP V6 exp
###############################################################################
print('H2S L2 ICMP EXP')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x11\x22\x33'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4d'

geneve_options=opt_src_slot_id + opt_dst_slot_id

geneve_ipv6_icmp_exp = Ether(dst='00:06:07:08:09:0A', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=2) / \
        IP(dst='1.2.3.4', src='4.3.2.1', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xAABBCC, options=geneve_options) / \
        Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        IPv6(dst='c00::201', src='200::1') / \
        ICMPv6EchoRequest(id=0x1234) / payload

dump_pkt(geneve_ipv6_icmp_exp)
###############################################################################
# end Athena Host-to-Switch L2 ICMP V6 EXP
###############################################################################

###############################################################################
# begin Athena Switch-To-Host L2 ICMP V6 inj
###############################################################################
print('S2H L2 ICMP V6 INJ')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x11\x22\x33'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4d'

geneve_options=opt_src_slot_id + opt_dst_slot_id

geneve_ipv6_icmp_inj = Ether(dst='00:01:02:03:04:05', src='00:06:07:08:09:0A') / \
        Dot1Q(vlan=2) / \
        IP(dst='4.3.2.1', src='1.2.3.4', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xAABBCC, options=geneve_options) / \
        Ether(dst='00:00:00:40:08:01', src='00:00:f1:d0:d1:d0') / \
        IPv6(dst='200::1', src='c00::201') / \
        ICMPv6EchoRequest(id=0x1234) / payload

dump_pkt(geneve_ipv6_icmp_inj)
###############################################################################
# end Athena Switch-to-Host L2 ICMP V6 INJ
###############################################################################

###############################################################################
# begin Athena Switch-To-Host L2 ICMP V6 EXP
###############################################################################
print('S2H L2 ICMP IPV6 EXP')


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv6_l2_icmp_exp = Ether(dst='00:00:00:40:08:01', src='00:00:f1:d0:d1:d0') / \
        Dot1Q(vlan=9) / \
        IPv6(dst='200::1', src='c00::201') / \
        ICMPv6EchoRequest(id=0x1234) / payload

dump_pkt(ipv6_l2_icmp_exp)
###############################################################################
# end Athena Switch-To-Host L2 ICMP V6 EXP
###############################################################################

###############################################################################
# begin Athena Host-To-Switch L2 UDP NAT INJ
###############################################################################
print('S2H L2 UDP IPV4 NAT INJ')


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv4_l2_udp_nat_inj = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=10) / \
        IP(dst='192.0.2.1', src='2.0.0.1') / \
        UDP(sport=0x03e8, dport=0x2710) / payload

dump_pkt(ipv4_l2_udp_nat_inj)
###############################################################################
# end Athena Host-To-Switch L2 UDP NAT INJ
###############################################################################

###############################################################################
# begin Athena Switch-To-Host L2 UDP NAT EXP
###############################################################################
print('S2H L2 UDP IPV4 NAT EXP')


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x11\x22\x33'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4e'

geneve_options=opt_src_slot_id + opt_dst_slot_id

ipv4_l2_udp_nat_exp = Ether(dst='00:06:07:08:09:0A', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=2) / \
        IP(dst='1.2.3.4', src='4.3.2.1', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xAABBCC, options=geneve_options) / \
        Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        IP(dst='192.0.2.1', src='3.0.0.1') / \
        UDP(sport=0x03e8, dport=0x2710) / payload

dump_pkt(ipv4_l2_udp_nat_exp)
###############################################################################
# end Athena Host-To-Switch L2 UDP NAT EXP
###############################################################################

###############################################################################
# begin Athena Switch-To-Host L2 UDP NAT INJ
###############################################################################
print('S2H L2 UDP IPV4 NAT INJ')


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x11\x22\x33'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4e'

geneve_options=opt_src_slot_id + opt_dst_slot_id

ipv4_geneve_udp_nat_inj = Ether(dst='00:01:02:03:04:05', src='00:06:07:08:09:0A') / \
        Dot1Q(vlan=2) / \
        IP(dst='4.3.2.1', src='1.2.3.4', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xAABBCC, options=geneve_options) / \
        Ether(dst='00:00:00:40:08:01', src='00:00:f1:d0:d1:d0') / \
        IP(dst='3.0.0.1', src='192.0.2.1') / \
        UDP(sport=0x2710, dport=0x03e8) / payload

dump_pkt(ipv4_geneve_udp_nat_inj)
###############################################################################
# end Athena Switch-To-Host L2 UDP NAT EXP
###############################################################################

###############################################################################
# begin Athena Switch-To-Host L2 UDP NAT EXP
###############################################################################
print('S2H L2 UDP IPV4 NAT EXP')


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x11\x22\x33'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4e'

geneve_options=opt_src_slot_id + opt_dst_slot_id

ipv4_s2h_l2_udp_nat_exp = Ether(dst='00:00:00:40:08:01', src='00:00:f1:d0:d1:d0') / \
        Dot1Q(vlan=10) / \
        IP(dst='2.0.0.1', src='192.0.2.1') / \
        UDP(sport=0x2710, dport=0x03e8) / payload

dump_pkt(ipv4_s2h_l2_udp_nat_exp)
###############################################################################
# end Athena Switch-To-Host L2 UDP NAT EXP
###############################################################################
###############################################################################
# begin Athena Switch-to-Host NAT IPv4 UDP
###############################################################################
print('H2S NAT IPv4 UDP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl'

udpompls_nat_ipv4_udp = Ether(dst='00:00:F1:D0:D1:D1', src='00:00:00:40:08:02') / \
                        Dot1Q(vlan=5) / \
                        IP(dst='192.0.2.1', src='2.0.0.1') / \
                        UDP(sport=0x03e8, dport=0x2710) / payload
dump_pkt(udpompls_nat_ipv4_udp)
###############################################################################
# end Athena Switch-To-Host L2 UDP NAT EXP
###############################################################################
###############################################################################
# begin Athena Switch-to-Host NAT IPv4 UDP
###############################################################################
print('H2S NAT IPv4 UDP EXP')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl\
           abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxyabcdefghijkl'

udpompls_nat_ipv4_udp = Ether(dst='00:06:07:08:09:0A', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=2) / \
        IP(dst='1.2.3.4', src='4.3.2.1', id=0, ttl=64) / \
        UDP(sport=0x000, dport=6635, chksum=0) / MPLS(label=0x12345, ttl=64, s=0) / \
        MPLS(label=0x6789e, ttl=64, s=1) / \
        IP(dst='192.0.2.1', src='3.0.0.1') / \
        UDP(sport=0x03e8, dport=0x2710) / payload
dump_pkt(udpompls_nat_ipv4_udp)
###############################################################################
# end Athena Switch-To-Host L2 UDP NAT EXP
###############################################################################

###############################################################################
# begin Athena Host-To-Switch L2 IPV6 UDP NAT INJ
###############################################################################
print('S2H L2 UDP IPV6 NAT INJ')


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipv4_l2_ipv6_udp_nat_inj = Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        Dot1Q(vlan=10) / \
        IPv6(src='200::1', dst='c00::201') / \
        UDP(sport=0x03e8, dport=0x2710) / payload

dump_pkt(ipv4_l2_ipv6_udp_nat_inj)
###############################################################################
# end Athena Host-To-Switch L2 IPV6 UDP NAT INJ
###############################################################################

###############################################################################
# begin Athena Switch-To-Host L2 IPV6 UDP NAT EXP
###############################################################################
print('S2H L2 UDP IPV6 NAT EXP')


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x11\x22\x33'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4e'

geneve_options=opt_src_slot_id + opt_dst_slot_id

ipv4_l2_ipv6_udp_nat_exp = Ether(dst='00:06:07:08:09:0A', src='00:01:02:03:04:05') / \
        Dot1Q(vlan=2) / \
        IP(dst='1.2.3.4', src='4.3.2.1', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xAABBCC, options=geneve_options) / \
        Ether(dst='00:00:f1:d0:d1:d0', src='00:00:00:40:08:01') / \
        IPv6(src='300::1', dst='c00::201') / \
        UDP(sport=0x03e8, dport=0x2710) / payload

dump_pkt(ipv4_l2_ipv6_udp_nat_exp)
###############################################################################
# end Athena Host-To-Switch L2 IPV6 UDP NAT EXP
###############################################################################

###############################################################################
# begin Athena Switch-To-Host L2IPV6  UDP NAT INJ
###############################################################################
print('S2H L2 UDP IPV6 NAT INJ')


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x11\x22\x33'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4e'

geneve_options=opt_src_slot_id + opt_dst_slot_id

ipv4_geneve_ipv6_udp_nat_inj = Ether(dst='00:01:02:03:04:05', src='00:06:07:08:09:0A') / \
        Dot1Q(vlan=2) / \
        IP(dst='4.3.2.1', src='1.2.3.4', id=0, ttl=64) / \
        UDP(sport=0x0, chksum=0) / \
        GENEVE(vni=0xAABBCC, options=geneve_options) / \
        Ether(dst='00:00:00:40:08:01', src='00:00:f1:d0:d1:d0') / \
        IPv6(src='c00::201', dst='300::1') / \
        UDP(sport=0x2710, dport=0x03e8) / payload

dump_pkt(ipv4_geneve_ipv6_udp_nat_inj)
###############################################################################
# end Athena Switch-To-Host L2 IPV6 UDP NAT EXP
###############################################################################

###############################################################################
# begin Athena Switch-To-Host L2 IPV6 UDP NAT EXP
###############################################################################
print('S2H L2 UDP IPV6 NAT EXP')


payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'

opt_src_slot_id='\x00\x00\x21\x01\x00\x11\x22\x33'
opt_dst_slot_id='\x00\x00\x22\x01\x00\x01\x23\x4e'

geneve_options=opt_src_slot_id + opt_dst_slot_id

ipv6_s2h_l2_udp_nat_exp = Ether(dst='00:00:00:40:08:01', src='00:00:f1:d0:d1:d0') / \
        Dot1Q(vlan=10) / \
        IPv6(src='c00::201', dst='200::1') / \
        UDP(sport=0x2710, dport=0x03e8) / payload

dump_pkt(ipv6_s2h_l2_udp_nat_exp)
###############################################################################
# end Athena Switch-To-Host L2 UDP NAT EXP
###############################################################################

###############################################################################
# begin Athena Switch-to-Host IPv4 VLAN0
###############################################################################
print('S2H IPv4 VLAN0')
payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
udpompls_ipv4_udp = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0xE4E7, dport=6635, chksum=0) / MPLS(label=0x12345, s=0) / \
        MPLS(label=0x6789f, s=1) / \
        IP(dst='2.0.0.1', src='192.0.2.1') / \
        UDP(sport=0x2710, dport=0x03e8) / payload
dump_pkt(udpompls_ipv4_udp)
###############################################################################
# end Athena Switch-to-Host IPv4 UDP
###############################################################################
