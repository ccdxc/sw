#! /usr/bin/python3
import sys
import os
import time
from subprocess import Popen
from jsonrpc2_zeromq import RPCClient

RPC_EP_PORT_LISTENER_START = 9999
endpoint_start_cmd = "infra/e2e/endpoint_server.py"

class EndpointManager(object):
    
    class Endpoint(object):
        def __init__(self, port):
            self._port = port
            self._process_hdl = None
            self._connector = None

        def __start_endpoint(self):
            cmd = [endpoint_start_cmd, "--port", str(self._port)]
            env = os.environ.copy()
            env["PYTHONPATH"] = os.curdir
            p = Popen(cmd, env=env)
            print ("* Starting Endpoint listener on port : " + str(self._port) + ", pid(" + str(p.pid) + ")")
            return p
        
        def Start(self):
            self._process_hdl = self.__start_endpoint()
            self._connector = RPCClient('tcp://localhost:{}'.format(self._port), timeout=100000)
        
        def Run(self, name, cmd, timeout=None, background=False):
            return self._connector.process_run(name, cmd, timeout, background)
 
        def CleanUp(self, name):
            return self._connector.cleanup(name)
        
        def AddArpEntry(self, name, ip, mac):
            self._connector.add_arp_entry(name, ip, mac)
        
        def Stop(self):
            self._connector.reset()
            self._process_hdl.kill()
                
    def  __init__(self, e2e_spec):
        self._num_eps = len(e2e_spec)
        self._spec = e2e_spec
        self._eps = []
        self._ep_map = {}


    def BringUpEndpoints(self):
        print ("Bringing up endpoints...")
        for num in range(0, self._num_eps):
            ep = self.Endpoint(RPC_EP_PORT_LISTENER_START + num)
            ep.Start()
            self._eps.append(ep)
        try:
            self.PingEndpoints()
        except Exception as ex:
            raise ex

    def __configure_endpoint(self, ep_cfg, ep):
        ep._connector.init(ep_cfg["name"], ep_cfg["macaddr"],
                           ep_cfg["ipaddr"], ep_cfg["local"],
                           ep_cfg["encap_vlan"])
        self._ep_map[ep_cfg["name"]] = ep
    
    def __setup_arp_entries(self):
        print ("Adding ARP entries for endpoints...")
        #TODO , this should be smarter.
        for ep_cfg in self._spec:
            for ep_name in self._ep_map:
                if ep_cfg["name"] == ep_name:
                    continue
                self._ep_map[ep_name].AddArpEntry(ep_name, ep_cfg["ipaddr"], 
                                                    ep_cfg["macaddr"])
    
    def ConfigureEndpoints(self):
        print ("Configuring endpoints...")
        for ep_cfg, ep in zip(self._spec, self._eps):
            self.__configure_endpoint(ep_cfg, ep)
        self.__setup_arp_entries()
    
    def TearDownEndpoints(self):
        print ("Tearing down endpoints...")
        for ep in self._eps:
            ep.Stop()

    def PingEndpoints(self):
        print ("Pinging Endpoints...")
        for ep in self._eps:
            try:
                msg =  (ep._connector.Echo("Hello World"))
                print ("Success pinging EP, running on port %s" % ep._port)
            except Exception as ex:
                print ("Problem pinging EP, running on port %s" % ep._port )
                raise ex

    def Run(self, ep_name, cmd, timeout=None, background=False):
        print ("Running Command on EP : %s , command : %s, timeout : %s" % (ep_name, cmd, str(timeout)))
        try:
            return self._ep_map[ep_name].Run(ep_name, cmd, timeout, background)
        except Exception as ex:
            print ("Failed : Running Command on EP : %s , command : %s " % (ep_name, cmd))
            print (ex)
            return False

    def CleanUp(self, ep_name):
        print ("Running Clean up  Command on EP : %s" % (ep_name))
        try:
            return self._ep_map[ep_name].CleanUp(ep_name)
        except Exception as ex:
            print (ex)
    
if __name__ == "__main__":
    ep_mgr = EndpointManager("../nic/conf/dol.conf")
    ep_mgr.BringUpEndpoints()
    ep_mgr.PingEndpoints()
    time.sleep(5)
    ep_mgr.TearDownEndpoints()
