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
        self.switch = None
        return

    def SetTunnels(self, objs):
        return self.tunnels.SetAll(objs)

    def SetSwitch(self,obj):
        self.switch = obj

    def GetSwitch(self):
        return self.switch

    def GetTunnelsVxlan(self):
        vxlan_tunnels = []
        for tun in self.tunnels.GetAllInList():
            if tun.IsVxlan(): vxlan_tunnels.append(tun)
        return vxlan_tunnels

    def GetTunnelsMplsOverUdp2(self):
        mpls_tunnels = []
        for tun in self.tunnels.GetAllInList():
            if tun.IsMplsOverUdp2(): mpls_tunnels.append(tun)
        return mpls_tunnels

    def GetTunnelsMplsOverUdp1(self):
        mpls_tunnels = []
        for tun in self.tunnels.GetAllInList():
            if tun.IsMplsOverUdp1(): mpls_tunnels.append(tun)
        return mpls_tunnels

    def GetTrunkingUplinks(self):
        return self.trunks.GetAllInList()
    def SetTrunkingUplinks(self, objs):
        return self.trunks.SetAll(objs)

Store = ApolloConfigStore()
