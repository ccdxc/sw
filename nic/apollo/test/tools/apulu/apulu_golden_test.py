#!/usr/bin/python3

import sys
import socket
import codecs

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
# begin golden/apulu.cc
###############################################################################

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
opkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:11:12:13:14:15', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(ipkt, 'g_snd_pkt1')
dump_pkt(opkt, 'g_rcv_pkt1')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:AA:BB:CC:DD:EE', src='00:12:34:56:78:90') / \
        IP(dst='100.101.102.103', src='12.1.1.1', id=0, ttl=64) / \
        UDP(dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:D1:D2:D3:D4:D5', src='00:11:12:13:14:15') / \
        IP(dst='11.11.1.1', src='10.10.1.1') / \
        TCP(sport=0x5678, dport=0x1234) / payload
opkt = Ether(dst='00:C1:C2:C3:C4:C5', src='00:11:12:13:14:15') / \
        Dot1Q(vlan=100) / \
        IP(dst='11.11.1.1', src='10.10.1.1') / \
        TCP(sport=0x5678, dport=0x1234) / payload
dump_pkt(ipkt, 'g_snd_pkt2')
dump_pkt(opkt, 'g_rcv_pkt2')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.10.10', src='11.11.11.11') / \
        TCP(sport=0x1234, dport=0x5678) / payload

arm_hdr_str = '006e000002ed41f250eb1123000000374b00010000000000000000000000'
p4_to_arm_header = codecs.decode(arm_hdr_str, 'hex')
opkt = p4_to_arm_header / \
        Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.10.10', src='11.11.11.11') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(ipkt, 'g_snd_pkt3')
dump_pkt(opkt, 'g_rcv_pkt3')
