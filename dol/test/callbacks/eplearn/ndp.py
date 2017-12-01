#! /usr/bin/python3
import pdb
import infra.api.api as infra_api

def GetNdpSourceMAC(testcase, packet):
    #return testcase.config.src.endpoint.macaddr
    return testcase.config.root.macaddr
def GetNdpSourceIP(testcase, packet):
    return testcase.config.root.GetIpv6Address()
def GetNdpDestMAC(testcase, packet):
    return "FF:FF:FF:FF:FF:FF"
def GetNdpDestIP(testcase, packet):
    return "fe80::c002:3ff:fee4:0"

def GetExpectDelay(testcase):
    return 10

def GetPacketEncaps(testcase, packet):
    encaps = []
    if testcase.config.root.segment.native == False:
        encaps.append(infra_api.GetPacketTemplate('ENCAP_QTAG'))

    if testcase.config.root.segment.IsFabEncapVxlan():
        encaps.append(infra_api.GetPacketTemplate('ENCAP_VXLAN'))

    return encaps
