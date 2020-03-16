#! /usr/bin/python3
import ipaddress
import json
import os
import re

import infra.common.objects as objects
import infra.config.base as base

import apollo.config.utils as utils
import apollo.config.topo as topo
from apollo.config.store import client as EzAccessStoreClient

# Start and End inclusive range
def irange(start, end):
    return range(start, end+1)

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

class Resmgr(base.ConfigObjectBase):
    #TODO: read from PDS header files / naples mem & init
    MAX_OPER = 1
    MAX_DEVICE = 1
    MAX_INTERFACE = 3
    MAX_TUNNEL = 2048 if utils.IsPipelineApulu() else 1023
    MAX_NEXTHOP = 4095
    MAX_NEXTHOPGROUP = 2048
    MAX_LMAPPING = 131072
    MAX_RMAPPING = 2097152
    MAX_VPC = 64
    MAX_VNIC = 128 if utils.IsPipelineArtemis() or utils.IsPipelineApulu() else 64
    MAX_HOST_INTERFACES = 8 if utils.IsPipelineApulu() else 2
    # Apulu supports 8 lif for now and therefore cfg cannot have more than 8 subnets
    MAX_SUBNET = MAX_HOST_INTERFACES if utils.IsPipelineApulu() else 64
    if utils.IsPipelineArtemis():
        MAX_POLICY = 1023
        MAX_ROUTE_TABLE = 128
        MAX_ROUTES_PER_ROUTE_TBL = 63
    elif utils.IsPipelineApulu():
        MAX_POLICY = 32
        MAX_ROUTE_TABLE = 8
        MAX_ROUTES_PER_ROUTE_TBL = 16383
    else:
        MAX_POLICY = 1023
        MAX_ROUTE_TABLE = 1024
        MAX_ROUTES_PER_ROUTE_TBL = 1023
    MAX_POLICY_PER_VNIC = 2
    MAX_POLICER_PER_DIRECTION = 1024
    MAX_MIRROR = 8
    MAX_TAG = 1
    MAX_METER = 64
    MAX_UNDERLAY_NHS = 2
    MAX_OVERLAY_NHS = 8
    MAX_DHCP_RELAY = 16
    MAX_NAT_PB = 1024
    MAX_BGP_SESSIONS = 5
    MAX_BGP_PEER_AF_SESSIONS = 5
    MAX_BGP_PEERS = 10
    MAX_BGP_EVPN_EVI = 50
    MAX_BGP_EVPN_EVI_RT = 50
    MAX_BGP_EVPN_IP_VRF = 50
    MAX_BGP_EVPN_IP_VRF_RT = 50

    NICMGR_HOST_LIF_BASE = 72
    NICMGR_HOST_LIF_COUNT = 1

    # Starts VPC prefixes from 10/8 to 42/8
    VPC_V4_PREFIX_BASE=10
    # Create overlapping prefixes for every 32 VPCs
    VPC_V4_PREFIX_OVERLAP_DIST=32
    # Starts VPC prefixes from aaaa:0001:0010/48 to aaaa:0001:00042/48
    VPC_V6_BASE='aaaa:0001'
    VPC_V6_PREFIX_BASE=10
    # Create overlapping prefixes for every 32 VPCs
    VPC_V6_PREFIX_OVERLAP_DIST=128

    InvalidMplsSlotIdAllocator = iter(irange(50001,90000))
    InvalidVxlanIdAllocator = iter(irange(10001,12000))
    # This will be used to avoid overlap between topology and resource manager id allocation
    # used only for Tunnel, Nexthop and Interface objects
    BaseDuplicateIdAllocator = 100000

    # TCP/UDP ports for flow and service mapping. Right now keeping it in resmgr.
    TransportSrcPort   = 100 # For VNET packets
    TransportDstPort   = 200 # For VNET packets
    TransportSvcPort   = 400 # For VIP, Underlay tcp/udp port
    TransportSrcLBPort = 101 # Local backend service port, 101 mapped to 400

    def __init__(self, node):
        super().__init__()
        self.Node = node
        self.EpochAllocator = iter(irange(1,4096))
        # tunnel id=1 is mytep
        self.TunnelIdAllocator = iter(irange(2,4096))
        self.VpcIdAllocator = iter(irange(1,1024))
        self.SubnetIdAllocator = iter(irange(1,1024))
        self.RemoteMappingIdAllocator = iter(irange(1,16*1024*1024))
        self.FlowIdAllocator = iter(irange(1,1*1024*1024))
        self.LocalMappingIdAllocator = iter(irange(1,128*1024))
        self.VnicVlanIdAllocator = iter(irange(1,1024))
        self.VnicMplsSlotIdAllocator = iter(irange(10000,11024))
        self.VnicIdAllocator = iter(irange(1,1024))
        self.InterfaceIdAllocator = iter(irange(1, 66))
        self.V4RouteTableIdAllocator = iter(irange(1,1024))
        self.V6RouteTableIdAllocator = iter(irange(10001,11024))
        self.RouteIdAllocator = iter(irange(1, 1024 * 1023))
        self.V4SecurityPolicyIdAllocator = iter(irange(1,2048))
        self.V6SecurityPolicyIdAllocator = iter(irange(10001,12048))
        self.SecurityPolicyIdAllocator = iter(irange(1,4*1024*1024))
        self.VirtualRouterMacAllocator = objects.TemplateFieldObject("macstep/00CC.0000.0001/0000.0000.0001")
        self.VnicMacAllocator = objects.TemplateFieldObject("macstep/00DD.0000.0001/0000.0000.0001")
        self.RemoteMappingMacAllocator = objects.TemplateFieldObject("macstep/00EE.0000.0001/0000.0000.0001")
        self.TepIpAddressAllocator = ipaddress.IPv4Network('172.16.0.0/16').hosts()
        self.TepIpv6AddressAllocator = ipaddress.IPv6Network('ffff::100:0/104').hosts()
        self.IGWMplsSlotIdAllocator = iter(irange(30001,31024))
        self.RemoteInternetNonNatTunAllocator = None
        self.RemoteInternetNatTunAllocator = None
        self.RemoteMplsVnicTunAllocator = None
        self.PublicIpAddressAllocator = ipaddress.IPv4Network('200.0.0.0/16').hosts()
        self.PublicIpv6AddressAllocator = ipaddress.IPv6Network('eeee:0:0:0::/64').hosts()
        self.IGWVxlanIdAllocator = iter(irange(50001, 51024))
        self.VxlanIdAllocator = iter(irange(80001, 81024))
        self.MirrorSessionIdAllocator = iter(irange(1, 8))
        self.PortIdAllocator = iter(irange(1, 4))
        self.DhcpIdAllocator = iter(irange(1, 16))
        self.NatPoolIdAllocator = iter(irange(1, 1000))
        self.PolicerIdAllocator = iter(irange(1, 2048))
        self.EvpnEviIdAllocator = iter(irange(1, 50))
        self.EvpnEviRtIdAllocator = iter(irange(1, 50))
        self.EvpnIpVrfIdAllocator = iter(irange(1, 50))
        self.EvpnIpVrfRtIdAllocator = iter(irange(1, 50))

        # ---------------------------------------------------------------------------------
        # Artemis specific configs
        # Nexthop IP allocator. Curently unique IP (globally)
        # TODO : Can have same IP across VPCs.
        self.NexthopIdAllocator = iter(irange(1,1*1024*1024))
        self.NexthopIpV4AddressAllocator = ipaddress.IPv4Network('210.0.0.0/16').hosts()
        self.NexthopIpV6AddressAllocator = ipaddress.IPv6Network('eeee:eeee:0:0::/64').hosts()
        self.NexthopMacAllocator = objects.TemplateFieldObject("macstep/0055.0000.0001/0000.0000.0001")
        self.NexthopGroupIdAllocator = iter(irange(1,1*1024))
        self.V4TagIdAllocator = iter(irange(1,1))
        self.V6TagIdAllocator = iter(irange(2,2))
        self.NexthopVxlanIdAllocator = iter(irange(90001, 91024))
        self.V4MeterIdAllocator = iter(irange(1,64))
        self.V6MeterIdAllocator = iter(irange(65,129))
        # Currently one to one mapping with CA-IP. TODO. Many to one
        self.SvcMappingPublicIpV4AddressAllocator = ipaddress.IPv4Network('150.0.0.0/16').hosts()
        self.SvcMappingPublicIpV6AddressAllocator = ipaddress.IPv6Network('eeee:dddd:dddd:0::/64').hosts()
        self.NexthopVlanIdAllocator = iter(irange(4001, 5124))
        # Provider IP for local and remote. One to One mapping now. Many to one can be done later
        self.ProviderIpV4Network = '220.0.0.0/8'
        self.ProviderIpV6Network = 'eeee:eeee:eee0:0::/64'
        self.ProviderIpV4Network = ipaddress.IPv4Network(self.ProviderIpV4Network)
        self.ProviderIpV6Network = ipaddress.IPv6Network(self.ProviderIpV6Network)
        self.ProviderIpV4AddressAllocator = ipaddress.IPv4Network(self.ProviderIpV4Network).hosts()
        self.ProviderIpV6AddressAllocator = ipaddress.IPv6Network(self.ProviderIpV6Network).hosts()
        self.VpcVxlanIdAllocator = iter(irange(50000, 51024))
        self.Nat46Address = ipaddress.IPv6Network('aaaa:aaaa:0:0::/64')
        self.TepMacAllocator = objects.TemplateFieldObject("macstep/0065.0000.0001/0000.0000.0001")
        # Public IP adddress in the underlay VC
        self.SvcMappingPublicIpV4AddressAllocator = ipaddress.IPv4Network('150.0.0.0/16').hosts()
        self.SvcMappingPublicIpV6AddressAllocator = ipaddress.IPv6Network('eeee:dddd:dddd:0::/64').hosts()
        # -------------------------------------------------------------------
        self.SvcTunAllocator = None
        self.RemoteSvcTunAllocator = None
        self.UnderlayTunAllocator = None
        self.UnderlayECMPTunAllocator = None
        self.OverlayNhGroupAllocator = None
        self.RemoteSvcTunIPv4Addr = ipaddress.IPv4Network('30.0.0.0/24').hosts()

        #Apulu specific configs
        self.UnderlayNHAllocator = None
        self.OverlayNHAllocator = None
        self.DualEcmpNhAllocator = None
        self.UnderlayNhGroupAllocator = None
        self.DualEcmpNhGroupAllocator = None
        self.DeviceMacAllocator = objects.TemplateFieldObject("macstep/0002.0100.0000/0000.0000.0001")
        self.NumVnicPolicyAllocator = utils.rrobiniter(range(Resmgr.MAX_POLICY_PER_VNIC+1))
        self.UnderlayNumNexthopsAllocator = utils.rrobiniter(range(1, Resmgr.MAX_UNDERLAY_NHS+1))
        self.OverlayNumNexthopsAllocator = utils.rrobiniter(range(1, Resmgr.MAX_OVERLAY_NHS+1))

        self.L3InterfaceIPv4PfxPool = iter(ipaddress.IPv4Network('40.40.0.0/16').subnets(new_prefix=24))

        # Host interface configs
        self.HostMemoryAllocator = None
        self.HostIfs = dict()
        # Host interface index allocator for IOTA.
        # Ifindices 72 and 73 (0x48, 0x49) are reserved, so we have a non-contiuous list
        HostIfIdxList = [0x80000046, 0x80000047, 0x8000004A, 0x8000004B, 0x8000004C, 0x8000004D, 0x8000004E, 0x8000004F]
        self.HostIfIdxAllocator = iter(HostIfIdxList)
        self.LoopbackIfIdAllocator = iter(irange(1, 16))

        # Metaswitch
        self.BgpIdAllocator = iter(irange(1, 10))
        self.BgpPeerIdAllocator = iter(irange(1, 10))
        self.BgpPeerAfIdAllocator = iter(irange(1, 10))

        return

    def Show(self):
        return

    def ReadHostInterfaceInfo(self):
        if utils.IsDryRun(): return
        # reads nicmgr.log and get interface info
        nicmgrlog = utils.GetNicmgrLogPath()
        f = open(nicmgrlog, "r")
        for ifobj in self.HostIfs.values():
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

    def InitializeHostMemMgr(self):
        if utils.IsDryRun(): return
        self.HostMemoryAllocator = objects.GetHostMemMgrObject()
        assert self.HostMemoryAllocator is not None

    def InitializeHostInterfaces(self):
        if not utils.IsDol(): return
        with open(utils.GetDeviceJsonPath(), 'r') as fp:
            obj = json.load(fp)
        lifbase = Resmgr.NICMGR_HOST_LIF_BASE
        lifcount = Resmgr.NICMGR_HOST_LIF_COUNT
        # read host interface info from device.json
        for ethdev in obj["eth_dev"]:
            if ethdev["type"] != 'host':
                continue
            ifname = ethdev["name"]
            intf = NicmgrInterface(ifname)
            intf.SetLifBase(lifbase)
            self.HostIfs.update({ifname: intf})
            lifbase += lifcount
        if utils.IsInterfaceSupported():
            self.InitializeHostMemMgr()
            self.ReadHostInterfaceInfo()

    def Initialize(self):
        self.InitializeHostInterfaces()

    def CreateInternetTunnels(self):
        objs = EzAccessStoreClient[self.Node].GetIgwNonNatTunnels()
        if len(objs) != 0:
            self.RemoteInternetNonNatTunAllocator = utils.rrobiniter(objs)
        objs = EzAccessStoreClient[self.Node].GetIgwNatTunnels()
        if len(objs) != 0:
            self.RemoteInternetNatTunAllocator = utils.rrobiniter(objs)

    def CollectSvcTunnels(self):
        if utils.IsPipelineArtemis():
            objs = EzAccessStoreClient[self.Node].GetSvcTunnels()
            if len(objs) != 0:
                self.SvcTunAllocator = utils.rrobiniter(objs)
            objs = EzAccessStoreClient[self.Node].GetSvcTunnels(True)
            if len(objs) != 0:
                self.RemoteSvcTunAllocator = utils.rrobiniter(objs)

    def CreateVnicTunnels(self):
        objs = EzAccessStoreClient[self.Node].GetWorkloadTunnels()
        if len(objs) != 0:
            self.RemoteMplsVnicTunAllocator = utils.rrobiniter(objs)

    def CreateUnderlayTunnels(self):
        objs = EzAccessStoreClient[self.Node].GetUnderlayTunnels()
        if len(objs) != 0:
            self.UnderlayTunAllocator = utils.rrobiniter(objs)
        objs = EzAccessStoreClient[self.Node].GetUnderlayTunnels(ecmp=True)
        if len(objs) != 0:
            self.UnderlayECMPTunAllocator = utils.rrobiniter(objs)

    def CreateUnderlayNHAllocator(self):
        objs = EzAccessStoreClient[self.Node].GetUnderlayNexthops()
        if len(objs) != 0:
            self.UnderlayNHAllocator = utils.rrobiniter(objs)

    def CreateOverlayNHAllocator(self):
        objs = EzAccessStoreClient[self.Node].GetOverlayNexthops()
        if len(objs) != 0:
            self.OverlayNHAllocator = utils.rrobiniter(objs)

    def CreateDualEcmpNhAllocator(self):
        objs = EzAccessStoreClient[self.Node].GetDualEcmpNexthops()
        if len(objs) != 0:
            self.DualEcmpNhAllocator = utils.rrobiniter(objs)

    def CreateUnderlayNhGroupAllocator(self):
        objs = EzAccessStoreClient[self.Node].GetUnderlayNhGroups()
        if len(objs) != 0:
            self.UnderlayNhGroupAllocator = utils.rrobiniter(objs)

    def CreateOverlayNhGroupAllocator(self):
        objs = EzAccessStoreClient[self.Node].GetOverlayNhGroups()
        if len(objs) != 0:
            self.OverlayNhGroupAllocator = utils.rrobiniter(objs)

    def CreateDualEcmpNhGroupAllocator(self):
        objs = EzAccessStoreClient[self.Node].GetDualEcmpNhGroups()
        if len(objs) != 0:
            self.DualEcmpNhGroupAllocator = utils.rrobiniter(objs)

    # Create subnets from base prefix
    # - base is a prefix in the form of '10.0.0.0/16'
    # - sublen is the subnet length, gt base prefix length.
    # - poolid , subnet pool index with in a vpc
    @staticmethod
    def CreateIPv4SubnetPool(base, sublen, poolid):
        assert(isinstance(base, ipaddress.IPv4Network))
        assert(sublen >= 10)
        assert(poolid < 16)
        addr = base.network_address + (poolid << (32 - 8))
        pfxstr = "%s/8" %(str(ipaddress.IPv4Address(addr)))
        base = ipaddress.IPv4Network(pfxstr)
        return iter(base.subnets(new_prefix=sublen))

    @staticmethod
    def CreateIPv6SubnetPool(base, sublen, poolid):
        assert(isinstance(base, ipaddress.IPv6Network))
        assert(sublen >= 64)
        assert(poolid < 16)
        addr = base.network_address + (poolid << (128 - 56))
        pfxstr = "%s/56" %(str(ipaddress.IPv6Address(addr)))
        base = ipaddress.IPv6Network(pfxstr)
        return iter(base.subnets(new_prefix=sublen))

    @staticmethod
    def CreateIpv4AddrPool(subnet):
        assert(isinstance(subnet, ipaddress.IPv4Network))
        return iter(subnet.hosts())

    @staticmethod
    def CreateIpv6AddrPool(subnet):
        assert(isinstance(subnet, ipaddress.IPv6Network))
        return iter(subnet.hosts())

    # The below function will be called for every Remote TEP
    @staticmethod
    def CreateRemoteVnicMplsSlotAllocator():
        mplsbase = 20000
        return iter(irange(mplsbase,mplsbase + 1027)) # 1M Remote Mappings/1022 Teps

    # The below function will be called for every Remote TEP
    @staticmethod
    def CreateRemoteVnicVxlanIdAllocator():
        vxlanbase = 30000
        return iter(irange(vxlanbase,vxlanbase + 1027)) # 1M Remote Mappings/1022 Teps

    @staticmethod
    def GetVpcIPv4Prefix(vpcid):
        pfxstr = '%d.0.0.0/8'%((vpcid%Resmgr.VPC_V4_PREFIX_OVERLAP_DIST)+Resmgr.VPC_V4_PREFIX_BASE)
        return ipaddress.IPv4Network(pfxstr)

    @staticmethod
    def GetVpcIPv6Prefix(vpcid):
        pfxstr = '%s:%04x::/48'%(Resmgr.VPC_V6_BASE, ((vpcid%Resmgr.VPC_V6_PREFIX_OVERLAP_DIST)+Resmgr.VPC_V6_PREFIX_BASE))
        return ipaddress.IPv6Network(pfxstr)

    @staticmethod
    def GetVpcInternetNatPoolPfx(vpcid):
        pfxstr = '50.5.%d.0/24'%(vpcid)
        return ipaddress.IPv4Network(pfxstr)

    @staticmethod
    def GetVpcInfraNatPoolPfx(vpcid):
        pfxstr = '10.%d.199.0/24'%(vpcid)
        return ipaddress.IPv4Network(pfxstr)

    @staticmethod
    def GetNatPoolPortRange():
        return 10000,20000

client = dict()
def Init(node):
    global client
    resmgrObj = Resmgr(node)
    resmgrObj.Initialize()
    client[node] = resmgrObj
    return
