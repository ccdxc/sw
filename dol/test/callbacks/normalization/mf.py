#! /usr/bin/python3
import pdb

MAC_ZERO    = '00:00:00:00:00:00'
MCAST_MAC   = '01:5e:00:00:00:01'
BCAST_MAC   = 'FF:FF:FF:FF:FF:FF'
MAC_EQ      = '00:00:00:00:00:99'

IP_MCAST    = '239.1.1.1'
IP_BCAST    = '255.255.255.255'
IP_LOOPBACK = '127.0.0.1'
IP_EQ       = '1.2.3.4'

IP6_LOOPBACK    = '::1'
IP6_MCAST       = 'FF01::1'
IP6_EQ          = '5555::1'

def GetMfSmac(tc, pkt):
    if tc.module.name == 'SMAC_ZERO':
        return MAC_ZERO
    elif tc.module.name == 'SMAC_MCAST':
        return MCAST_MAC
    elif tc.module.name == 'SMAC_BCAST':
        return BCAST_MAC
    elif tc.module.name == 'SMAC_EQ_DMAC':
        return MAC_EQ
    return tc.config.src.endpoint.macaddr

def GetMfDmac(tc, pkt):
    if tc.module.name == 'DMAC_ZERO':
        return MAC_ZERO
    elif tc.module.name == 'SMAC_EQ_DMAC':
        return MAC_EQ
    return tc.config.dst.endpoint.macaddr

def GetMfSip(tc, pkt):
    if tc.module.name == 'SIP_MCAST_V4':
        return IP_MCAST
    elif tc.module.name == 'SIP_BCAST_V4':
        return IP_BCAST
    elif tc.module.name == 'SIP_LOOPBACK_V4':
        return IP_LOOPBACK
    elif tc.module.name == 'SIP_EQ_DIP_V4':
        return IP_EQ
    return tc.config.flow.sip

def GetMfDip(tc, pkt):
    if tc.module.name == 'DIP_LOOPBACK_V4':
        return IP_LOOPBACK
    elif tc.module.name == 'SIP_EQ_DIP_V4':
        return IP_EQ
    return tc.config.flow.dip

def GetMfSip6(tc, pkt):
    if tc.module.name == 'SIP_MCAST_V6':
        return IP6_MCAST
    elif tc.module.name == 'SIP_LOOPBACK_V6':
        return IP6_LOOPBACK
    elif tc.module.name == 'SIP_EQ_DIP_V6':
        return IP6_EQ
    return tc.config.flow.sip

def GetMfDip6(tc, pkt):
    if tc.module.name == 'DIP_LOOPBACK_V6':
        return IP6_LOOPBACK
    elif tc.module.name == 'SIP_EQ_DIP_V6':
        return IP6_EQ
    return tc.config.flow.dip

def GetMfTtl(tc, pkt):
    if tc.module.name == 'TTL_ZERO_V4' or\
       tc.module.name == 'TTL_ZERO_V6':
        return 0
    return 64

def GetMfIpv4Version(tc, pkt):
    if tc.module.name == 'IPV4_BAD_VERSION':
        return 5
    return 4

def GetMfIpv6Version(tc, pkt):
    if tc.module.name == 'IPV6_BAD_VERSION':
        return 5
    return 4
