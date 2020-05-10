#! /usr/bin/env python 
import sys
import time
import argparse
from scapy.all import *
from scapy.utils import rdpcap

parser = argparse.ArgumentParser(description='Packet Scapy Utility to send packets from Host/Workload.')

parser.add_argument('--pcap-file', dest='pcap_file', required = True,
                    help='Packet PCAP file.')
parser.add_argument('--iface', dest='iface', required = True,
                    help='Interface to send the packet.')
parser.add_argument('--count', dest='count', type=int,
                    default=1, help='Packet send count.')
parser.add_argument('--interval', dest='interval', type=float,
                    default=0.01, help='Packet interval.')
args = parser.parse_args()

if args.count >= 100:
    args.interval = 0.001

pkts = rdpcap(args.pcap_file)
for itr in range(args.count):
    for pkt in pkts:
        sendp(pkt, inter=0.001, loop=False, iface=args.iface)
    time.sleep(args.interval)
