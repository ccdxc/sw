import os
import subprocess
import sys
from infra.app import AppEngine

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
        if p.returncode:
            print ("Command exit code : ", p.returncode)
            return False
    
    print ("Success : ", cmd)
    return True

class Interface(object):
    
    def __init__(self, name, vlan=None, mac_address=None):
        self._name = name
        self._vlan = vlan
        self._mac_address = mac_address
        self._vlan_intfs = {}
        
    def AddVlanInterface(self, name, vlan):
        intf = Interface(name, vlan)
        self._vlan_intfs[vlan] = intf
    
    def GetVlanInterface(self, vlan):
        return self._vlan_intfs.get(vlan, None)
    
    def DeleteVlanInterface(self, vlan):
        if vlan in self._vlan_intfs:
            del self._vlan_intfs[vlan]
    
class NS(AppEngine):
    
    def __init__(self, name):
        self._name = name
        self._cmd_prefix = "ip netns exec %s" % self._name
        self._interfaces = {}
    
    def Init(self):
        #Create name space
        self.Reset()
        cmd = "ip netns add %s" % self._name
        run(cmd)
        

    def AddVlan(self, interface, vlan, name=None):
        intf = self._interfaces.get(interface)
        if not intf:
            raise Exception("Primary Interface not found!")
        vlan_intf = name if name else intf._name + "_" + str(vlan)
        intf.AddVlanInterface(vlan_intf, vlan)
        cmd =  self._cmd_prefix + \
         " ip link add link %s name %s type vlan id %s" \
         % (intf._name, vlan_intf, str(vlan))
        run(cmd)
        cmd = self._cmd_prefix + " ifconfig %s up" % (vlan_intf)
        run(cmd)

    def DeleteVlan(self, interface, vlan):
        intf = self._interfaces.get(interface, None)
        if not intf:
            raise("Primary Interface not found")
        vlan_intf = intf.GetVlanInterface(vlan)
        if not vlan_intf:
            raise("Vlan Interface not found")        
        cmd =  self._cmd_prefix + \
         " ip link delete dev %s" \
         % (vlan_intf._name)
        run(cmd)
        intf.DeleteVlanInterface(vlan)
    
    def AttachInterface(self, interface):
        intf = Interface(interface)
        cmd = "ip link set dev %s netns %s" % (interface, self._name)
        run(cmd)
        cmd = self._cmd_prefix + " ifconfig %s up" % (interface)
        run(cmd)
        self._interfaces[interface] = intf
    
    def DetachInterface(self, interface):
        intf = self._interfaces.get(interface, None)
        if not intf:
            return
        run("ip netns exec " + self._name + " ip link set " + interface + " netns 1")
        run("ifconfig " + interface + " up")
        del self._interfaces[interface]


    def DetachInterfaces(self):
        for intf in list(self._interfaces.keys()):
            self.DetachInterface(intf)

    def SetMacAddress(self, interface, mac_address, vlan=None):
        intf = self._interfaces.get(interface, None)
        if not intf:
            raise("Primary Interface not found")
        if vlan:
            intf = intf.GetVlanInterface(vlan)
            if not intf:
                raise("Vlan Interface not found")
            
        cmd =  self._cmd_prefix + " ifconfig %s hw ether %s" % (intf._name, mac_address)
        run(cmd)
 
    def SetIpAddress(self, interface, ipaddress, prefixlen, vlan=None):
        intf = self._interfaces.get(interface, None)
        if not intf:
            raise("Primary Interface not found")
        if vlan:
            intf = intf.GetVlanInterface(vlan)
            if not intf:
                raise("Vlan Interface not found")
        cmd = self._cmd_prefix + " ifconfig %s %s/%d" % (intf._name,
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
        
    def Run(self, cmd, timeout=None, background=False):
        cmd = self._cmd_prefix + " " + cmd
        return run(cmd, timeout, background)
    
    def Delete(self):
        #Delete name space
        self.UnBindInterfaces()
        cmd = "ip netns del %s" % self._name
        run(cmd)
    
    def GetInftName(self):
        return self._vlan_intf or self._intf

if __name__ == "__main__":
    ns = NS("ns0")
    ns.Reset()
    ns.Init()
    ns.BindInterface("EP0007")
    ns.SetMacAddress("EP0007", "8:00:00:00:00:01")
    ns.SetIpAddress("EP0007", "2.2.2.2", 24)
    ns.Run("ifconfig")
    try:
        import time
        time.sleep(100000)
    except:
        pass
    ns.Delete()