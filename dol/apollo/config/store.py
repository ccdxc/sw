#! /usr/bin/python3
from infra.common.objects   import ObjectDatabase as ObjectDatabase
from infra.config.store     import ConfigStore as ConfigStore
import infra.common.defs as defs

class EzAccessStore:
# Static variables and methods
    # Batch client
    batchClient = None
    templates = ConfigStore.templates
    specs = ConfigStore.specs
    dutNode = 1
    configClientDict = dict()
    dhcprelayConfig = None

# Class members
    def __init__(self, node):
        self.Node = node
        self.objects = ConfigStore.objects

        # Custom Database for easy access.
        self.trunks = ObjectDatabase()
        self.dhcprelayObjs = ObjectDatabase()
        self.tunnels = ObjectDatabase()
        self.nexthops = ObjectDatabase()
        self.nexthopgroups = ObjectDatabase()
        self.device = None
        self.underlay_vpc = None
        self.hostport = None
        self.switchport = None
        self.dutNode = 1
        self.underlay_ips = []
        return

    def SetTunnels(self, objs):
        if defs.TEST_TYPE == "IOTA":
            self.tunnels.db.clear()
        return self.tunnels.SetAll(objs)

    def SetNexthops(self, objs):
        if defs.TEST_TYPE == "IOTA":
            self.nexthops.db.clear()
        return self.nexthops.SetAll(objs)

    def SetNexthopgroups(self, objs):
        if defs.TEST_TYPE == "IOTA":
            self.nexthopgroups.db.clear()
        return self.nexthopgroups.SetAll(objs)

    def SetDevice(self,obj):
        self.device = obj

    def GetDevice(self):
        return self.device

    def SetHostPort(self, port):
        self.hostport = port

    def GetHostPort(self):
        return self.hostport

    def SetSwitchPort(self, port):
        self.switchport = port

    def GetSwitchPort(self):
        return self.switchport

    def SetUnderlayVPC(self, obj):
        self.underlay_vpc = obj

    def GetUnderlayVPCId(self):
        if self.underlay_vpc:
            return self.underlay_vpc.VPCId
        else:
            return -1

    def GetProviderIPAddr(self, count):
        if self.underlay_vpc:
            return self.underlay_vpc.GetProviderIPAddr(count)
        else:
            return None,-1

    def GetSvcMapping(self, ipversion):
        if self.underlay_vpc:
            return self.underlay_vpc.GetSvcMapping(ipversion)
        else:
            return None,-1

    def IsBitwMode(self):
        return self.device.IsBitwMode()

    def IsHostMode(self):
        return self.device.IsHostMode()

    def IsDeviceEncapTypeMPLS(self):
        return self.device.IsEncapTypeMPLS()

    def IsDeviceEncapTypeVXLAN(self):
        return self.device.IsEncapTypeVXLAN()

    def IsDeviceLearningEnabled(self):
        return self.device.IsLearningEnabled()

    def IsBridgingEnabled(self):
        return self.device.IsBridgingEnabled()

    def IsDeviceOverlayRoutingEnabled(self):
        if self.device is None:
            return False
        return self.device.IsOverlayRoutingEnabled()

    def GetDeviceEncapType(self):
        return self.device.EncapType

    def GetWorkloadTunnels(self):
        tunnels = []
        for tun in self.tunnels.GetAllInList():
            if tun.IsWorkload(): tunnels.append(tun)
        return tunnels

    def GetIgwNonNatTunnels(self):
        tunnels = []
        for tun in self.tunnels.GetAllInList():
            if tun.IsIgw() and tun.IsNat() is False:
                 tunnels.append(tun)
        return tunnels

    def GetIgwNatTunnels(self):
        tunnels = []
        for tun in self.tunnels.GetAllInList():
            if tun.IsIgw() and tun.IsNat(): tunnels.append(tun)
        return tunnels

    def GetSvcTunnels(self, remote=False):
        tunnels = []
        for tun in self.tunnels.GetAllInList():
            if tun.IsSvc() and tun.Remote is remote:
                tunnels.append(tun)
        return tunnels

    def GetUnderlayTunnels(self, ecmp=False):
        tunnels = []
        for tun in self.tunnels.GetAllInList():
            if ecmp is False:
                if tun.IsUnderlay(): tunnels.append(tun)
            elif ecmp is True:
                if tun.IsUnderlayEcmp(): tunnels.append(tun)
        return tunnels

    def GetOverlayTunnels(self):
        tunnels = []
        for tun in self.tunnels.GetAllInList():
            if tun.IsOverlay(): tunnels.append(tun)
        return tunnels

    def GetUnderlayNexthops(self, ecmp=False):
        nhops = []
        for nh in self.nexthops.GetAllInList():
            if ecmp is False:
                if nh.IsUnderlay(): nhops.append(nh)
            if ecmp is True:
                if nh.IsUnderlayEcmp(): nhops.append(nh)
        return nhops

    def GetUnderlayNhGroups(self):
        nhgs = []
        for nhg in self.nexthopgroups.GetAllInList():
            if nhg.IsUnderlay():
                nhgs.append(nhg)
        return nhgs

    def GetOverlayNexthops(self, ecmp=False):
        nhops = []
        for nh in self.nexthops.GetAllInList():
            if nh.IsOverlay(): nhops.append(nh)
        return nhops

    def GetDualEcmpNexthops(self):
        nhops = []
        for nh in self.nexthops.GetAllInList():
            if nh.IsOverlay() and nh.DualEcmp: nhops.append(nh)
        return nhops

    def GetOverlayNhGroups(self):
        nhgs = []
        for nhg in self.nexthopgroups.GetAllInList():
            if nhg.IsOverlay(): nhgs.append(nhg)
        return nhgs

    def GetDualEcmpNhGroups(self):
        nhgs = []
        for nhg in self.nexthopgroups.GetAllInList():
            if nhg.IsOverlay() and nhg.DualEcmp: nhgs.append(nhg)
        return nhgs

    @staticmethod
    def SetDHCPRelayInfo(configobj):
        EzAccessStore.dhcprelayConfig = configobj

    @staticmethod
    def GetDHCPRelayInfo():
        return EzAccessStore.dhcprelayConfig

    def GetDhcpRelayObjects(self):
        return self.dhcprelayObjs.GetAllInList()

    def SetDhcpRelayObjects(self, objs):
        return self.dhcprelayObjs.SetAll(objs)

    def GetTrunkingUplinks(self):
        return self.trunks.GetAllInList()

    def SetTrunkingUplinks(self, objs):
        return self.trunks.SetAll(objs)

    def GetNodeUuid(self, node):
        if node in self.uuid_map:
            node_uuid = self.uuid_map[node]
            node_uuid = node_uuid.replace('.', '')
            if node_uuid == '':
                return None
            return int(node_uuid, 16)
        return None

    def SetUnderlayIPs(self, underlay_ips):
        self.underlay_ips = underlay_ips

    def GetUnderlayIPs(self):
        return self.underlay_ips

    def GetLoopbackIp(self):
        for ip in self.underlay_ips:
            if 'Loopback' in ip.Name:
                return ip.IP
        return None

    def GetLoopbackRemoteTEP(self):
        for ip in self.underlay_ips:
            if 'Loopback' in ip.Name:
                return ip.RemoteTEP
        return None

    def GetUnderlayIp(self, intf_name):
        for ip in self.underlay_ips:
            if ip.Name == intf_name:
                return ip.IP
        return None

    def GetUnderlayMaskLen(self, intf_name):
        for ip in self.underlay_ips:
            if ip.Name == intf_name:
                return ip.MaskLen
        return None

    def GetUnderlayBGPRemoteASN(self, intf_name):
        for ip in self.underlay_ips:
            if ip.Name == intf_name:
                return getattr(ip, "BGPRemoteASN", None)
        return None

    def GetUnderlayNexthop(self, intf_name):
        for ip in self.underlay_ips:
            if ip.Name == intf_name:
                return getattr(ip, "Nexthop", None)
        return None

    @staticmethod
    def SetBatchClient(obj):
        EzAccessStore.batchClient = obj

    @staticmethod
    def GetBatchClient():
        return EzAccessStore.batchClient

    @staticmethod
    def SetDUTNode(node):
        EzAccessStore.dutNode = node

    @staticmethod
    def GetDUTNode():
        return EzAccessStore.dutNode

    @staticmethod
    def SetUuidMap(uuid_map):
        EzAccessStore.uuid_map = uuid_map

    @staticmethod
    def GetUuidMap(node):
        return EzAccessStore.uuid_map

    @staticmethod
    def SetConfigClientDict(obj):
        EzAccessStore.configClientDict = obj
        return

    @staticmethod
    def GetConfigClient(objType):
        return EzAccessStore.configClientDict.get(objType.name.lower(), None)

client = dict()
def Init(node):
    global client
    if not node in client:
        storeObj = EzAccessStore(node)
        client[node] = storeObj
