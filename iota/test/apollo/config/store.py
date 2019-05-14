#! /usr/bin/python3
from infra.common.objects   import ObjectDatabase as ObjectDatabase
from infra.config.store     import ConfigStore as ConfigStore
from infra.common.logging   import logger as logger

class ApolloConfigStore:
    def __init__(self):
        self.objects    = ConfigStore.objects
        self.templates  = ConfigStore.templates
        self.specs      = ConfigStore.specs

        # Custom Database for easy access.
        self.trunks = ObjectDatabase()
        self.tunnels = ObjectDatabase()
        self.devices = ObjectDatabase()
        return

    def SetTunnels(self, objs):
        return self.tunnels.SetAll(objs)

    def AddTunnel(self, id, obj):
        return self.tunnels.Set(id, obj)

    def AddDevice(self, name, obj):
        self.devices.Set(name, obj)

    def GetDevice(self, name):
        return self.devices.Get(name)

    def GetDevices(self):
        return self.devices.GetAll()

    def IsDeviceEncapTypeMPLS(self,name):
        return self.devices.Get(name).IsEncapTypeMPLS()

    def IsDeviceEncapTypeVXLAN(self, name):
        return self.devices.Get(name).IsEncapTypeVXLAN()

    def GetDeviceEncapType(self, name):
        return self.devices.Get(name).EncapType

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

    def GetTrunkingUplinks(self):
        return self.trunks.GetAllInList()

    def SetTrunkingUplinks(self, objs):
        return self.trunks.SetAll(objs)

Store = ApolloConfigStore()
