#! /usr/bin/python3
import pdb
from functools import reduce 
import infra.api.api as infra_api
import infra.penscapy.penscapy as penscapy


def GetDhcpSrcIp(testcase, packet):
    return "0.0.0.0"

def GetDhcpXid(testcase, packet):
    return testcase.config.root.ipaddrs[0].getnum()

def GetDhcpDstIp(testcase, packet):
    return "255.255.255.255"

def GetDhcpDestMAC(testcase, packet):
    return "FF:FF:FF:FF:FF:FF"

def GetDhcpChaddr(testcase, packet):
    return  reduce(lambda x,y: x + y , [bytes.fromhex(x) for x in testcase.config.root.macaddr.get().split(":")]) 
    
def GetDhcpOptions(testcase, packet):
    if "DISCOVER" in packet.GID():
        dhcp_options = [("message-type","discover"),
                         ("client_id", GetDhcpChaddr(testcase, packet)),
                         "end"
                         ]
    elif "OFFER" in packet.GID():
        dhcp_options = [("message-type","offer"),
                        ("subnet_mask", "255.255.255.0"),
                        ("renewal_time", 2000),
                        ("rebinding_time", 2000),
                        ("lease_time", 2000),
                        ("server_id", testcase.pvtdata.dhcp_server.ipaddrs[0].get()),
                         "end"
                         ]
    elif "REQUEST" in packet.GID():
        dhcp_options = [("message-type","request"),
                        ("server_id", testcase.pvtdata.dhcp_server.ipaddrs[0].get()),
                         "end"
                         ]
    elif "ACK" in packet.GID():
        dhcp_options = [("message-type","ack"),
                        ("subnet_mask", "255.255.255.0"),
                        ("renewal_time", 2000),
                        ("rebinding_time", 2000),
                        ("lease_time", 2000),
                        ("server_id", testcase.pvtdata.dhcp_server.ipaddrs[0].get()),
                         "end"
                         ]        
    return penscapy.DHCP(options=dhcp_options)


def GetDhcpSname(testcase, packet):
    return bytes(bytearray(64))

def GetDhcpFile(testcase, packet):
    return bytes(bytearray(64))

def GetExpectDelay(testcase):
    if testcase.module.iterator.Get().type in ["DISCOVER", "REQUEST"]:
        #Delay more for broadcast packet.
        return 10
    else:
        return 2

def GetPacketEncaps(testcase, packet):
    encaps = []
    if testcase.config.root.segment.native == False:
        encaps.append(infra_api.GetPacketTemplate('ENCAP_QTAG'))

    if testcase.config.root.segment.IsFabEncapVxlan():
        encaps.append(infra_api.GetPacketTemplate('ENCAP_VXLAN'))

    return encaps

def GetRootIP(testcase, packet):
    return testcase.config.root.GetIpAddress()

def GetDhcpServerVlan(testcase, packet):
    return testcase.pvtdata.dhcp_server.segment.vlan_id

def GetDhcpServerMAC(testcase, packet):
    return testcase.pvtdata.dhcp_server.macaddr.get() 

def GetDhcpServerIp(testcase, packet):
    return testcase.pvtdata.dhcp_server.ipaddrs[0].get() 
    
def GetDhcpServerPort(testcase, args = None):
    if testcase.config.root.tenant.IsHostPinned():
        return testcase.config.root.pinintf.ports
    return testcase.pvtdata.dhcp_server.intf.ports

def GetDhcpSeverPacketEncaps(testcase, packet):
    encaps = []
    if testcase.pvtdata.dhcp_server.segment.native == False:
        encaps.append(infra_api.GetPacketTemplate('ENCAP_QTAG'))

    if testcase.pvtdata.dhcp_server.segment.IsFabEncapVxlan():
        encaps.append(infra_api.GetPacketTemplate('ENCAP_VXLAN'))

    return encaps

