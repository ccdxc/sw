import importlib
import json
import time

from infra.hntap import HntapFactory
from infra.ns import NS
from infra.endpoint import Endpoint
import consts

def Get_Ep_Pair(eps, src_ep_type, dst_ep_type):
    def __is_ep_remote(endpoint):
        return endpoint["spec"]["interface-type"] != "lif"

    host_eps = [entry for entry in eps if not __is_ep_remote(entry)] 
    remote_eps = [entry for entry in eps if __is_ep_remote(entry)]
    ep_pair_type = src_ep_type + "-" + dst_ep_type
    if ep_pair_type == "host-host":
        return host_eps[0], host_eps[1]
    elif ep_pair_type in [ "host-remote", "remote-host"]:
        return host_eps[0], remote_eps[0]
    elif ep_pair_type == "remote-remote":
        return remote_eps[0], remote_eps[1]
    else:
        assert(0)


def GenerateEpPairCfg(src_ep_type="host", dst_ep_type="remote"):
    data = json.load(open(consts.E2E_CFG_FILE))
    ep1, ep2 = Get_Ep_Pair(data["Endpoints"]["endpoints"], src_ep_type, dst_ep_type)
    return Endpoint(ep1["meta"]["name"], ep1, data), Endpoint(ep2["meta"]["name"], ep2, data) 


def SetUpNs(name, ep):
    namespace_name = "_".join([name, "NET" if ep["remote"] else "HOST"])
    print ("Bringing up namespace for EP %s %s %s" % (name, ep["macaddr"], ep["ipaddrs"][0]))
    ns = NS(namespace_name)
    ns.BindInterface(name, None if ep["remote"] else ep.get("encap_vlan") )
    ns.SetMacAddress(ep["macaddr"])
    ns.SetIpAddress(ep["ipaddrs"][0], 24)
    return ns
           
        
class E2eTest(object):

    def __init__(self, cfg, naples_container=None):
        self._name = cfg["name"]
        self._src_type = cfg["endpoint"]["src"]
        self._dst_type = cfg["endpoint"]["dst"]
        self._enabled = cfg["enabled"]
        self._src_ep = None
        self._dst_ep = None
        self._modules = cfg["modules"]
        self._naples_container = naples_container
    
    def __str__(self):
        return "E2E_TEST: %s" % self._name
    
    def __get_hntap_cfg(self, ep):
        return { "name" : ep._name, "local" : ep._local,
                 "port" : ep._port, "lif_id" : ep._lif_id
               }
        
    def BringUp(self, nomodel=False):
        self._src_ep, self._dst_ep = GenerateEpPairCfg(self._src_type,
                                        self._dst_type)
        
        hntap_cfg = [ self.__get_hntap_cfg(self._src_ep),
                      self.__get_hntap_cfg(self._dst_ep) ]
        with open(consts.HNTAP_CFG_FILE, "w") as fp:
            json.dump(hntap_cfg, fp)
        self._hntap = HntapFactory.Get(consts.HNTAP_CFG_FILE, container=self._naples_container)
        self._hntap.Run(nomodel)
        self.__configure_endpoints()


    def __configure_endpoints(self):
        self._src_ep.Init()
        self._dst_ep.Init()
            
        #Add Arp entries for now.
        self._src_ep._app.AddArpEntry(self._dst_ep.GetIp(), self._dst_ep.GetMac())
        self._dst_ep._app.AddArpEntry(self._src_ep.GetIp(), self._src_ep.GetMac())
        
    def __clean_up_endpoints(self):
        print ("Cleaning up endpoints...")
        self._src_ep.Delete()
        self._dst_ep.Delete()
        
    def Run(self):
        if not self._enabled:
            return True
        
        ret_code = True
        for module_info in self._modules:
            module = consts.modules_dir + "/" + module_info["module"]["program"]
            module = module.replace("/", ".")
            module = importlib.import_module(module)        
            print ("Running Test :", module_info["module"]["name"])
            self.BringUp()
            self.PrintEnvironmentSummary()
            run = getattr(module, "Run")
            ret = run(self._src_ep, self._dst_ep)
            if (ret):
                print ("Test Passed")
            else:
                print ("Test Failed")
                ret_code = False
            self.Teardown()
        return ret_code
        
        
    def PrintEnvironmentSummary(self):
        print ("***********  Source EP information ********")
        self._src_ep.PrintEpInformation()
        print ("************ Destination EP information *******")
        self._dst_ep.PrintEpInformation()
        
    def Teardown(self):
        self.__clean_up_endpoints()
        self._hntap.Stop()
        time.sleep(2)

class E2eEnv(object):

    def __init__(self, cfg_file):
        self._cfg = json.load(open(cfg_file))
        self._eps = []
        self.__parse_endpoints()
    
    def __get_lif_oport(self, intf_name):
        for intf in self._cfg["Interfaces"]:
            if intf["meta"]["name"] == intf_name:
                if intf["spec"]["type"] == "UPLINK":
                    for uplink_intf in self._cfg["HalInterfaces"]:
                        if uplink_intf["spec"]["key_or_handle"]["KeyOrHandle"]["InterfaceId"] == intf["status"]["id"]:
                            return 0, uplink_intf["spec"]["IfInfo"]["IfUplinkInfo"]["port_num"], uplink_intf["status"].get("encap_vlan", 0)
                elif intf["spec"]["type"] == "ENIC":
                    for lif in self._cfg["HalLifs"]:
                        if lif["spec"]["key_or_handle"]["KeyOrHandle"]["LifId"] == intf["status"]["id"]:
                            return lif["status"]["hw_lif_id"], 0, intf["status"].get("encap_vlan", 0)
                else:
                    assert(0)
        print ("Interface information not found for ", intf_name)
        assert(0)
        
        
    def __parse_endpoints(self):
        for endpoint in self._cfg["Endpoints"]:
            name = endpoint["meta"]["Name"]
            lif, oport, encap_vlan = self.__get_lif_oport(endpoint["status"]["Interface"])
            ep_cfg = { "EndpointObject" : { name : {"remote" : lif <= 0 , "intf" : endpoint["status"]["Interface"]} },
                       "UplinkObject" : { endpoint["status"]["Interface"] : { "port" : oport }}, 
                       "EnicObject" : { endpoint["status"]["Interface"] :
                                        { "lif_id" : lif ,
                                          "encap_vlan_id" : encap_vlan
                                         } },   }
            ep_cfg_data = ep_cfg["EndpointObject"][name]
            ep_cfg_data["macaddr"] = endpoint["status"]["MacAddress"]
            ip_addr, prefix_len = endpoint["status"]["IPv4Address"].split("/")
            ep_cfg_data["ipaddrs"] = [ ip_addr ]
            ep = Endpoint(name, ep_cfg)
            self._eps.append(ep)
    
    def __get_hntap_configuration(self):
        ep_cfgs = []
        for ep in self._eps:
            ep_cfg = { "name" : ep._name,  "local" : ep._local,  "port" : ep._port, "lif_id" : int(ep._lif_id)}
            ep_cfgs.append(ep_cfg)
        return ep_cfgs
    
    def BringUp(self, nomodel=False):
        self._ep_cfgs = self.__get_hntap_configuration()
        with open(consts.HNTAP_CFG_FILE, "w") as fp:
            json.dump(self._ep_cfgs, fp)
        self._hntap = Hntap(consts.HNTAP_CFG_FILE)
        self._hntap.Run(nomodel)
        self.__configure_endpoints()


    def __setup_arp_entries(self):
        print ("Adding ARP entries for endpoints...")
        #TODO , this should be smarter.
        for ep in self._eps:
            for other_ep in self._eps:
                if other_ep == ep:
                    continue
                ep._app.AddArpEntry(other_ep.GetIp(), other_ep.GetMac())
                
    def __configure_endpoints(self):
        for ep  in self._eps:
            ep.Init()
        #Add Arp entries for now.
        self.__setup_arp_entries()
        
    def __clean_up_endpoints(self):
        print ("Cleaning up endpoints...")
        for ep in self._eps:
            ep.Delete()
        
        
    def PrintEnvironmentSummary(self):
        for ep in self._eps:
            print ("***********  EP information ********")
            ep.PrintEpInformation()
        
    def Teardown(self):
        self.__clean_up_endpoints()
        self._hntap.Stop()
        time.sleep(2)
