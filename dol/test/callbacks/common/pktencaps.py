#! /usr/bin/python3

import pdb

def AddPacketEncapQtag(pkt, encap):
    if pkt.headers_order[0] == 'outereth':
        pkt.headers.qtag.fields.type = pkt.headers.outereth.fields.type
        pkt.headers.outereth.fields.type = encap.headers.eth.fields.type
    elif  pkt.headers_order[0] == 'eth':
        pkt.headers.qtag.fields.type = pkt.headers.eth.fields.type
        pkt.headers.eth.fields.type = encap.headers.eth.fields.type
    pkt.headers_order.insert(1, 'qtag')
    return

def AddPacketEncapQinQ(tc, pkt):
    return

def AddPacketEncapVxlan(pkt, encap):
    pkt.headers_order = ['outereth', 'outeripv4', 'outerudp', 'vxlan'] + pkt.headers_order
    return
