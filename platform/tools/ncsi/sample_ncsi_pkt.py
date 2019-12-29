#!/usr/bin/python

from scapy.all import *

def getmac(interface):
    try:
        mac = open('/sys/class/net/'+interface+'/address').readline()
    except:
      mac = "00:00:00:00:00:00"

    return mac[0:17]


#Create NCSI class
class Ncsi(Packet):
    name = "NcsiPacket "
    fields_desc=[ XByteField("mc_id",0),
        XByteField("header_rev",1),
        XByteField("rsvd0",0),
        XByteField("inst_id",0),
        XByteField("ctrlpkt_type",0),
        XByteField("chan_id",0),
        BitField("payload_len",0,12),
        BitField("rsvd1",0,4),
        IntField("rsvd2",0),
        IntField("rsvd3",0) ]

bind_layers(Ether, Ncsi, type=0x88F8)

n=Ncsi(ctrlpkt_type=0x0B)
#n=Ncsi(mc_id=0, inst_id=3, ctrlpkt_type=0x32, chan_id=2, payload_len=64)/Raw(b'x0'*64)
#eth = Ether(dst='ff:ff:ff:ff:ff:ff', src=getmac("enp183s0"), type=0x88F8)
eth = Ether(dst='ff:ff:ff:ff:ff:ff', src='00:ff:ff:ff:ff:ff', type=0x88F8)

pkt=eth/n/"abcdefghijklmnop"

if len(pkt) < 60:
    pad_len = 60 - len(pkt)
    pad = Padding()
    pad.load = '\x00'*pad_len
    pkt = pkt/pad
pkt.show2()
sendp(pkt, iface="enp183s0")

