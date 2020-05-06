#!/usr/bin/python3

import sys
import socket
import codecs

sys.path.insert(0, '../dol')
sys.path.insert(0, '../dol/third_party')
from infra.penscapy.penscapy import *
from scapy.packet import *
from scapy.contrib.geneve import *
from socket import ntohs, ntohl

class CapriIntrinsic(Packet):
    name = "Cari Intrinsic"
    fields_desc = [
            BitField("tm_iport", 0, 4),
            BitField("tm_oport", 0, 4),
            BitField("tm_iq", 0, 5),
            BitField("lif", 0, 11),
            BitField("timestamp", 0, 48),
            ByteField("tm_span_session", 0),
            ShortField("tm_replicate_ptr", 0),
            BitField("tm_replicate_en", 0, 1),
            BitField("tm_cpu", 0, 1),
            BitField("tm_q_depth", 0, 14),
            BitField("drop", 0, 1),
            BitField("bypass", 0, 1),
            BitField("hw_error", 0, 1),
            BitField("tm_oq", 0, 5),
            BitField("debug_trace", 0, 1),
            BitField("csum_err", 0, 5),
            BitField("error_bits", 0, 6),
            BitField("tm_instance_type", 0, 4)]

class CapriTxDMAIntrinsic(Packet):
    name = "Cari TxDMA Intrinsic"
    fields_desc = [
            BitField("qid", 0, 24),
            BitField("dma_cmd_ptr", 0, 6),
            BitField("qstate_addr", 0, 34),
            BitField("qtype", 0, 3),
            BitField("txdma_rsv", 0, 5)]

class P4PlusToP4(Packet):
    name = "P4+ to P4"
    fields_desc = [
            BitField("p4plus_app_id", 0, 4),
            BitField("pad", 0, 4),
            BitField("flow_index", 0, 24),
            BitField("lkp_inst", 0, 1),
            BitField("compute_inner_l4_csum", 0, 1),
            BitField("compute_l4_csum", 0, 1),
            BitField("insert_vlan_tag", 0, 1),
            BitField("update_udp_len", 0, 1),
            BitField("update_tcp_seq_no", 0, 1),
            BitField("update_ip_len", 0, 1),
            BitField("update_ip_id", 0, 1),
            ByteField("udp_opt_bytes", 0),
            BitField("dst_lport", 0, 11),
            BitField("dst_lport_valid", 0, 1),
            BitField("pad1", 0, 1),
            BitField("tso_last_segment", 0, 1),
            BitField("tso_first_segment", 0, 1),
            BitField("tso_valid", 0, 1),
            ShortField("ip_id_delta", 0),
            IntField("tcp_seq_delta", 0),
            BitField("gso_start", 0, 14),
            BitField("compute_inner_ip_csum", 0, 1),
            BitField("compute_ip_csum", 0, 1),
            BitField("gso_offset", 0, 14),
            BitField("flow_index_valid", 0, 1),
            BitField("gso_valid", 0, 1),
            BitField("pcp", 0, 3),
            BitField("dei", 0, 1),
            BitField("vid", 0, 12)]

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
            ByteField("tcp_flags", 0),
            IntField("session_id", 0),
            ShortField("lif", 0),
            ShortField("egress_bd_id", 0),
            ShortField("service_xlate_id", 0),
            ShortField("mapping_xlate_id", 0),
            ShortField("tx_meter_id", 0),
            ShortField("nexthop_id", 0),
            ShortField("vpc_id", 0),
            ShortField("vnic_id", 0),
            ShortField("dnat_id", 0),
            BitField("rx_packet", 0, 1),
            BitField("flow_hit", 0, 1),
            BitField("flow_role", 0, 1),
            BitField("is_local", 0, 1),
            BitField("is_l3_vnid", 0, 1),
            BitField("snat_type", 0, 2),
            BitField("dnat_en", 0, 1),
            BitField("mapping_hit", 0, 1),
            BitField("sacl_action", 0, 1),
            BitField("sacl_root", 0, 3),
            BitEnumField("nexthop_type", 0, 2,
                {0: 'VPC', 1: 'ECMP', 2: 'Tunnel', 3: 'Nexthop'}),
            BitField("drop", 0, 1),
            BitField("defunct_flow", 0, 1),
            BitField("local_mapping_ip_type", 0, 2),
            BitField("pad", 0, 5),
            ByteField("epoch", 0),
            IntField("sw_meta", 0),
            ShortField("route_priority", 0)]

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
opkt = P4ToARM(packet_len=ntohs(0x6e), flags='VLAN+IPv4', \
        ingress_bd_id=ntohs(0x02ed), flow_hash=ntohl(0xbf9de0c5), \
        l2_1_offset=0x11, l3_1_offset=0x23, l4_1_offset=0x37, \
        payload_offset=0x4b, lif=ntohs(0x1), epoch=0x55, \
        nexthop_type='Nexthop', vpc_id=ntohs(0x2ec), vnic_id=ntohs(0x2fe), \
        tcp_flags=0x2, mapping_hit=1) / \
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

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.2.3', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
opkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='13.13.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=4789, chksum=0) / VXLAN(vni=0xAFEED) / \
        Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:11:12:13:14:15', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='10.10.2.3', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(ipkt, 'g_snd_pkt5')
dump_pkt(opkt, 'g_rcv_pkt5')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678, flags='F') / payload
opkt = P4ToARM(packet_len=ntohs(0x6e), flags='VLAN+IPv4', flow_hit=1, \
        ingress_bd_id=ntohs(0x02ed), flow_hash=ntohl(0x1b96cb81), \
        l2_1_offset=0x11, l3_1_offset=0x23, l4_1_offset=0x37, \
        payload_offset=0x4b, lif=ntohs(0x1), session_id=ntohl(0x55e51), \
        tcp_flags=0x1, nexthop_type='Nexthop', epoch=0x55, \
        vpc_id=ntohs(0x2ec), vnic_id=ntohs(0x2ee), mapping_hit=1) / \
        Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678, flags='F') / payload
dump_pkt(ipkt, 'g_snd_pkt6')
dump_pkt(opkt, 'g_rcv_pkt6')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.2', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
opkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:11:12:13:14:15', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='10.10.1.2', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(ipkt, 'g_snd_pkt7')
dump_pkt(opkt, 'g_rcv_pkt7')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.3', src='11.11.1.7') / \
        TCP(sport=0x1234, dport=0x5678) / payload
opkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:11:12:13:14:15', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='10.10.1.3', src='11.11.1.7') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(ipkt, 'g_snd_pkt8')
dump_pkt(opkt, 'g_rcv_pkt8')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.9', src='11.11.1.7') / \
        TCP(sport=0x1234, dport=0x5678) / payload
opkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:11:12:13:14:15', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='10.10.1.9', src='11.11.1.7') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(ipkt, 'g_snd_pkt9')
dump_pkt(opkt, 'g_rcv_pkt9')

tcp_opts = [('MSS', 1460), ('WScale', 2), ('SAckOK', ''), ('NOP', None), ('EOL', None)]
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678, flags='S', options=tcp_opts)
opkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:11:12:13:14:15', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678, flags='S', options=tcp_opts)
dump_pkt(ipkt, 'g_snd_pkt10')
dump_pkt(opkt, 'g_rcv_pkt10')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.1', src='11.11.1.11') / \
        TCP(sport=0x1234, dport=0x5678) / payload
opkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:11:12:13:14:15', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='10.10.1.1', src='11.11.1.11') / \
        TCP(sport=0x1234, dport=0x5678) / payload
mpkt1 = Ether(dst='00:0E:0E:0E:0E:0E', src='00:E1:E2:E3:E4:E5') / \
        IP(dst='200.1.1.2', src='200.1.1.1', id=0, ttl=64) / \
        GRE() / \
        ERSPAN(vlan=100, sessionid=1, d=0, gra=3) / \
        ipkt
mpkt2 = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:0E:0E:0E:0E:0E', src='00:E1:E2:E3:E4:E5') / \
        IP(dst='200.1.1.2', src='200.1.1.1', id=0, ttl=64) / \
        GRE() / \
        ERSPAN(vlan=100, sessionid=5, d=0, gra=3) / \
        ipkt
dump_pkt(ipkt, 'g_snd_pkt11')
dump_pkt(opkt, 'g_rcv_pkt11')
dump_pkt(mpkt1, 'g_rcv_mpkt11_1')
dump_pkt(mpkt2, 'g_rcv_mpkt11_2')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.1', src='11.11.1.12') / \
        TCP(sport=0x1234, dport=0x5678) / payload
opkt = P4ToARM(packet_len=ntohs(0x6e), flags='VLAN+IPv4', epoch=0x56, \
        ingress_bd_id=ntohs(0x02ed), flow_hash=ntohl(0x0ebf5db7), \
        l2_1_offset=0x1f, l3_1_offset=0x31, l4_1_offset=0x45, flow_hit=1, \
        payload_offset=0x59, lif=ntohs(0x1), session_id=ntohl(0x55e51), \
        nexthop_type='Nexthop', vpc_id=ntohs(0x2ec), \
        vnic_id=ntohs(0x20c), tcp_flags=0x2, mapping_hit=1, defunct_flow=1) / \
        ipkt
dump_pkt(ipkt, 'g_snd_pkt12')
dump_pkt(opkt, 'g_rcv_pkt12')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = CapriIntrinsic(tm_oport = 0xB, lif=1) / \
        CapriTxDMAIntrinsic() / \
        P4PlusToP4(update_ip_len=1, compute_ip_csum=1, compute_l4_csum=1, \
        compute_inner_ip_csum=1, compute_inner_l4_csum=1) / \
        Ether(dst='00:01:02:03:04:05', src='00:C1:C2:C3:C4:C5') / \
        Dot1Q(vlan=100) / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        UDP(sport=0x1234, dport=6081) / GENEVE(vni=0xFEDCBA) / \
        Ether(dst='00:11:12:13:14:15', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='210.10.1.1', src='211.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
opkt = Ether(dst='00:12:34:56:78:90', src='00:AA:BB:CC:DD:EE') / \
        IP(dst='12.12.1.1', src='100.101.102.103', id=0, ttl=64) / \
        UDP(sport=0x0, dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:11:12:13:14:15', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='10.10.1.1', src='11.11.1.1') / \
        UDP(sport=0x1234, dport=6081) / GENEVE(vni=0xFEDCBA) / \
        Ether(dst='00:11:12:13:14:15', src='00:D1:D2:D3:D4:D5') / \
        IP(dst='210.10.1.1', src='211.11.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(ipkt, 'g_snd_pkt13')
dump_pkt(opkt, 'g_rcv_pkt13')

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
ipkt = Ether(dst='00:AA:BB:CC:DD:EE', src='00:12:34:56:78:90') / \
        IP(dst='100.101.102.103', src='12.1.1.1', id=0, ttl=64) / \
        UDP(dport=4789, chksum=0) / VXLAN(vni=0xABCDEF) / \
        Ether(dst='00:D1:D2:D3:D4:D5', src='00:11:12:13:14:15') / \
        IP(dst='11.11.1.1', src='10.10.1.1') / \
        UDP(sport=0x1234, dport=6081) / GENEVE(vni=0xFEDCBA) / \
        Ether(dst='00:D1:D2:D3:D4:D5', src='00:11:12:13:14:15') / \
        IP(dst='211.11.1.1', src='210.10.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload

opkt = Ether(dst='00:C1:C2:C3:C4:C5', src='00:11:12:13:14:15') / \
        Dot1Q(vlan=100) / \
        IP(dst='11.11.1.1', src='10.10.1.1') / \
        UDP(sport=0x1234, dport=6081) / GENEVE(vni=0xFEDCBA) / \
        Ether(dst='00:D1:D2:D3:D4:D5', src='00:11:12:13:14:15') / \
        IP(dst='211.11.1.1', src='210.10.1.1') / \
        TCP(sport=0x1234, dport=0x5678) / payload
dump_pkt(ipkt, 'g_snd_pkt14')
dump_pkt(opkt, 'g_rcv_pkt14')
