#! /usr/bin/python3

def IsArpRequest(val):
    return 'ARP_REQ' in val
def IsArpResponse(val):
    return 'ARP_RESP' in val
def IsRarpRequest(val):
    return 'RARP_REQ' in val
def IsRarpResponse(val):
    return 'RARP_RESP' in val

def GetArpOpcode(testcase, packet):
    val = testcase.module.iterator.Get()
    if IsRarpRequest(val): return 0x3
    elif IsRarpResponse(val): return 0x4
    elif IsArpRequest(val): return 0x1
    elif IsArpResponse(val): return 0x2
    assert(0)
    return 0x0
def GetArpSourceMAC(testcase, packet):
    return testcase.config.src.endpoint.macaddr
def GetArpSourceIP(testcase, packet):
    return testcase.config.src.endpoint.GetIpAddress()
def GetArpDestMAC(testcase, packet):
    return testcase.config.dst.endpoint.macaddr
def GetArpDestIP(testcase, packet):
    return testcase.config.dst.endpoint.GetIpAddress()
