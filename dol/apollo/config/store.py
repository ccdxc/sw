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
        return

    def SetTunnels(self, objs):
        return self.tunnels.SetAll(objs)

    def SetDevice(self,obj):
        self.device = obj

    def GetDevice(self):
        return self.device

    def IsDeviceEncapTypeMPLS(self):
        return self.device.IsEncapTypeMPLS()

    def IsDeviceEncapTypeVXLAN(self):
        return self.device.IsEncapTypeVXLAN()

    def GetWorkloadTunnels(self):
        tunnels = []
        for tun in self.tunnels.GetAllInList():
            if tun.IsWorkload(): tunnels.append(tun)
        return tunnels

    def GetIgwTunnels(self):
        tunnels = []
        for tun in self.tunnels.GetAllInList():
            if tun.IsIgw(): tunnels.append(tun)
        return tunnels

    def GetTrunkingUplinks(self):
        return self.trunks.GetAllInList()
    def SetTrunkingUplinks(self, objs):
        return self.trunks.SetAll(objs)

Store = ApolloConfigStore()
