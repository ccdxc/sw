#! /usr/bin/python3
import infra.common.objects as objects
import apollo.config.utils as utils
from apollo.config.store import Store
import ipaddress

# Start and End inclusive range
def irange(start, end):
    return range(start, end+1)

EpochAllocator = iter(irange(1,4096))
TunnelIdAllocator = iter(irange(1,1024))
VpcIdAllocator = iter(irange(1,1024))
SubnetIdAllocator = iter(irange(1,1024))
RemoteMappingIdAllocator = iter(irange(1,1*1024*1024))
FlowIdAllocator = iter(irange(1,1*1024*1024))
LocalMappingIdAllocator = iter(irange(1,33*1024))
VnicVlanIdAllocator = iter(irange(1,1024))
VnicMplsSlotIdAllocator = iter(irange(10000,11024))
VnicIdAllocator = iter(irange(1,1024))
V4RouteTableIdAllocator = iter(irange(1,1024))
V6RouteTableIdAllocator = iter(irange(10001,11024))
SecurityPolicyIdAllocator = iter(irange(1,4*1024*1024))
VirtualRouterMacAllocator = objects.TemplateFieldObject("macstep/00CC.0000.0001/0000.0000.0001")
VnicMacAllocator = objects.TemplateFieldObject("macstep/00DD.0000.0001/0000.0000.0001")
RemoteMappingMacAllocator = objects.TemplateFieldObject("macstep/00EE.0000.0001/0000.0000.0001")
TepIpAddressAllocator = ipaddress.IPv4Network('172.16.0.0/21').hosts()
RouteIPv6Allocator = iter(ipaddress.IPv6Network('cccc:0001:0001:0001::/64').subnets(new_prefix=96))
RouteIPv4Allocator = iter(ipaddress.IPv4Network('192.0.0.0/8').subnets(new_prefix=24))
IGWMplsSlotIdAllocator = iter(irange(30001,31024))
InvalidMplsSlotIdAllocator = iter(irange(50001,90000))
RemoteMplsInternetTunAllocator = None
RemoteMplsVnicTunAllocator = None
PublicIpAddressAllocator = ipaddress.IPv4Network('200.0.0.0/24').hosts()
PublicIpv6AddressAllocator = ipaddress.IPv6Network('eeee:0:0:0::/64').hosts()
IGWVxlanIdAllocator = iter(irange(50001, 51024))
VxlanIdAllocator = iter(irange(80001, 81024))

# Create subnets from base prefix
# - base is a prefix in the form of '10.0.0.0/16'
# - sublen is the subnet length, gt base prefix length.
# - poolid , subnet pool index with in a vpc
def CreateIPv4SubnetPool(base, sublen, poolid):
    assert(isinstance(base, ipaddress.IPv4Network))
    assert(sublen >= 16)
    assert(poolid < 16)
    addr = base.network_address + (poolid << (32 - 12))
    pfxstr = "%s/12" %(str(ipaddress.IPv4Address(addr)))
    base = ipaddress.IPv4Network(pfxstr)
    return iter(base.subnets(new_prefix=sublen))

def CreateIPv6SubnetPool(base, sublen, poolid):
    assert(isinstance(base, ipaddress.IPv6Network))
    assert(sublen >= 64)
    assert(poolid < 16)
    addr = base.network_address + (poolid << (128 - 56))
    pfxstr = "%s/56" %(str(ipaddress.IPv6Address(addr)))
    base = ipaddress.IPv6Network(pfxstr)
    return iter(base.subnets(new_prefix=sublen))

def CreateIpv4AddrPool(subnet):
    assert(isinstance(subnet, ipaddress.IPv4Network))
    return iter(subnet.hosts())

def CreateIpv6AddrPool(subnet):
    assert(isinstance(subnet, ipaddress.IPv6Network))
    return iter(subnet.hosts())

def CreateMplsInternetTunnels():
    global RemoteMplsInternetTunAllocator
    objs = Store.GetIgwTunnels()
    if len(objs) != 0:
        RemoteMplsInternetTunAllocator = utils.rrobiniter(objs)

def CreateMplsVnicTunnels():
    global RemoteMplsVnicTunAllocator
    objs = Store.GetWorkloadTunnels()
    if len(objs) != 0:
        RemoteMplsVnicTunAllocator = utils.rrobiniter(objs)

# The below function will be called for every Remote TEP
def  CreateRemoteVnicMplsSlotAllocator():
    mplsbase = 20000
    return iter(irange(mplsbase,mplsbase + 1027)) # 1M Remote Mappings/1022 Teps

# The below function will be called for every Remote TEP
def  CreateRemoteVnicVxlanIdAllocator():
    vxlanbase = 30000
    return iter(irange(vxlanbase,vxlanbase + 1027)) # 1M Remote Mappings/1022 Teps

# Starts VPC prefixes from 10/8 to 42/8
VPC_V4_PREFIX_BASE=10
# Create overlapping prefixes for every 32 VPCs
VPC_V4_PREFIX_OVERLAP_DIST=32
def GetVpcIPv4Prefix(vpcid):
    pfxstr = '%d.0.0.0/8'%((vpcid%VPC_V4_PREFIX_OVERLAP_DIST)+VPC_V4_PREFIX_BASE)
    return ipaddress.IPv4Network(pfxstr)

# Starts VPC prefixes from aaaa:0001:0010/48 to aaaa:0001:00042/48
VPC_V6_BASE='aaaa:0001'
VPC_V6_PREFIX_BASE=10
# Create overlapping prefixes for every 32 VPCs
VPC_V6_PREFIX_OVERLAP_DIST=128
def GetVpcIPv6Prefix(vpcid):
    pfxstr = '%s:%04x::/48'%(VPC_V6_BASE, ((vpcid%VPC_V6_PREFIX_OVERLAP_DIST)+VPC_V6_PREFIX_BASE))
    return ipaddress.IPv6Network(pfxstr)

