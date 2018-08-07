# /usr/bin/python3
import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api

def GetL2FlowL4LbPreNatSmac(testcase, packet):
    if testcase.config.flow.IsSnat():
        return testcase.config.src.l4lb.backend.ep.macaddr
    return testcase.config.src.endpoint.macaddr

def GetL2FlowL4LbPreNatDmac(testcase, packet):
    if testcase.config.flow.IsDnat():
        return testcase.config.dst.l4lb.service.macaddr
    return testcase.config.dst.endpoint.macaddr

def GetL2FlowL4LbPostNatSmac(testcase, packet):
    if testcase.config.flow.IsSnat():
        return testcase.config.src.l4lb.service.ep.macaddr
    # Twice-NAT and rflow
    if testcase.config.flow.IsTwiceNAT() and not testcase.config.flow.IsIflow():
        return testcase.config.src.l4lb.service.ep.macaddr
    return testcase.config.src.endpoint.macaddr

def GetL2FlowL4LbPostNatDmac(testcase, packet):
    if testcase.config.flow.IsDnat():
        return testcase.config.dst.l4lb.backend.ep.macaddr
    return testcase.config.dst.endpoint.macaddr

def GetL3FlowL4LbPreNatSmac(testcase, packet):
    if testcase.config.flow.IsSnat():
        return testcase.config.src.l4lb.backend.ep.macaddr
    return testcase.config.src.endpoint.macaddr

def GetL3FlowL4LbPreNatDmac(testcase, packet):
    return testcase.config.src.segment.macaddr

def GetL3FlowL4LbPostNatSmac(testcase, packet):
    return testcase.config.dst.segment.macaddr

def GetL3FlowL4LbPostNatDmac(testcase, packet):
    if testcase.config.flow.IsDnat():
        return testcase.config.dst.l4lb.backend.ep.macaddr
    return testcase.config.dst.endpoint.macaddr

def GetPostNatDIP(testcase, packet):
    if testcase.config.flow.IsDnat() and\
        testcase.config.dst.l4lb.IsNATDSR():
            return testcase.config.flow.dip
    return testcase.config.flow.nat_dip

def GetL4LbFlowDestPorts(testcase):
    if testcase.config.flow.IsDnat():
        return testcase.config.dst.l4lb.backend.ep.intf.ports
    return testcase.config.dst.endpoint.intf.ports

def GetL4LbFlowSrcPorts(testcase):
    if testcase.config.flow.IsSnat():
        return testcase.config.src.l4lb.backend.ep.intf.ports
    return testcase.config.src.endpoint.intf.ports

def GetL4LbFlowPostNatL4SPort(testcase, packet):
    if testcase.config.flow.nat_sport == 0:
        # IP Only NAT
        return testcase.config.flow.sport
    else:
        return testcase.config.flow.nat_sport

def GetL4LbFlowPostNatL4DPort(testcase, packet):
    if testcase.config.flow.nat_dport == 0:
        # IP Only NAT
        return testcase.config.flow.dport
    else:
        return testcase.config.flow.nat_dport
