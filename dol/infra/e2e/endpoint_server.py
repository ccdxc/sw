#! /usr/bin/python3
import sys
import os
import logging

from infra.e2e.ns import NS
from jsonrpc2_zeromq import RPCServer
import argparse
from datetime import datetime
from pip._vendor.colorama.ansi import Back

class Endpoint:
    
    def __init__(self, name, mac_addr, ip_addr,
                 local = True, encap_vlan=None):
        self._name = name
        self._mac_addr = mac_addr
        self._ip_address = ip_addr
        self._prefix_len = 24
        self._encap_vlan = encap_vlan
        self._local = local
        self._process = {}
        
    def Init(self):
        ns = NS(self._name)
        ns.BindInterface(self._name, self._encap_vlan if self._local else None)
        ns.SetMacAddress(self._mac_addr)
        if self._ip_address:
            ns.SetIpAddress(self._ip_address, self._prefix_len)
        self._ns = ns
    
    def Delete(self):
        self.CleanUp()
        self._ns.Delete()
    
    def CleanUp(self):
        for pid in self._process:
            self._process[pid].kill() 
        self._process = {}       
        
    def Run(self, cmd, timeout=None, background=False):
        ret = self._ns.Run(cmd, timeout, background)
        if background:
            self._process[id(ret)] = ret
            ret = id(ret)
        return ret
    
    def AddArpEntry(self, ip, mac):
         self._ns.AddArpEntry(ip, mac)
        
    def ProcessKill(self, pid):
        if self._process.get(pid, None):
            self._process[pid].kill()
        
    def GetIp(self):
        return self._ip_address.get()

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

class EndpointServer(RPCServer):
    
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._endpoints = {}
        
    def Start(self):
        self.run()
        
    def handle_echo_method(self, msg):
        #Return the message as is
        return msg
    
    def handle_init_method(self, name, mac_addr, ip_addr,
                 local = True, encap_vlan=None):
        resp = {}
        self.handle_reset_method(name)
        self._endpoints[name] = Endpoint(name, mac_addr, ip_addr, local, encap_vlan)
        self._endpoints[name].Init()
        return { "status" : "ok"}

    def handle_reset_method(self, name = None):
        if name:
            if self._endpoints.get(name, None):
                self._endpoints[name].Delete()
        else:
            for ep_name in self._endpoints:
                self._endpoints[ep_name].Delete()

    def handle_process_run_method(self, name, cmd, timeout=None, background=False):
        if self._endpoints.get(name, None):
            return self._endpoints[name].Run(cmd, timeout, background)        
    
    def handle_process_kill_method(self, name, pid):
        if self._endpoints.get(name, None):
            self._endpoints[name].ProcessKill(pid)

    def handle_cleanup_method(self, name):
        if self._endpoints.get(name, None):
            self._endpoints[name].CleanUp()
                
    def handle_add_arp_entry_method(self, name, ip, mac):
        if self._endpoints.get(name, None):
            self._endpoints[name].AddArpEntry(ip, mac)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', dest='port',
                        default=None, help='Server Port to start')
    args = parser.parse_args()
    ep = EndpointServer("tcp://127.0.0.1:" + args.port)
    ep.Start()