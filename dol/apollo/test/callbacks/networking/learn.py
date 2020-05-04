# /usr/bin/python3
import pdb
import ipaddress
from infra.common.objects import MacAddressBase

def GetLearnDelay(testcase, args):
    if args.delay_type == "age":
        return testcase.config.devicecfg.LearnAgeTimeout
    else:
        return 30

def GetMACAddrForValidation(testcase, args):
    mac_addr = testcase.config.localmapping.VNIC.MACAddr
    if testcase.module.name != "LEARN_NEG_MULT_UNTAG_VNIC_SAME_SUBNET":
        return mac_addr
    # spoof second untagged MAC
    spoof_mac_addr = MacAddressBase(string = str(mac_addr))
    spoof_mac_addr.update(1024)
    return spoof_mac_addr

def GetIPAddrForValidation(testcase, args):
    if testcase.module.name != "LEARN_NEG_IP_NOT_IN_SUBNET":
        return str(testcase.config.localmapping.IPAddr)
    # generate out of subnet IP address
    prefix = testcase.config.localmapping.VNIC.SUBNET.IPPrefix[1]
    ip_start = int(next(prefix.hosts()))
    return str(ipaddress.ip_address(ip_start + prefix.num_addresses + 3))
