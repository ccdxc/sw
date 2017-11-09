#! /usr/bin/python3

import pdb
import infra.penscapy.penscapy as penscapy

def GetUdpLength(tc, pkt):
    udpindex = pkt.hdrsorder.index('udp')
    rem_hdrs = pkt.hdrsorder[udpindex:]
       
    udplen = 0
    for hdr in rem_hdrs:
        if hdr == 'udp_options':
            break
        elif hdr == 'payload':
            udplen += pkt.GetPayloadSize()
        else:
            hdrobj = getattr(pkt.headers, hdr, None)
            assert(hdrobj)
            udplen += hdrobj.meta.size
    return udplen
