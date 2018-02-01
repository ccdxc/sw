#!/usr/bin/python

import sys
sys.path.insert(0, 'model_sim/src')
import model_wrap
from scapy.layers.all import *

pkt = bytes(Ether(dst='00:01:02:03:04:05', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3') / \
        UDP(dport=4789) / \
        GRE() / GRE() / \
        Ether(dst='00:11:12:13:14:15', src='00:B1:B2:B3:B4:B5') / \
        IP(dst='12.1.2.3', src='13.1.2.3') / \
        UDP(dport=4789) / \
        GRE() / GRE() / \
        Ether(dst='00:21:22:23:24:25', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='14.1.2.3', src='15.1.2.3') / \
        TCP(sport=0xABBA, dport=0xBEEF))

rpkt = bytes(Ether(dst='00:01:02:03:04:05', src='A3:21:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3') / \
        UDP(dport=4789) / \
        GRE() / GRE() / \
        Ether(dst='00:11:12:13:14:15', src='00:B1:B2:B3:B4:B5') / \
        IP(dst='12.1.2.3', src='13.1.2.3') / \
        UDP(dport=4789) / \
        GRE() / GRE() / \
        Ether(dst='00:21:22:23:24:25', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='14.1.2.3', src='15.1.2.3') / \
        TCP(sport=0xABBA, dport=0xBEEF))

print map("0x{:02X}".format, [ord(b) for b in bytes(pkt)])
print map("0x{:02X}".format, [ord(b) for b in bytes(rpkt)])

#model_wrap.zmq_connect()
#model_wrap.step_network_pkt(pkt, 1)
#(rpkt, port, cos) = model_wrap.get_next_pkt()

pkt = bytes(Ether(dst='00:01:02:03:04:05', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3') / \
        UDP(dport=4789) / \
        GRE() / GRE() / \
        Ether(dst='00:11:12:13:14:15', src='00:B1:B2:B3:B4:B5') / \
        IP(dst='12.1.2.3', src='13.1.2.3') / \
        UDP(dport=4789) / \
        GRE() / GRE() / \
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
        TCP(sport=0x1234, dport=0x5678, chksum=tcpchksum))

#model_wrap.zmq_connect()
#model_wrap.step_network_pkt(pkt, 1)
#(rpkt, port, cos) = model_wrap.get_next_pkt()

print map("0x{:02X}".format, [ord(b) for b in bytes(pkt)])
print map("0x{:02X}".format, [ord(b) for b in bytes(rpkt)])

payload = 'abcdefghijlkmnopqrstuvwzxyabcdefghijlkmnopqrstuvwzxy'
pkt = Ether(dst='00:21:22:23:24:25', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='14.1.2.3', src='15.1.2.3') / \
        TCP(sport=0x1234, dport=0x5678) / payload

rpkt = Ether(dst='00:01:02:03:04:05', src='00:A1:A2:A3:A4:A5') / \
        IP(dst='10.1.2.3', src='11.1.2.3', id=0, chksum=0) / \
        UDP(dport=4789, chksum=0) / \
        GRE(strict_route_source=1) / GRE(proto=0) / \
        Ether(dst='00:11:12:13:14:15', src='00:B1:B2:B3:B4:B5') / \
        IP(dst='12.1.2.3', src='13.1.2.3', id=0, chksum=0) / \
        UDP(dport=4789, chksum=0) / \
        GRE(strict_route_source=1) / GRE(proto=0) / \
        Ether(dst='00:21:22:23:24:25', src='00:C1:C2:C3:C4:C5') / \
        IP(dst='14.1.2.3', src='15.1.2.3') / \
        TCP(sport=0x1234, dport=0x5678) / payload

#model_wrap.zmq_connect()
#model_wrap.step_network_pkt(pkt, 1)
#(rpkt, port, cos) = model_wrap.get_next_pkt()

print map("0x{:02X}".format, [ord(b) for b in bytes(pkt)])
print map("0x{:02X}".format, [ord(b) for b in bytes(rpkt)])
