# /usr/bin/python3
import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api
import test.callbacks.networking.packets as networking
import infra.penscapy.penscapy as penscapy

def __get_acl_from_tc(tc):
    return tc.pvtdata.acl

def GetPacketTemplate(testcase, packet):
    acl = __get_acl_from_tc(testcase)
    if acl.MatchOnProto():
        if testcase.config.flow.IsIPV4():
            template = 'ETH_IPV4_PROTO_200' 
        else:
            template = 'ETH_IPV6_PROTO_190' 
        return infra_api.GetPacketTemplate(template)
    return networking.GetPacketTemplateByFlow(testcase, packet)

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

    return tc.config.flow.ethertype

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

def GetTCPFlags(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnTCPFlags():
        return acl.MatchTCPFlags()

    return None

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

def GetOuterDIP(tc,packet):
    acl = __get_acl_from_tc(tc)
    
    if acl.MatchOnTEPMiss():
        return acl.MatchTEPMissDIP()

    return tc.config.src.endpoint.intf.ltep

def GetIpv4Options(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnIpOptions():
        return penscapy.IPOption_Traceroute()
    return None

def GetIpv6ExtHeaders(tc, packet):
    acl = __get_acl_from_tc(tc)

    if acl.MatchOnIpOptions():
        return penscapy.IPv6ExtHdrHopByHop()
    return None
