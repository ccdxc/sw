#! /usr/bin/python3
import ipaddress
import os
import re

import infra.common.objects as objects
import apollo.config.utils as utils
from apollo.config.store import Store

# Start and End inclusive range
def irange(start, end):
    return range(start, end+1)

HostMemoryAllocator = None
HostIntf2DevCmdAddrMap = dict()
Lif2QstateMap = None

EpochAllocator = iter(irange(1,4096))
# tunnel id=1 is mytep
TunnelIdAllocator = iter(irange(2,1024))
VpcIdAllocator = iter(irange(1,1024))
SubnetIdAllocator = iter(irange(1,1024))
RemoteMappingIdAllocator = iter(irange(1,1*1024*1024))
FlowIdAllocator = iter(irange(1,1*1024*1024))
LocalMappingIdAllocator = iter(irange(1,33*1024))
VnicVlanIdAllocator = iter(irange(1,1024))
VnicMplsSlotIdAllocator = iter(irange(10000,11024))
VnicIdAllocator = iter(irange(1,1024))
InterfaceIdAllocator = iter(irange(1, 8))
V4RouteTableIdAllocator = iter(irange(1,1024))
V6RouteTableIdAllocator = iter(irange(10001,11024))
V4SecurityPolicyIdAllocator = iter(irange(1,2048))
V6SecurityPolicyIdAllocator = iter(irange(10001,12048))
SecurityPolicyIdAllocator = iter(irange(1,4*1024*1024))
VirtualRouterMacAllocator = objects.TemplateFieldObject("macstep/00CC.0000.0001/0000.0000.0001")
VnicMacAllocator = objects.TemplateFieldObject("macstep/00DD.0000.0001/0000.0000.0001")
RemoteMappingMacAllocator = objects.TemplateFieldObject("macstep/00EE.0000.0001/0000.0000.0001")
TepIpAddressAllocator = ipaddress.IPv4Network('172.16.0.0/21').hosts()
TepIpv6AddressAllocator = ipaddress.IPv6Network('ffff::100:0/104').hosts()
IGWMplsSlotIdAllocator = iter(irange(30001,31024))
InvalidMplsSlotIdAllocator = iter(irange(50001,90000))
RemoteInternetNonNatTunAllocator = None
RemoteInternetNatTunAllocator = None
RemoteMplsVnicTunAllocator = None
PublicIpAddressAllocator = ipaddress.IPv4Network('200.0.0.0/16').hosts()
PublicIpv6AddressAllocator = ipaddress.IPv6Network('eeee:0:0:0::/64').hosts()
IGWVxlanIdAllocator = iter(irange(50001, 51024))
VxlanIdAllocator = iter(irange(80001, 81024))
InvalidVxlanIdAllocator = iter(irange(10001,12000))
MirrorSessionIdAllocator = iter(irange(1, 8))
PortIdAllocator = iter(irange(1, 2))

# ---------------------------------------------------------------------------------
# Artemis specific configs
# Nexthop IP allocator. Curently unique IP (globally)
# TODO : Can have same IP across VPCs.
NexthopIdAllocator = iter(irange(1,1*1024*1024))
NexthopIpV4AddressAllocator = ipaddress.IPv4Network('210.0.0.0/16').hosts()
NexthopIpV6AddressAllocator = ipaddress.IPv6Network('eeee:eeee:0:0::/64').hosts()
NexthopMacAllocator = objects.TemplateFieldObject("macstep/0055.0000.0001/0000.0000.0001")
V4TagIdAllocator = iter(irange(1,1))
V6TagIdAllocator = iter(irange(2,2))
NexthopVxlanIdAllocator = iter(irange(90001, 91024))
V4MeterIdAllocator = iter(irange(1,64))
V6MeterIdAllocator = iter(irange(65,129))
# Currently one to one mapping with CA-IP. TODO. Many to one
SvcMappingPublicIpV4AddressAllocator = ipaddress.IPv4Network('150.0.0.0/16').hosts()
SvcMappingPublicIpV6AddressAllocator = ipaddress.IPv6Network('eeee:dddd:dddd:0::/64').hosts()
NexthopVlanIdAllocator = iter(irange(4001, 5124))
# Provider IP for local and remote. One to One mapping now. Many to one can be done later
ProviderIpV4Network = '220.0.0.0/16'
ProviderIpV6Network = 'eeee:eeee:eee0:0::/64'
ProviderIpV4Network = ipaddress.IPv4Network(ProviderIpV4Network)
ProviderIpV6Network = ipaddress.IPv6Network(ProviderIpV6Network)
ProviderIpV4AddressAllocator = ipaddress.IPv4Network(ProviderIpV4Network).hosts()
ProviderIpV6AddressAllocator = ipaddress.IPv6Network(ProviderIpV6Network).hosts()
VpcVxlanIdAllocator = iter(irange(50000, 51024))
Nat46Address = ipaddress.IPv6Network('aaaa:aaaa:0:0::/64')
TepMacAllocator = objects.TemplateFieldObject("macstep/0065.0000.0001/0000.0000.0001")
# TCP/UDP ports for flow and service mapping. Right now keeping it in resmgr.
TransportSrcPort   = 100 # For VNET packets
TransportDstPort   = 200 # For VNET packets
TransportSvcPort   = 400 # For VIP, Substrate tcp/udp port
TransportSrcLBPort = 101 # Local backend service port, 101 mapped to 400
# Public IP adddress in the substrate VC
SvcMappingPublicIpV4AddressAllocator = ipaddress.IPv4Network('150.0.0.0/16').hosts()
SvcMappingPublicIpV6AddressAllocator = ipaddress.IPv6Network('eeee:dddd:dddd:0::/64').hosts()
# -------------------------------------------------------------------
SvcTunAllocator = None
RemoteSvcTunAllocator = None
RemoteSvcTunIPv4Addr = ipaddress.IPv4Network('30.0.0.0/24').hosts()

#TODO: read from PDS header files & init
MAX_DEVICE = 1
MAX_TUNNEL = 1023
MAX_VPC = 64
MAX_VNIC = 128 if utils.IsPipelineArtemis() else 64
MAX_SUBNET = 64
MAX_ROUTE_TABLE = 128 if utils.IsPipelineArtemis() else 1024
MAX_ROUTES_PER_ROUTE_TBL = 63 if utils.IsPipelineArtemis() else 1023
MAX_POLICY = 1023
MAX_MIRROR = 8
MAX_TAG = 1
MAX_METER = 64

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

def CreateInternetTunnels():
    global RemoteInternetNonNatTunAllocator
    global RemoteInternetNatTunAllocator
    objs = Store.GetIgwNonNatTunnels()
    if len(objs) != 0:
        RemoteInternetNonNatTunAllocator = utils.rrobiniter(objs)
    objs = Store.GetIgwNatTunnels()
    if len(objs) != 0:
        RemoteInternetNatTunAllocator = utils.rrobiniter(objs)

def CollectSvcTunnels():
    if utils.IsPipelineArtemis():
        global SvcTunAllocator
        objs = Store.GetSvcTunnels()
        if len(objs) != 0:
            SvcTunAllocator = utils.rrobiniter(objs)
        global RemoteSvcTunAllocator
        objs = Store.GetSvcTunnels(True)
        if len(objs) != 0:
            RemoteSvcTunAllocator = utils.rrobiniter(objs)

def CreateVnicTunnels():
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

def __hostmemmgr_init():
    global HostMemoryAllocator
    HostMemoryAllocator = objects.GetHostMemMgrObject()
    assert HostMemoryAllocator is not None

def __hostif_init():
    # reads nicmgr.log and generate host if to devcmd mem addr mapping
    global HostIntf2DevCmdAddrMap
    nicmgrlog = os.path.join(os.environ['WS_TOP'], "nic/nicmgr.log")
    pattern = ' eth(\d+): regs_mem_addr (\w+) devcmd_mem_addr (\w+)'
    f = open(nicmgrlog, "r")
    for line in f:
        match = re.search(pattern, line)
        if match is None:
            continue
        ifname = "eth%d" % (int(match.groups()[0]))
        devcmdaddr = int(match.groups()[2], base=16)
        HostIntf2DevCmdAddrMap.update({ifname: devcmdaddr})
    f.close()
    return

def __lif2qstatemap_init():
    global Lif2QstateMap
    Lif2QstateMap = dict()
    nicmgrlog = os.path.join(os.environ['WS_TOP'], "nic/nicmgr.log")
    pattern = ' lif-(\d{2}): qtype: (\d{1}), qstate_base: (\w+)'
    f = open(nicmgrlog, "r")
    for line in f:
        match = re.search(pattern, line)
        if match is None:
            continue
        lif_id = int(match.groups()[0])
        q_type = int(match.groups()[1])
        qstate_base  = int(match.groups()[2], base=16)
        qstateaddr_list = Lif2QstateMap.get(lif_id, [])
        qstateaddr_list.insert(q_type, qstate_base)
        Lif2QstateMap.update({lif_id: qstateaddr_list})
    f.close()
    return

def InitNicMgrObjects():
    __hostmemmgr_init()
    __hostif_init()
    __lif2qstatemap_init()
