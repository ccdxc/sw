#! /usr/bin/python3
import pdb

import infra.common.objects as objects

MAC_ZERO    = '00:00:00:00:00:00'
MCAST_MAC   = '01:5e:00:00:00:01'
BCAST_MAC   = 'FF:FF:FF:FF:FF:FF'

IP_MCAST    = '239.1.1.1'
IP_BCAST    = '255.255.255.255'
IP_LOOPBACK = '127.0.0.1'
IP_EQ       = '1.2.3.4'
IP_ZERO     = '0.0.0.0'

IP6_LOOPBACK    = '::1'
IP6_MCAST       = 'FF01::1'
IP6_EQ          = '5555::1'
IP6_ZERO        = '::'

MFFS = {
    'smac::SMAC_ZERO'               : MAC_ZERO,
    'smac::SMAC_MCAST'              : MCAST_MAC,
    'smac::SMAC_BCAST'              : BCAST_MAC,
    'smac::SMAC_EQ_DMAC'            : 'ref://testcase/config/src/endpoint/macaddr',

    'outersmac::OUTER_SMAC_ZERO'    : MAC_ZERO,
    'outersmac::OUTER_SMAC_MCAST'   : MCAST_MAC,
    'outersmac::OUTER_SMAC_BCAST'   : BCAST_MAC,
    'outersmac::OUTER_SMAC_EQ_DMAC' : 'ref://testcase/config/src/endpoint/intf/rmacaddr',

    'dmac::DMAC_ZERO'               : MAC_ZERO,
    'dmac::SMAC_EQ_DMAC'            : 'ref://testcase/config/src/endpoint/macaddr',

    'outerdmac::OUTER_DMAC_ZERO'    : MAC_ZERO,
    'outerdmac::OUTER_SMAC_EQ_DMAC' : 'ref://testcase/config/src/endpoint/intf/rmacaddr',

    'sip::SIP_MCAST_V4'             : IP_MCAST,
    'sip::SIP_BCAST_V4'             : IP_BCAST,
    'sip::SIP_LOOPBACK_V4'          : IP_LOOPBACK,
    'sip::SIP_EQ_DIP_V4'            : IP_EQ,

    'outersip::OUTER_SIP_MCAST_V4'      : IP_MCAST,
    'outersip::OUTER_SIP_BCAST_V4'      : IP_BCAST,
    'outersip::OUTER_SIP_LOOPBACK_V4'   : IP_LOOPBACK,
    'outersip::OUTER_SIP_EQ_DIP_V4'     : IP_EQ,

    'dip::DIP_LOOPBACK_V4'          : IP_LOOPBACK,
    'dip::SIP_EQ_DIP_V4'            : IP_EQ,
    'dip::DIP_ZERO_V4'              : IP_ZERO,

    'outerdip::OUTER_DIP_LOOPBACK_V4'   : IP_LOOPBACK,
    'outerdip::OUTER_SIP_EQ_DIP_V4'     : IP_EQ,

    'sip6::SIP_MCAST_V6'            : IP6_MCAST,
    'sip6::SIP_LOOPBACK_V6'         : IP6_LOOPBACK,
    'sip6::SIP_EQ_DIP_V6'           : IP6_EQ,

    'dip6::DIP_LOOPBACK_V6'         : IP6_LOOPBACK,
    'dip6::SIP_EQ_DIP_V6'           : IP6_EQ,
    'dip6::DIP_ZERO_V6'             : IP6_ZERO,

    'ttl::TTL_ZERO_V4'              : 0,
    'ttl::TTL_ZERO_V6'              : 0,
    
    'outerttl::OUTER_TTL_ZERO_V4'   : 0,

    'ver::IPV4_BAD_VERSION'         : 5,
    'ver::IPV6_BAD_VERSION'         : 5,
    
    'outerver::OUTER_IPV4_BAD_VERSION'  : 5,
}

def __get_lookup_label(tc, prefix):
    return '%s::%s' % (prefix, tc.module.name)

def __get_mff(tc, prefix, default):
    lkplabel = __get_lookup_label(tc, prefix)
    if lkplabel in MFFS:
        val = MFFS[lkplabel]
    else:
        val = default
    
    if type(val) is not str:
        return val

    if objects.IsTemplateFieldString(val):
        obj = objects.TemplateFieldObject(val)
        if objects.IsReference(obj):
            return obj.Get(tc)
        else:
            assert(0)
    return val

def GetMfSip6(tc, pkt):
    return __get_mff(tc, 'sip6', tc.config.flow.sip)

def GetMfDip6(tc, pkt):
    return __get_mff(tc, 'dip6', tc.config.flow.dip)

def GetMfTtl(tc, pkt):
    return __get_mff(tc, 'ttl', 64)

def GetMfOuterTtl(tc, pkt):
    return __get_mff(tc, 'outerttl', 64)

def GetMfIpv4Version(tc, pkt):
    return __get_mff(tc, 'ver', 4)

def GetMfOuterIpv4Version(tc, pkt):
    return __get_mff(tc, 'outerver', 4)

def GetMfIpv6Version(tc, pkt):
    return __get_mff(tc, 'ver', 6)

def GetMfOuterSmac(tc, pkt):
    return __get_mff(tc, 'outersmac', tc.config.src.endpoint.intf.rmacaddr)

def GetMfSmac(tc, pkt):
    return __get_mff(tc, 'smac', tc.config.src.endpoint.macaddr)

def GetMfDmac(tc, pkt):
    return __get_mff(tc, 'dmac', tc.config.dst.endpoint.macaddr)

def GetMfOuterDmac(tc, pkt):
    return __get_mff(tc, 'outerdmac', tc.config.src.endpoint.intf.macaddr)

def GetMfSip(tc, pkt):
    return __get_mff(tc, 'sip', tc.config.flow.sip)

def GetMfOuterSip(tc, pkt):
    return __get_mff(tc, 'outersip', tc.config.src.endpoint.intf.rtep)

def GetMfDip(tc, pkt):
    return __get_mff(tc, 'dip', tc.config.flow.dip)

def GetMfOuterDip(tc, pkt):
    return __get_mff(tc, 'outerdip', tc.config.src.endpoint.intf.ltep)

