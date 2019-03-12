#! /usr/bin/python3
import infra.common.objects as objects
import ipaddress

# Start and End inclusive range
def irange(start, end):
    return range(start, end+1)

EpochAllocator = iter(irange(1,4096))
TunnelIdAllocator = iter(irange(1,4096))
PcnIdAllocator = iter(irange(1,1024))
SubnetIdAllocator = iter(irange(1,1024*1024))
PcnVlanIdAllocator = iter(irange(1,1024))
RouteTableIdAllocator = iter(irange(1,1024*1024))
SecurityPolicyIdAllocator = iter(irange(1,4*1024*1024))
VirtualRouterMacAllocator = objects.TemplateFieldObject("macstep/00CC.0000.0001/0000.0000.0001")
TepIpAddressAllocator = ipaddress.IPv4Network('172.16.0.0/21').hosts()

# Create subnets from base prefix
# - base is a prefix in the form of '10.0.0.0/16'
# - sublen is the subnet length, gt base prefix length.
def CreateIPv4SubnetPool(base, sublen):
    assert(isinstance(base, ipaddress.IPv4Network))
    return iter(base.subnets(new_prefix=sublen))
def CreateIPv6SubnetPool(base, sublen):
    assert(isinstance(base, ipaddress.IPv6Network))
    return iter(base.subnets(new_prefix=sublen))
def CreateIpv4AddrPool(subnet):
    assert(isinstance(subnet, ipaddress.IPv4Network))
    return iter(subnet.hosts())
def CreateIpv6AddrPool(subnet):
    assert(isinstance(subnet, ipaddress.IPv6Network))
    return iter(subnet.hosts())

# Starts PCN prefixes from 10/8 to 42/8
PCN_PREFIX_BASE=10
# Create overlapping prefixes for every 32 PCNs
PCN_PREFIX_OVERLAP_DIST=32
def GetPcnIpv4Prefix(pcnid):
    pfxstr = '%d.0.0.0/8'%((pcnid%PCN_PREFIX_OVERLAP_DIST)+PCN_PREFIX_BASE)
    return ipaddress.IPv4Network(pfxstr)
    
