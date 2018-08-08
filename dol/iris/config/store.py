#! /usr/bin/python3
from infra.common.objects   import ObjectDatabase as ObjectDatabase
from infra.common.logging   import logger as logger
from infra.config.store     import ConfigStore as ConfigStore

class IrisConfigStore:
    def __init__(self):
        self.objects    = ConfigStore.objects
        self.templates  = ConfigStore.templates
        self.specs      = ConfigStore.specs
        
        # Custom Database for easy access.
        self.trunks = ObjectDatabase()
        self.tunnels = ObjectDatabase()
        return

    def SetTunnels(self, objs):
        return self.tunnels.SetAll(objs)
    def GetTunnelsVxlan(self):
        vxlan_tunnels = []
        for tun in self.tunnels.GetAllInList():
            if tun.IsVxlan(): vxlan_tunnels.append(tun)
        return vxlan_tunnels

    def GetTrunkingUplinks(self):
        return self.trunks.GetAllInList()
    def SetTrunkingUplinks(self, objs):
        return self.trunks.SetAll(objs)

Store = IrisConfigStore()
