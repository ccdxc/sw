from infra.ns import NS
from infra.app import AppFactory


def SetUpNs(name, ep):
    namespace_name = "_".join([name, "NET" if ep["remote"] else "HOST"])
    print ("Bringing up namespace for EP %s %s %s" % (name, ep["macaddr"], ep["ipaddrs"][0]))
    ns = NS(namespace_name)
    ns.BindInterface(name, None if ep["remote"] else ep.get("encap_vlan"))
    ns.SetMacAddress(ep["macaddr"])
    ns.SetIpAddress(ep["ipaddrs"][0], 24)
    return ns
           
class Endpoint:
    
    def __init__(self, name, data):
        self._name = name
        ep_cfg = data["EndpointObject"][name]
        self._mac_addr = ep_cfg["macaddr"]
        self._ip_address = ep_cfg["ipaddrs"][0]
        self._prefix_len = 24
        self._lif_id = 0
        self._port = 0
        if not ep_cfg["remote"]:
            self._encap_vlan = data["EnicObject"][ep_cfg["intf"]]["encap_vlan_id"]
            self._lif_id = data["EnicObject"][ep_cfg["intf"]]["lif_id"]
        else:
            self._encap_vlan = None
            self._port = data["UplinkObject"][ep_cfg["intf"]]["port"]
        self._local = not ep_cfg["remote"]


    def Init(self, id=None):
        app = AppFactory.Get(self._name, id=id)
        app.AttachInterface(self._name)
        app.SetMacAddress(self._name, self._mac_addr)
        if self._local:
            app.AddVlan(self._name, self._encap_vlan)
            app.SetMacAddress(self._name, self._mac_addr, self._encap_vlan)
        if self._ip_address:
            app.SetIpAddress(self._name, self._ip_address, self._prefix_len, self._encap_vlan)
        self._app = app
    
    def Delete(self):
        print ("Cleaning up endpoint : %s" % self._name)
        self._app.Stop()
    
    def Run(self, cmd, timeout=None, background=False):
        return self._app.RunCommand(cmd, timeout, background)
        
    def GetIp(self):
        return self._ip_address

    def GetMac(self):
        return self._mac_addr
    
    def PrintEpInformation(self):
        print("\t Endpoint Type      : ", "Local" if self._local else "Remote")
        self._app.PrintAppInformation()
        
if __name__ == "__main__":
    pass
    