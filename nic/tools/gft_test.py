#!/usr/bin/python3

import sys
#sys.path.insert(0, 'model_sim/src')
#import model_wrap

sys.path.insert(0, '../dol')
sys.path.insert(0, '../dol/third_party')
from infra.penscapy.penscapy import *

def dump_pkt(pkt):
    print('***')
    for p in range(0, len(pkt), 8):
        chunk = bytes(pkt)[p:p+8]
        print(', '.join('0x{:02X}'.format(b) for b in chunk), end=",\n")

###############################################################################

pkt = bytes(Ether(dst='00:01:02:03:04:05', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3') / \
        UDP(dport=4789) / VXLAN() / \
        Ether(dst='00:11:12:13:14:15', src='00:B1:B2:B3:B4:B5') / \
        IP(dst='12.1.2.3', src='13.1.2.3') / \
        UDP(dport=4789) / VXLAN() / \
        Ether(dst='00:21:22:23:24:25', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='14.1.2.3', src='15.1.2.3') / \
        TCP(sport=0xABBA, dport=0xBEEF))

rpkt = bytes(Ether(dst='00:01:02:03:04:05', src='A3:21:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3') / \
        UDP(dport=4789) / VXLAN() / \
        Ether(dst='00:11:12:13:14:15', src='00:B1:B2:B3:B4:B5') / \
        IP(dst='12.1.2.3', src='13.1.2.3') / \
        UDP(dport=4789) / VXLAN() / \
        Ether(dst='00:21:22:23:24:25', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='14.1.2.3', src='15.1.2.3') / \
        TCP(sport=0xABBA, dport=0xBEEF))

dump_pkt(pkt)
dump_pkt(rpkt)

#model_wrap.zmq_connect()
#model_wrap.step_network_pkt(pkt, 1)
#(rpkt, port, cos) = model_wrap.get_next_pkt()

###############################################################################

pkt = bytes(Ether(dst='00:01:02:03:04:05', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3') / \
        UDP(dport=4789) / VXLAN() / \
        Ether(dst='00:11:12:13:14:15', src='00:B1:B2:B3:B4:B5') / \
        IP(dst='12.1.2.3', src='13.1.2.3') / \
        UDP(dport=4789) / VXLAN() / \
        Ether(dst='00:21:22:23:24:25', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='14.1.2.3', src='15.1.2.3') / \
        TCP(sport=0x1234, dport=0x5678))

# hack till checksum fields are fixed
tpkt = Ether(dst='00:21:22:23:24:25', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='14.1.2.3', src='15.1.2.3') / \
        TCP(sport=0x1234, dport=0x5678)
npkt = Ether(tpkt.build())
ipchksum = npkt.getlayer(IP).chksum
tcpchksum = npkt.getlayer(TCP).chksum

rpkt = bytes(Ether(dst='00:FE:ED:FE:ED:FE', src='00:DE:AE:AE:AE:AF') / \
        IP(dst='14.10.11.12', src='15.10.11.12', chksum=ipchksum) / \
        TCP(sport=0x1234, dport=0x5678, chksum=tcpchksum) / '\0\0\0\0\0\0')

dump_pkt(pkt)
dump_pkt(rpkt)

#model_wrap.zmq_connect()
#model_wrap.step_network_pkt(pkt, 1)
#(rpkt, port, cos) = model_wrap.get_next_pkt()

###############################################################################

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
pkt = Ether(dst='00:21:22:23:24:25', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='14.1.2.3', src='15.1.2.3') / \
        TCP(sport=0x1234, dport=0x5678) / payload

rpkt = Ether(dst='00:01:02:03:04:05', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3', id=0, chksum=0) / \
        UDP(dport=4789, chksum=0) / VXLAN() / \
        Ether(dst='00:11:12:13:14:15', src='00:B1:B2:B3:B4:B5') / \
        IP(dst='12.1.2.3', src='13.1.2.3', id=0, chksum=0) / \
        UDP(dport=4789, chksum=0) / VXLAN() / \
        Ether(dst='00:21:22:23:24:25', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='14.1.2.3', src='15.1.2.3') / \
        TCP(sport=0x1234, dport=0x5678) / payload

dump_pkt(pkt)
dump_pkt(rpkt)

#model_wrap.zmq_connect()
#model_wrap.step_network_pkt(pkt, 1)
#(rpkt, port, cos) = model_wrap.get_next_pkt()

###############################################################################

pkt = Ether(dst='00:01:02:03:04:05', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3') / \
        UDP() / BTH(opcode=4) / bytes([b for b in range(0,32)]) / ICRC()

dump_pkt(pkt)

###############################################################################

pkt = Ether(dst='00:01:02:03:04:05', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3') / \
        UDP() / VXLAN() / \
        Ether(dst='00:11:12:13:14:15', src='00:B1:B2:B3:B4:B5') / \
        IP(dst='12.1.2.3', src='13.1.2.3') / \
        UDP() / BTH(opcode=4) / bytes([b for b in range(0,32)]) / ICRC()

dump_pkt(pkt)

###############################################################################

pkt = Ether(dst='00:E1:E2:E3:E4:E5', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3') / \
        UDP() / BTH(opcode=4) / bytes([b for b in range(0,32)]) / ICRC()

dump_pkt(pkt)

###############################################################################

pkt = Ether(dst='00:F1:F2:F3:F4:F5', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3') / \
        UDP() / BTH(opcode=4) / bytes([b for b in range(0,32)]) / ICRC()

rpkt = Ether(dst='00:01:02:03:04:05', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3', id=0, chksum=0) / \
        UDP(dport=4789, chksum=0) / VXLAN() / \
        Ether(dst='00:F1:F2:F3:F4:F5', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3') / \
        UDP() / BTH(opcode=4) / bytes([b for b in range(0,32)]) / ICRC()

dump_pkt(pkt)
dump_pkt(rpkt)
