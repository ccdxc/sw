#! /usr/bin/python3
import ipaddress
import json
import os
import re

import infra.common.objects as objects
import apollo.config.utils as utils
import apollo.config.topo as topo
from apollo.config.store import EzAccessStore

# Start and End inclusive range
def irange(start, end):
    return range(start, end+1)

HostMemoryAllocator = None
HostIfs = dict()
NICMGR_HOST_LIF_BASE = 71
NICMGR_HOST_LIF_COUNT = 1

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
InterfaceIdAllocator = iter(irange(1, 66))
V4RouteTableIdAllocator = iter(irange(1,1024))
V6RouteTableIdAllocator = iter(irange(10001,11024))
RouteIdAllocator = iter(irange(1, 1024 * 1023))
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
UnderlayTunAllocator = None
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
NexthopGroupIdAllocator = iter(irange(1,1*1024))
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
TransportSvcPort   = 400 # For VIP, Underlay tcp/udp port
TransportSrcLBPort = 101 # Local backend service port, 101 mapped to 400
# Public IP adddress in the underlay VC
SvcMappingPublicIpV4AddressAllocator = ipaddress.IPv4Network('150.0.0.0/16').hosts()
SvcMappingPublicIpV6AddressAllocator = ipaddress.IPv6Network('eeee:dddd:dddd:0::/64').hosts()
# -------------------------------------------------------------------
SvcTunAllocator = None
RemoteSvcTunAllocator = None
UnderlayTunAllocator = None
UnderlayECMPTunAllocator = None
OverlayNhGroupAllocator = None
RemoteSvcTunIPv4Addr = ipaddress.IPv4Network('30.0.0.0/24').hosts()

#Apulu specific configs
UnderlayNHAllocator = None
OverlayNHAllocator = None
DualEcmpNhAllocator = None
UnderlayNhGroupAllocator = None
DualEcmpNhGroupAllocator = None
DeviceMacAllocator = objects.TemplateFieldObject("macstep/0002.0100.0000/0000.0000.0001")

#TODO: read from PDS header files & init
MAX_DEVICE = 1
MAX_TUNNEL = 1023
MAX_VPC = 64
MAX_VNIC = 128 if utils.IsPipelineArtemis() else 64
# Apulu supports 8 lif for now and therefore cfg cannot have more than 8 subnets
MAX_SUBNET = 8 if utils.IsPipelineApulu() else 64
MAX_ROUTE_TABLE = 128 if utils.IsPipelineArtemis() else 1024
MAX_ROUTES_PER_ROUTE_TBL = 63 if utils.IsPipelineArtemis() else 1023
MAX_POLICY = 1023
MAX_POLICY_PER_VNIC = 5
MAX_MIRROR = 8
MAX_TAG = 1
MAX_METER = 64
MAX_UNDERLAY_NHS = 2
MAX_OVERLAY_NHS = 8

NumVnicPolicyAllocator = utils.rrobiniter(range(MAX_POLICY_PER_VNIC+1))
UnderlayNumNexthopsAllocator = utils.rrobiniter(range(1, MAX_UNDERLAY_NHS+1))
OverlayNumNexthopsAllocator = utils.rrobiniter(range(1, MAX_OVERLAY_NHS+1))

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
    objs = EzAccessStore.GetIgwNonNatTunnels()
    if len(objs) != 0:
        RemoteInternetNonNatTunAllocator = utils.rrobiniter(objs)
    objs = EzAccessStore.GetIgwNatTunnels()
    if len(objs) != 0:
        RemoteInternetNatTunAllocator = utils.rrobiniter(objs)

def CollectSvcTunnels():
    if utils.IsPipelineArtemis():
        global SvcTunAllocator
        objs = EzAccessStore.GetSvcTunnels()
        if len(objs) != 0:
            SvcTunAllocator = utils.rrobiniter(objs)
        global RemoteSvcTunAllocator
        objs = EzAccessStore.GetSvcTunnels(True)
        if len(objs) != 0:
            RemoteSvcTunAllocator = utils.rrobiniter(objs)

def CreateVnicTunnels():
    global RemoteMplsVnicTunAllocator
    objs = EzAccessStore.GetWorkloadTunnels()
    if len(objs) != 0:
        RemoteMplsVnicTunAllocator = utils.rrobiniter(objs)

def CreateUnderlayTunnels():
    global UnderlayTunAllocator
    objs = EzAccessStore.GetUnderlayTunnels()
    if len(objs) != 0:
        UnderlayTunAllocator = utils.rrobiniter(objs)
    global UnderlayECMPTunAllocator
    objs = EzAccessStore.GetUnderlayTunnels(ecmp=True)
    if len(objs) != 0:
        UnderlayECMPTunAllocator = utils.rrobiniter(objs)

def CreateUnderlayNHAllocator():
    global UnderlayNHAllocator
    objs = EzAccessStore.GetUnderlayNexthops()
    if len(objs) != 0:
        UnderlayNHAllocator = utils.rrobiniter(objs)

def CreateOverlayNHAllocator():
    global OverlayNHAllocator
    objs = EzAccessStore.GetOverlayNexthops()
    if len(objs) != 0:
        OverlayNHAllocator = utils.rrobiniter(objs)

def CreateDualEcmpNhAllocator():
    global DualEcmpNhAllocator
    objs = EzAccessStore.GetDualEcmpNexthops()
    if len(objs) != 0:
        DualEcmpNhAllocator = utils.rrobiniter(objs)

def CreateUnderlayNhGroupAllocator():
    global UnderlayNhGroupAllocator
    objs = EzAccessStore.GetUnderlayNhGroups()
    if len(objs) != 0:
        UnderlayNhGroupAllocator = utils.rrobiniter(objs)

def CreateOverlayNhGroupAllocator():
    global OverlayNhGroupAllocator
    objs = EzAccessStore.GetOverlayNhGroups()
    if len(objs) != 0:
        OverlayNhGroupAllocator = utils.rrobiniter(objs)

def CreateDualEcmpNhGroupAllocator():
    global DualEcmpNhGroupAllocator
    objs = EzAccessStore.GetDualEcmpNhGroups()
    if len(objs) != 0:
        DualEcmpNhGroupAllocator = utils.rrobiniter(objs)

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

class NicmgrInterface:
    def __init__(self, ifname):
        self.IfName = ifname
        self.DevcmdMemAddr = None
        self.LifBase = 0
        self.LifCount = 1
        self.Lif2QstateMap = dict()
        return

    def SetLifBase(self, lifbase):
        self.LifBase = lifbase

    def SetLifCount(self, lifcount):
        self.Lifcount = lifcount

    def SetDevCmdAddr(self, devcmd_mem_addr):
        self.DevcmdMemAddr = devcmd_mem_addr

def __get_nicmgr_log_path():
    rel_path = "nic/nicmgr.log"
    abs_path = os.path.join(os.environ['WS_TOP'], rel_path)
    return abs_path

def __hostif_init():
    # reads nicmgr.log and get interface info
    global HostIfs
    nicmgrlog = __get_nicmgr_log_path()
    f = open(nicmgrlog, "r")
    for ifobj in HostIfs.values():
        # get lifbase and count
        pattern = ' %s: lif_base (\w+) lif_count (\w+)' % (ifobj.IfName)
        for line in f:
            match = re.search(pattern, line)
            if match is None:
                continue
            lifbase = int(match.groups()[0])
            lifcount = int(match.groups()[1])
            ifobj.SetLifBase(lifbase)
            ifobj.SetLifCount(lifcount)
            break

        # get devcmd addr
        pattern = ' %s: regs_mem_addr (\w+) devcmd_mem_addr (\w+)' % (ifobj.IfName)
        for line in f:
            match = re.search(pattern, line)
            if match is None:
                continue
            devcmdaddr = int(match.groups()[1], base=16)
            ifobj.SetDevCmdAddr(devcmdaddr)
            break

        # get qstate addr for all lifs under this device
        for i in range(lifcount):
            lif_id = lifbase + i
            pattern = ' lif-%d: qtype: (\d{1}), qstate_base: (\w+)' % (lif_id)
            for line in f:
                match = re.search(pattern, line)
                if match is None:
                    continue
                q_type = int(match.groups()[0])
                qstate_base  = int(match.groups()[1], base=16)
                qstateaddr_list = ifobj.Lif2QstateMap.get(lif_id, [])
                qstateaddr_list.insert(q_type, qstate_base)
                ifobj.Lif2QstateMap.update({lif_id: qstateaddr_list})
                if (q_type == 7):
                    break
    f.close()
    return

def InitNicMgrObjects():
    __hostmemmgr_init()
    __hostif_init()

def __get_device_json_path():
    rel_path = "nic/conf/%s/device.json" % (utils.GetPipelineName())
    abs_path = os.path.join(os.environ['WS_TOP'], rel_path)
    return abs_path

def __initialize_hostifs():
    global HostIfs
    with open(__get_device_json_path(), 'r') as fp:
        obj = json.load(fp)
    lifbase = NICMGR_HOST_LIF_BASE
    lifcount = NICMGR_HOST_LIF_COUNT
    # read host interface info from device.json
    for ethdev in obj["eth_dev"]:
        if ethdev["type"] != 'host':
            continue
        ifname = ethdev["name"]
        intf = NicmgrInterface(ifname)
        intf.SetLifBase(lifbase)
        HostIfs.update({ifname: intf})
        lifbase += lifcount

def Init():
    __initialize_hostifs()
    if utils.IsDryRun():
        return
    if utils.IsInterfaceSupported():
        InitNicMgrObjects()
    return

def ResetRouteIdAllocator():
    global RouteIdAllocator
    RouteIdAllocator = iter(irange(1, 1024))
