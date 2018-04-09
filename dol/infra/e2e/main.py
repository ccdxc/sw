import time
import json
import os
from infra.e2e.hntap import Hntap
from infra.e2e.endpoint_manager import EndpointManager
import infra.common.glopts as glopts

EpMgr = None

def __GenerateEpCfgs(cfg_file):
    
    def get_lif_oport(ep):
        if ep["remote"]:
            return 0, data["UplinkObject"][ep["intf"]]["port"]
        else:
            return data["LifObject"][data["EnicObject"][ep["intf"]]["lif"]]["hw_lif_id"], 0
    
    data = json.load(open(cfg_file))
    ep_cfgs = []
    for ten_id, ten in data["TenantObject"].items():
        if ten["type"] == "TENANT":
            eps = [ (ep_name, ep) for ep_name, ep in data["EndpointObject"].items() if ep["tenant"] == ten_id ]
            for ep_name, ep in eps:
                ep_lif , ep_oport = get_lif_oport(ep)
                cfg = { "name" : ep_name, "local" : not ep["remote"], "port" : ep_oport,
                        "lif_id" : ep_lif, "macaddr" : ep["macaddr"], "ipaddr" : ep["ipaddrs"][0],
                        "prefix_len" : 24, "intf_name" : ep["intf"],
                         "encap_vlan" : data["EnicObject"][ep["intf"]]["encap_vlan_id"] 
                                        if not ep["remote"] else None}
                ep_cfgs.append(cfg)
                
    return ep_cfgs

def __start_htnap(cfg_file):
    global hntap_hdl
    hntap_hdl = Hntap(cfg_file)
    hntap_hdl.Run()   

def __stop_hntap():
    hntap_hdl.Stop()

def Start(dol_cfg_file):
    ep_cfgs = __GenerateEpCfgs(dol_cfg_file)
    e2e_cfg_file = os.environ['WS_TOP']  + '/nic/' + glopts.GlobalOptions.e2ecfg
    with open(e2e_cfg_file, "w") as fp:
        json.dump(ep_cfgs, fp)    
    __start_htnap(e2e_cfg_file)
    global EpMgr
    EpMgr = EndpointManager(ep_cfgs)
    EpMgr.BringUpEndpoints()
    EpMgr.ConfigureEndpoints()
    
def Stop():
    EpMgr.TearDownEndpoints()
    __stop_hntap()

def RunCommand(ep_name, cmd, background=False, timeout=None):
    return EpMgr.Run(ep_name, cmd, background=background, timeout=timeout)

def CopyFile(ep_name, file):
    return EpMgr.CopyFile(ep_name, file)

def CleanUp(ep_name):
    return EpMgr.CleanUp(ep_name)

if __name__ == "__main__":
    Start("../nic/conf/dol.conf")
    time.sleep(5)
    Stop()