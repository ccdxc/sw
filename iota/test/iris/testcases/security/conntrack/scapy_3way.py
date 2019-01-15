#! /usr/bin/python3

from scapy.all import *

ip=IP(src="192.168.100.102", dst="192.168.100.101")
syn_packet = TCP(sport=52255, dport=1237, flags="S", seq=100, options=[('MSS',689),('WScale',1)])
synack_packet = sr1(ip/syn_packet)
my_ack = synack_packet.seq+1
ack_packet = TCP(sport=52255, dport=1237, flags="A", seq=101, ack=my_ack)
send(ip/ack_packet)
