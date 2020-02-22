#! /usr/bin/python3
import pdb
from functools import reduce 
import infra.api.api as infra_api
import infra.penscapy.penscapy as penscapy
import struct

def GetDhcpClientSrcIp(testcase, packet):
    return "0.0.0.0"

def GetDhcpXid(testcase, packet):
    return 10

def GetDhcpBroadcastIp(testcase, packet):
    return "255.255.255.255"

def GetDhcpBroadcastMAC(testcase, packet):
    return "FF:FF:FF:FF:FF:FF"

def GetDhcpChaddr(testcase, packet):
    chaddr = "aa:bb:cc:dd:ee:ff"
    return  reduce(lambda x,y: x + y , [bytes.fromhex(x) for x in chaddr.split(":")]) 

def GetAssignedIp(testcase, packet):
    return "10.1.1.2"

def GetDhcpSubnetIp(testcase, packet):
    #fetch it from vnic and send
    return "10.1.1.1"

def GetDhcpOptions(testcase, packet):
    #get bd vnid, subnet prefix and subnet ip, vpc vnid ready
    subnet_prefix = "1.1.1.0"
    subnet_ip = "1.1.1.1"
    bd_vni = 0xffffff
    vpc_vni = 0xabcdef

    #option 82 contains multiple suboptions to be filled each of which contains
    # byte 0 -- suboption number
    # byte 1 -- suboption length
    # byte 2+ -- suboption data then repeat until all suboptions done

    option82 = struct.pack('BB', 1, 5);  
    # TODO suboption 1 data 3 byte VNID and 2 byte vnicId  - total 5
    # get this vnid from subnet and fill 2 bytes from lifid (where to get this from ?)
    bd_vni = 0xffff01 #fill this with true vnid
    option82 += struct.pack('BBB', 1, 2, 3)
    vnic = testcase.config.localmapping.VNIC
    vnic_id = 3
    option82 += struct.pack('h', vnic_id)

    #suboption 5  subnet prefix
    option82 += struct.pack('BB', 5, 4)
    #option82 += subnet_prefix
    option82 += struct.pack('I', 0x01010100)

    #suboption 11 subnet ip
    option82 += struct.pack('BB', 11, 4)
    #option82 += subnet_ip
    option82 += struct.pack('I', 0x01010101)

    # suboption 151 VSS type and vrf info 4 bytes and 4 bytes padding
    option82 += struct.pack('BB', 151, 8)
    option82 += struct.pack('B', 1)
    vpc_vni = 0xffff02
    #fill actual vni TODO
    option82 += struct.pack('BBB', 1, 2, 4 )
    option82 += struct.pack('I', 0)

    #suboption 152 
    option82 += struct.pack('BB', 152, 0)

    # TODO add end option
    option82 += struct.pack('B', 0xff)

    if "DISCOVER" in packet.GID():
        if "HOST" in packet.GID():
            dhcp_options = [("message-type","discover"),
                            ("client_id", GetDhcpChaddr(testcase, packet)),
                            "end"
                            ]
        elif "SERVER" in packet.GID():
            dhcp_options = [("message-type","discover"),
                            ("client_id", GetDhcpChaddr(testcase, packet)),
                            ("relay_agent_Information", option82),
                            "end"
                            ]
    elif "OFFER" in packet.GID():
        dhcp_options = [("message-type","offer"),
                        ("subnet_mask", "255.255.255.0"),
                        ("renewal_time", 2000),
                        ("rebinding_time", 2000),
                        ("lease_time", 2000),
                        ("server_id", "1.1.1.1"),
                        ("relay_agent_Information", option82),
                         "end"
                         ]
    elif "REQUEST" in packet.GID():
        if "HOST" in packet.GID():
            dhcp_options = [("message-type","request"),
                            ("server_id", "1.1.1.1"),
                            ("requested_addr", "10.1.1.2"),
                             "end"
                            ]
        elif "SERVER" in packet.GID():
            dhcp_options = [("message-type","request"),
                            ("server_id", "1.1.1.1"),
                            ("requested_addr", "10.1.1.2"),
                            ("relay_agent_Information", option82),
                             "end"
                            ]
    elif "ACK" in packet.GID():
        dhcp_options = [("message-type","ack"),
                        ("subnet_mask", "255.255.255.0"),
                        ("renewal_time", 2000),
                        ("rebinding_time", 2000),
                        ("lease_time", 2000),
                        ("server_id", "1.1.1.1"),
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
    encaps.append(infra_api.GetPacketTemplate('ENCAP_QTAG'))

    return encaps


def GetDhcpServerVlan(testcase, packet):
    return testcase.config.localmapping.VNIC.VlanId

def GetDhcpServerMAC(testcase, packet):
    return "a1:b2:c3:d4:e5:f6"

def GetDhcpServerIp(testcase, packet):
    return str(testcase.config.dhcprelay.ServerIp)
