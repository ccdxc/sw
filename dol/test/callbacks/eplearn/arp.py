#! /usr/bin/python3
import pdb
import infra.api.api as infra_api


def IsArpRequest(val):
    return 'ARP_REQ' in val
def IsArpResponse(val):
    return 'ARP_RESP' in val
def IsRarpRequest(val):
    return 'RARP_REQ' in val
def IsRarpResponse(val):
    return 'RARP_RESP' in val

def GetArpEtherType(testcase, packet):
    return 0x806

def IsPriorityTagged(pvtdata):
    if 'priotag' in pvtdata.__dict__:
        return pvtdata.priotag
    return False

def GetArpOpcode(testcase, packet):
    val = testcase.module.iterator.Get()
    if IsRarpRequest(val.type): return 0x3
    elif IsRarpResponse(val.type): return 0x4
    elif IsArpRequest(val.type): return 0x1
    elif IsArpResponse(val.type): return 0x2
    assert(0)
    return 0x0
def GetArpSourceMAC(testcase, packet):
    #return testcase.config.src.endpoint.macaddr
    return testcase.config.root.macaddr
def GetArpSourceIP(testcase, packet):
    return testcase.config.root.GetIpAddress()
def GetArpDestMAC(testcase, packet):
    return "FF:FF:FF:FF:FF:FF"
def GetArpDestIP(testcase, packet):
    return "1.1.1.1"

def GetExpectDelay(testcase):
    return 10

def GetPacketEncaps(testcase, packet):
    encaps = []
    if testcase.config.root.segment.native == False:
        encaps.append(infra_api.GetPacketTemplate('ENCAP_QTAG'))

    if testcase.config.root.segment.IsFabEncapVxlan():
        encaps.append(infra_api.GetPacketTemplate('ENCAP_VXLAN'))

    return encaps
