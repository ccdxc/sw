import importlib

from infra.hntap import Hntap
from infra.ns import NS
from infra.endpoint import Endpoint
import consts
import json


def Get_Ep_Pair(eps, src_ep_type, dst_ep_type):
    host_eps = [ (entry[0] , entry[1] ) for entry in eps if not entry[1]["remote"]] 
    remote_eps = [ (entry[0], entry[1] ) for entry in eps if entry[1]["remote"]]
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
    
    def get_lif_oport(ep):
        if ep["remote"]:
            return 0, data["UplinkObject"][ep["intf"]]["port"]
        else:
            return data["LifObject"][data["EnicObject"][ep["intf"]]["lif"]]["hw_lif_id"], 0
        
    data = json.load(open(consts.DOL_CFG_FILE))
    ep_pair_cfg = []
    for ten_id, ten in data["TenantObject"].items():
        if ten["type"] == "TENANT":
            eps = [ (ep_name, ep) for ep_name, ep in data["EndpointObject"].items() if ep["tenant"] == ten_id ]
            ep1, ep2 = Get_Ep_Pair(eps, src_ep_type, dst_ep_type)
            ep1_lif , ep1_oport = get_lif_oport(ep1[1])
            ep2_lif , ep2_oport = get_lif_oport(ep2[1])
            cfg = { "src" : { "name" : ep1[0], "local" : not ep1[1]["remote"], "port" : ep1_oport,
                               "lif_id" : ep1_lif},
                    "dst" : { "name" : ep2[0], "local" : not ep2[1]["remote"], "port" : ep2_oport,
                             "lif_id" : ep2_lif }
                   }
            ep_pair_cfg.append(cfg)
            #For now, returning after 1 pair creation.
            return ep_pair_cfg
        
def SetUpNs(name, ep):
    namespace_name = "_".join([name, "NET" if ep["remote"] else "HOST"])
    print ("Bringing up namespace for EP %s %s %s" % (name, ep["macaddr"], ep["ipaddrs"][0]))
    ns = NS(namespace_name)
    ns.BindInterface(name, None if ep["remote"] else ep.get("encap_vlan") )
    ns.SetMacAddress(ep["macaddr"])
    ns.SetIpAddress(ep["ipaddrs"][0], 24)
    return ns
           
        
class E2eTest(object):

    def __init__(self, cfg):
        self._name = cfg["name"]
        self._src_type = cfg["endpoint"]["src"]
        self._dst_type = cfg["endpoint"]["dst"]
        self._enabled = cfg["enabled"]
        self._src_ep = None
        self._dst_ep = None
        self._modules = cfg["modules"]
    
    def __str__(self):
        return "E2E_TEST: %s" % self._name
        
    def BringUp(self):
        self._ep_pair_cfg = GenerateEpPairCfg(self._src_type,
                                                  self._dst_type)
        with open(consts.HNTAP_CFG_FILE, "w") as fp:
            json.dump(self._ep_pair_cfg, fp)
        self._hntap = Hntap(consts.HNTAP_CFG_FILE)
        self._hntap.Run()
        self.__configure_endpoints()


    def __configure_endpoints(self):
        data = json.load(open(consts.DOL_CFG_FILE))
        for ep_pair in self._ep_pair_cfg:
            
            src_name = ep_pair["src"]["name"]
            self._src_ep = Endpoint(src_name, data)
            self._src_ep.Init()
            
            dst_name = ep_pair["dst"]["name"]
            self._dst_ep = Endpoint(dst_name, data)
            self._dst_ep.Init()
            
            #Add Arp entries for now.
            self._src_ep._ns.AddArpEntry(self._dst_ep.GetIp(), self._dst_ep.GetMac())
            self._dst_ep._ns.AddArpEntry(self._src_ep.GetIp(), self._src_ep.GetMac())
        
    def __clean_up_endpoints(self):
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
            run = getattr(module, "Run")
            ret = run(self._src_ep, self._dst_ep)
            if (ret):
                print ("Test Passed")
            else:
                print ("Test Failed")
                ret_code = False
                
            teadown = getattr(module, "Teardown")
            teadown(self._src_ep, self._dst_ep)
        return ret_code
        
        
    def PrintEnvironmentSummary(self):
        print ("***********  Source EP information ********")
        self._src_ep.PrintEpInformation()
        print ("************ Destination EP information *******")
        self._dst_ep.PrintEpInformation()
        
    def Teardown(self):
        self.__clean_up_endpoints()
        self._hntap.Stop()