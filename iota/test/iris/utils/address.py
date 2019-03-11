#! /usr/bin/python3

def convertMacStr2Dec(mac_string):
    """ given a mac address in string format, convert it to integer (base10) """
    # convertMacStr2Dec('00:bb:cc:33:01:02') will return 806584779010
    mac_addr_int = int(mac_string.translate(str.maketrans(dict.fromkeys('.-:'))), 16)
    return mac_addr_int

def formatMacAddr(mac_addr_int):
    """ given a mac address in integer (base10) format, convert it to string notation """
    # formatMacAddr(806584779010) will return '00:bb:cc:33:01:02'
    mac_addr_hex = "{:012x}".format(mac_addr_int)
    mac_addr_str = ":".join(mac_addr_hex[i:i+2] for i in range(0, len(mac_addr_hex), 2))
    return mac_addr_str

def convertMcastIP2McastMAC(ipv4mcast_addr):
    #ipv4mcast_addr is assumed to be IPv4 Multicast Address in string format
    ipv4mcast_mac_base =  '01:00:5e:'
    v4octets = ipv4mcast_addr.split('.')
    #Get last 23 bits
    second_octet = int(v4octets[1]) & 127
    third_octet = int(v4octets[2])
    fourth_octet = int(v4octets[3])
    ipv4mcast_mac_offset = format(second_octet, '02x') + ':' + format(third_octet, '02x') + ':' + format(fourth_octet, '02x')
    v4mcast_mac = ipv4mcast_mac_base + ipv4mcast_mac_offset
    return v4mcast_mac

def is_gvrp_mcast_mac(mac_addr_str):
    # GARP VLAN Registration Protocol (also known as IEEE 802.1q GVRP)
    if "01:80:c2:00:00:21" == mac_addr_str:
        return True
    return False

def is_all_node_mc_mac(mac_addr_str):
    # All Node Multicast Address
    if "33:33:00:00:00:01" == mac_addr_str:
        return True
    return False

