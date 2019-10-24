#! /usr/bin/python3
from infra.common.objects   import ObjectDatabase as ObjectDatabase
from infra.common.logging   import logger as logger
from infra.config.store     import ConfigStore as ConfigStore

class ApolloConfigStore:
    def __init__(self):
        self.objects    = ConfigStore.objects
        self.templates  = ConfigStore.templates
        self.specs      = ConfigStore.specs

        # Custom Database for easy access.
        self.trunks = ObjectDatabase()
        self.tunnels = ObjectDatabase()
        self.device = None
        self.substrate_vpc = None
        # Batch client
        self.batchClient = None
        self.hostport = None
        self.switchport = None
        return

    def SetBatchClient(self, obj):
        self.batchClient = obj

    def GetBatchClient(self):
        return self.batchClient

    def SetTunnels(self, objs):
        return self.tunnels.SetAll(objs)

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

    def SetSubstrateVPC(self, obj):
        self.substrate_vpc = obj

    def GetSubstrateVPCId(self):
        if self.substrate_vpc:
            return self.substrate_vpc.VPCId
        else:
            return -1

    def GetProviderIPAddr(self, count):
        if self.substrate_vpc:
            return self.substrate_vpc.GetProviderIPAddr(count)
        else:
            return None,-1

    def GetSvcMapping(self, ipversion):
        if self.substrate_vpc:
            return self.substrate_vpc.GetSvcMapping(ipversion)
        else:
            return None,-1

    def IsDeviceEncapTypeMPLS(self):
        return self.device.IsEncapTypeMPLS()

    def IsDeviceEncapTypeVXLAN(self):
        return self.device.IsEncapTypeVXLAN()

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

    def GetTrunkingUplinks(self):
        return self.trunks.GetAllInList()

    def SetTrunkingUplinks(self, objs):
        return self.trunks.SetAll(objs)

Store = ApolloConfigStore()
