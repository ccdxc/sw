# /usr/bin/python3
import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api

def __get_acl_from_tc(tc):
    return tc.pvtdata.acl

def GetMacSA(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnMacSA():
        return acl.MatchMacSA()

    return tc.config.src.endpoint.macaddr

def GetMacDA(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnMacDA():
        return acl.MatchMacDA()

    return tc.config.dst.endpoint.macaddr

def GetEtherType(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnEtherType():
        return acl.MatchEtherType()

    return None

def GetIPv4SIP(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnIPv4SIP():
        return acl.MatchIPv4SIP()

    return tc.config.flow.sip

def GetIPv4DIP(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnIPv4DIP():
        return acl.MatchIPv4DIP()

    return tc.config.flow.dip 

def GetIPv6SIP(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnIPv6SIP():
        return acl.MatchIPv6SIP()

    return tc.config.flow.sip

def GetIPv6DIP(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnIPv6DIP():
        return acl.MatchIPv6DIP()

    return tc.config.flow.dip 

def GetProto(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnProto():
        return acl.MatchProto()

    if tc.config.flow.IsIPV4():
        return tc.config.flow.proto.lower()
    else:
        return tc.config.flow.proto.upper()

def GetUDPSport(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnUDPSport():
        return acl.MatchUDPSport()

    return tc.config.flow.sport

def GetUDPDport(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnUDPDport():
        return acl.MatchUDPDport()

    return tc.config.flow.dport

def GetTCPSport(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnTCPSport():
        return acl.MatchTCPSport()

    return tc.config.flow.sport

def GetTCPDport(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnTCPDport():
        return acl.MatchTCPDport()

    return tc.config.flow.dport

def GetICMPCode(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnICMPCode():
        return acl.MatchICMPCode()

    return tc.config.flow.icmpcode

def GetICMPType(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnICMPType():
        return acl.MatchICMPType()

    return tc.config.flow.icmptype
