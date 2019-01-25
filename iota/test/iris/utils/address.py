#! /usr/bin/python3

def convertMacStr2Dec(mac_string):
    # convertMacStr2Dec('00:bb:cc:33:01:02') will return 806584779010
    #mac_addr_int = int(mac_string.translate(None, ".-:"), 16)
    mac_addr_int = int(mac_string.translate(str.maketrans(dict.fromkeys('.-:'))), 16)
    return mac_addr_int

def formatMacAddr(mac_addr_int):
    # formatMacAddr(806584779010) will return '00:bb:cc:33:01:02'
    #mac_addr arg is in decimal
    mac_addr_hex = "{:012x}".format(mac_addr_int)
    mac_addr_str = ":".join(mac_addr_hex[i:i+2] for i in range(0, len(mac_addr_hex), 2))
    return mac_addr_str
