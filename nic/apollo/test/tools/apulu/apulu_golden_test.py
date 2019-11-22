#!/usr/bin/python3

import sys
import socket
import codecs

sys.path.insert(0, '../dol')
sys.path.insert(0, '../dol/third_party')
from infra.penscapy.penscapy import *
from scapy.packet import *

class P4ToARM(Packet):
    name = "P4ToARM"
    fields_desc = [
            ShortField("packet_len", 0),
            FlagsField("flags", 0, 16,
                ["VLAN", "IPv4", "IPv6", "InnerEth", "InnerIPv4", "InnerIPv6"]),
            ShortField("ingress_bd_id", 0),
            IntField("flow_hash", 0),
            ByteField("l2_1_offset", 0),
            ByteField("l3_1_offset", 0),
            ByteField("l4_1_offset", 0),
            ByteField("l2_2_offset", 0),
            ByteField("l3_2_offset", 0),
            ByteField("l4_2_offset", 0),
            ByteField("payload_offset", 0),
            ShortField("lif", 0),
            ShortField("egress_bd_id", 0),
            ShortField("service_xlate_id", 0),
            ShortField("mapping_xlate_id", 0),
            ShortField("tx_meter_id", 0),
            ShortField("nexthop_id", 0),
            BitField("pad", 0, 5),
            BitEnumField("nexthop_type", 0, 2,
                {0: 'VPC', 1: 'ECMP', 2: 'Tunnel', 3: 'Nexthop'}),
            BitField("drop", 0, 1) ]

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
opkt = P4ToARM(packet_len=0x6e, flags='VLAN+IPv4', ingress_bd_id=0x02ed, \
               flow_hash=0x41f250eb, l2_1_offset=0x11, l3_1_offset=0x23, \
               l4_2_offset=0x37, payload_offset=0x4b, lif=0x1, \
               nexthop_id=0x1ef, nexthop_type='Tunnel') / \
        Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.10.10', src='11.11.11.11') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(ipkt, 'g_snd_pkt3')
dump_pkt(opkt, 'g_rcv_pkt3')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.2.2', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
opkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='13.13.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=6635, chksum=0) / \
        MPLS(label=0xFEED, s=1, ttl=64) / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:11:12:13:14:15', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='10.10.2.2', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(ipkt, 'g_snd_pkt4')
dump_pkt(opkt, 'g_rcv_pkt4')
