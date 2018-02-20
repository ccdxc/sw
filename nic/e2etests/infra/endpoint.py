from infra.ns import NS


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
        if not ep_cfg["remote"]:
            self._encap_vlan = data["EnicObject"][ep_cfg["intf"]]["encap_vlan_id"]
        else:
            self._encap_vlan = None
        self._local = not ep_cfg["remote"]
        
    def Init(self):
        ns = NS(self._name)
        ns.BindInterface(self._name, self._encap_vlan if self._local else None)
        ns.SetMacAddress(self._mac_addr)
        if self._ip_address:
            ns.SetIpAddress(self._ip_address, self._prefix_len)
        self._ns = ns
    
    def Delete(self):
        self._ns.Delete()
    
    def Run(self, cmd, timeout=None):
        return self._ns.Run(cmd, timeout)
        
    def GetIp(self):
        return self._ip_address

    def GetMac(self):
        return self._mac_addr
    
    def PrintEpInformation(self):
        print("\tEP Information   : ", self._name)
        print("\t Namespace       : ", self._ns._name)
        print("\t Interface       : ", self._ns.GetInftName())
        print("\t IP address      : ", self._ip_address)
        print("\t Mac address     : ", self._mac_addr)
        print("\t Encap Vlan      : ", self._encap_vlan)
        print("\t Endpoint Type   : ", "Local" if self._local else "Remote")
        
if __name__ == "__main__":
    pass
    