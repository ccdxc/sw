#! /usr/bin/python3
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import cfglogger as cfglogger

class ConfigStore:
    def __init__(self):
        self.objects    = ObjectDatabase(cfglogger)
        self.templates  = ObjectDatabase(cfglogger)
        self.specs      = ObjectDatabase(cfglogger)
        
        # Custom Database for easy access.
        self.trunks = ObjectDatabase(cfglogger)
        self.tunnels = ObjectDatabase(cfglogger)
        return

    def SetTunnels(self, objs):
        return self.tunnels.SetAll(objs)
    def GetTunnelsVxlan(self):
        vxlan_tunnels = []
        for tun in self.tunnels.GetAll():
            if tun.IsVxlan(): vxlan_tunnels.append(tun)
        return vxlan_tunnels

    def GetTrunkingUplinks(self):
        return self.trunks.GetAll()
    def SetTrunkingUplinks(self, objs):
        return self.trunks.SetAll(objs)

Store = ConfigStore()
