# /usr/bin/python3
import pdb
from infra.api.objects import PacketHeader
import infra.api.api as infra_api

def GetL4LbFlowSmac(testcase, packet):
    if testcase.config.flow.IsSnat():
        return testcase.config.src.l4lb_backend.ep.macaddr
    return testcase.config.src.endpoint.macaddr

def GetL4LbFlowDmac(testcase, packet):
    if testcase.config.flow.IsDnat():
        return testcase.config.dst.l4lb_service.macaddr
    return testcase.config.dst.endpoint.macaddr

def GetL4LbFlowDestPorts(testcase):
    if testcase.config.flow.IsDnat():
        return testcase.config.dst.l4lb_backend.ep.intf.ports
    return testcase.config.dst.endpoint.intf.ports

def GetL4LbFlowSrcPorts(testcase):
    if testcase.config.flow.IsSnat():
        return testcase.config.src.l4lb_backend.ep.intf.ports
    return testcase.config.src.endpoint.intf.ports
