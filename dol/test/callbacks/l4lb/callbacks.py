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

def GetL4LbFlowDestPorts(testcase):
    if testcase.config.flow.IsDnat():
        return testcase.config.dst.l4lb.backend.ep.intf.ports
    return testcase.config.dst.endpoint.intf.ports

def GetL4LbFlowSrcPorts(testcase):
    if testcase.config.flow.IsSnat():
        return testcase.config.src.l4lb.backend.ep.intf.ports
    return testcase.config.src.endpoint.intf.ports
