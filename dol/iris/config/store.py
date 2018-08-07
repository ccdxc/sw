#! /usr/bin/python3
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger as logger

class ConfigStore:
    def __init__(self):
        self.objects    = ObjectDatabase()
        self.templates  = ObjectDatabase()
        self.specs      = ObjectDatabase()
        
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

Store = ConfigStore()
