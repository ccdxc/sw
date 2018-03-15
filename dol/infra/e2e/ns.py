import os
import subprocess
import sys

def run(cmd, timeout=None, background=False):
    print ("Running : ", cmd)
    if background:
        p = subprocess.Popen(cmd, shell=True)
        return p 
    else:
        try:
            p = subprocess.run(cmd, shell=True, timeout=timeout)
        except subprocess.TimeoutExpired:
            print ("timed out : ", cmd)
            return False
        if p.returncode:
            print ("Command exit code : ", p.returncode)
            return False
    
    print ("Success : ", cmd)
    return True
        
class NS(object):
    
    def __init__(self, name):
        self._name = name
        self._intf = None
        self._vlan_intf = None
        self._cmd_prefix = "ip netns exec %s" % self._name
        self.Reset()
    
    def Init(self):
        #Create name space
        cmd = "ip netns add %s" % self._name
        run(cmd)
        

    def BindInterface(self, interface, vlan=None):
        self._intf = interface
        cmd = "ip link set dev %s netns %s" % (self._intf, self._name)
        run(cmd)
        cmd = self._cmd_prefix + " ifconfig %s up" % (self._intf)
        run(cmd)
        if (vlan):
            self._vlan_intf = self._intf + "_" + str(vlan)
            cmd =  self._cmd_prefix + \
             " ip link add link %s name %s type vlan id %s" \
             % (self._intf, self._vlan_intf, str(vlan))
            run(cmd)
            cmd = self._cmd_prefix + " ifconfig %s up" % (self._vlan_intf)
            run(cmd)
            
    
    def UnBindInterface(self):
        pass

    def SetMacAddress(self, mac_address):
         cmd =  self._cmd_prefix + " ifconfig %s hw ether %s" % (self._intf, mac_address)
         run(cmd)
         if (self._vlan_intf):
             cmd =  self._cmd_prefix + " ifconfig %s hw ether %s" % (self._vlan_intf, mac_address)
             run(cmd)
    
    def SetIpAddress(self, ipaddress, prefixlen):
        cmd = self._cmd_prefix + " ifconfig %s %s/%d" % (self._vlan_intf or self._intf,
                                                          ipaddress, int(prefixlen))
        run(cmd)
    
    def AddArpEntry(self, ipaddress, mac_addr):
        cmd = self._cmd_prefix + " arp -s %s  %s" % (ipaddress, mac_addr)
        run(cmd)
        
    def SetDefaultRoute(self, ipaddress):
        pass
        
    def Reset(self):
        try:
            self.Delete()
        except:
            pass
        self.Init()
        
    def Run(self, cmd, timeout=None, background=False):
        cmd = self._cmd_prefix + " " + cmd
        return run(cmd, timeout, background)
    
    def Delete(self):
        #Delete name space
        cmd = "ip netns del %s" % self._name
        run(cmd)
    
    def GetInftName(self):
        return self._vlan_intf or self._intf

if __name__ == "__main__":
    ns = NS("ns0")
    ns.BindInterface("hntap_host0")
    ns.SetMacAddress("8:00:00:00:00:01")
    ns.SetIpAddress("2.2.2.2", 24)
    ns.Run("ifconfig")
    try:
        import time
        time.sleep(100000)
    except:
        pass
    ns.Delete()