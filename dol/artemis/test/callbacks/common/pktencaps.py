#! /usr/bin/python3

import pdb

def AddPacketEncapQtag(pkt, encap):
    if pkt.hdrsorder[0] == 'outereth':
        pkt.headers.qtag.fields.type = pkt.headers.outereth.fields.type
        pkt.headers.outereth.fields.type = encap.headers.eth.fields.type
    elif  pkt.hdrsorder[0] == 'eth':
        pkt.headers.qtag.fields.type = pkt.headers.eth.fields.type
        pkt.headers.eth.fields.type = encap.headers.eth.fields.type
    pkt.hdrsorder.insert(1, 'qtag')
    return

def AddPacketEncapQinQ(pkt, encap):
    if pkt.hdrsorder[0] == 'outereth':
        pkt.headers.qtag.fields.type = pkt.headers.outereth.fields.type
        pkt.headers.outereth.fields.type = encap.headers.eth.fields.type
    elif  pkt.hdrsorder[0] == 'eth':
        pkt.headers.qtag.fields.type = pkt.headers.eth.fields.type
        pkt.headers.eth.fields.type = encap.headers.eth.fields.type
    pkt.hdrsorder.insert(1, 'qtag')
    pkt.hdrsorder.insert(1, 'outerqtag')
    return

def AddPacketEncapVxlan(pkt, encap):
    if pkt.hdrsorder[1] == 'qtag':
        pkt.headers.eth.fields.type = pkt.headers.qtag.fields.type
        pkt.hdrsorder.pop(1)
    if encap.meta.id == 'ENCAP_VXLAN':
        pkt.hdrsorder = ['outereth', 'outeripv4', 'outerudp', 'vxlan'] + pkt.hdrsorder
    elif encap.meta.id == 'ENCAP_VXLAN_IPV6':
        pkt.hdrsorder = ['outereth', 'outeripv6', 'outerudp', 'vxlan'] + pkt.hdrsorder
    elif encap.meta.id == 'ENCAP_VXLAN2':
        pkt.hdrsorder = ['outereth_1', 'outeripv4_1', 'outerudp_1', 'outervxlan', 'outereth', 'outeripv4', 'outerudp', 'vxlan'] + pkt.hdrsorder
    else:
        assert 0
    return

def AddPacketEncapErspan(pkt, encap):
    pkt.hdrsorder = ['erspaneth', 'erspanqtag', 'erspanipv4', 'erspangre', 'erspan'] + pkt.hdrsorder
    return

def AddPacketEncapGre(pkt, encap):
    pkt.hdrsorder = ['outereth', 'outeripv4', 'outergre'] + pkt.hdrsorder
    return
