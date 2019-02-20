#! /usr/bin/python3
from scapy.all import *
from scapy.utils import rdpcap
import sys

pkts = rdpcap(sys.argv[1])
for pkt in pkts:
    sendp(pkt, iface="eth1")
