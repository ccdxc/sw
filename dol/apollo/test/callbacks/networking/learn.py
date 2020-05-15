# /usr/bin/python3
import pdb
import ipaddress
from infra.common.objects import MacAddressBase

def GetLearnDelay(testcase, args):
    if args.delay_type == "age":
        return testcase.config.devicecfg.LearnAgeTimeout
    else:
        return 30

def GetIPAddrForValidation(testcase, args):
    if testcase.module.name != "LEARN_NEG_IP_NOT_IN_SUBNET":
        return str(testcase.config.localmapping.IPAddr)
    # generate out of subnet IP address
    prefix = testcase.config.localmapping.VNIC.SUBNET.IPPrefix[1]
    ip_start = int(next(prefix.hosts()))
    return str(ipaddress.ip_address(ip_start + prefix.num_addresses + 3))
