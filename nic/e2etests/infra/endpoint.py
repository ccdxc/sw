import re
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
           

def _get_id_from_string(string):
    patterns= [r'\d+']
    for p in patterns:
        match= re.findall(p, string)
        return int(match[0])

def _is_ep_remote(endpoint):
    return endpoint["spec"]["interface-type"] != "lif"

def _get_network_from_config(nw_name, full_cfg):
    for network in full_cfg["Networks"]["networks"]:
        if network["meta"]["name"] == nw_name:
            return network
           
class Endpoint:
    
    def __init__(self, name, data, full_cfg):
        self._name = name
        ep_cfg = data
        self._mac_addr = ep_cfg["spec"]["mac-address"]
        self._ip_address = ep_cfg["spec"]["ipv4-address"].split("/")[0]
        self._prefix_len = 24 
        self._lif_id = 0
        self._port = 0
        if not _is_ep_remote(ep_cfg):
            self._encap_vlan = ep_cfg["spec"]["useg-vlan"] 
            self._lif_id = _get_id_from_string(ep_cfg["spec"]["interface"])
        else:
            network = _get_network_from_config(ep_cfg["spec"]["network-name"], full_cfg)
            self._encap_vlan = network["spec"].get("vlan-id")
            self._port = _get_id_from_string(ep_cfg["spec"]["interface"])
        self._local = not _is_ep_remote(ep_cfg)


    def Init(self, id=None):
        app = AppFactory.Get(self._name, id=id)
        app.AttachInterface(self._name)
        app.SetMacAddress(self._name, self._mac_addr)
        if self._encap_vlan:
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
    