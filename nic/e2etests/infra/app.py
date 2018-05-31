#!/usr/bin/python3
import os
import subprocess
import sys
import time
import docker
import json
import pdb
import consts

DockerEnvClient = docker.from_env()
DockerApiClient = docker.APIClient(base_url='unix://var/run/docker.sock')

class AppEngine(object):

    def __init__(self, name, image):
        self._name = name
        
    def BringUp(self):
        pass
    
    def RunCommand(self, cmd, timeout=None, background=False):
        pass
    
    def AttachInterface(self, interface_name):
        pass
    
    def AddVlan(self, interface, vlan):
        pass
        
    def DeleteVlan(self, interface, vlan):
        pass

    def DetachInterface(self, interface_name):
        pass
            
    def DetachInterfaces(self):
        pass
    
    def SetMacAddress(self, interface, mac_address, vlan=None):
        pass

    def SetIpAddress(self, interface, ipaddress, prefixlen, vlan=None):
        pass
        
    def Stop(self):
        pass
    
    def PrintAppInformation(self):
        pass
    
def run(cmd, timeout=None, background=False, stdout=None):
    print ("Running : ", cmd)
    if background:
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
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

def clean_up_container(name):
    try:
        DockerApiClient.stop(name)
        print("Stopped previously running container")
    except:
        print("Container was not running before.")
    try:
        DockerApiClient.remove_container(name)
        print("Removed previously running container")
    except:
        print("Container image was not found before.")

class Interface(object):
    
    def __init__(self, name, vlan=None, mac_address=None):
        self._name = name
        self._vlan = vlan
        self._mac_address = mac_address
        self._ip_address = "0.0.0.0"
        self._prefix_len = 0
        self._vlan_intfs = {}
        
    def AddVlanInterface(self, name, vlan):
        intf = Interface(name, vlan)
        self._vlan_intfs[vlan] = intf
    
    def GetVlanInterface(self, vlan):
        return self._vlan_intfs.get(vlan, None)
    
    def DeleteVlanInterface(self, vlan):
        if vlan in self._vlan_intfs:
            del self._vlan_intfs[vlan]

    def PrintInterfaceInformation(self):
        print("\t\t Intetrface name     : ", self._name)
        print("\t\t Mac address         : ", self._mac_address)
        print("\t\t Encap Vlan          : ", self._vlan)
        print("\t\t IP Address          : ", self._ip_address)
        for intf_name in self._vlan_intfs:
            self._vlan_intfs[intf_name].PrintInterfaceInformation()
    
    def SetIpAddress(self, address, prefix_len):
        self._ip_address = address
        self._prefix_len = prefix_len
        
class NS(AppEngine):
    
    def __init__(self, name, config_file=None, id=None, skip_init=False):
        self._name = name
        self._cmd_prefix = "ip netns exec %s" % self._name
        self._interfaces = {}
        if not skip_init:
            self.Init()
    
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
        intf._mac_address = mac_address
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
        intf.SetIpAddress(ipaddress, prefixlen)
    
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
        
    def RunCommand(self, cmd, timeout=None, background=False):
        cmd = self._cmd_prefix + " " + cmd
        return run(cmd, timeout, background)
    
    def Delete(self):
        #Delete name space
        self.UnBindInterfaces()
        cmd = "ip netns del %s" % self._name
        run(cmd)
    
    def GetInftName(self):
        return self._vlan_intf or self._intf    

    def PrintAppInformation(self):
        print("\t Namespace           : ", self._name)
        print("\t Interfaces")
        for intf_name in self._interfaces:
            self._interfaces[intf_name].PrintInterfaceInformation()

    def AddArpEntry(self, ipaddress, mac_addr):
        cmd = self._cmd_prefix + " arp -s %s  %s" % (ipaddress, mac_addr)
        run(cmd)

    def MoveInterface(self, interface, ns):
        cmd = self._cmd_prefix  + " ip link set " + interface + " netns " + str(ns)
        return run(cmd)
        
class Container(NS):
    
    def __init__(self, name, config_file, container_id=None):
        data = json.load(open(config_file))
        if not container_id:
            self._container_name = name
            clean_up_container(name)
            self._image = data["App"]["registry"] + data["App"]["image"]
            self._container_obj = DockerEnvClient.containers.run(self._image,
                                               name = name,
                                               detach=True,
                                               auto_remove=True,
                                               stdin_open=True,
                                               tty=True,
                                               network_disabled=True)
        else:
            #Container is already running
            self._container_obj = DockerEnvClient.containers.get(container_id)
            self._container_name = container_id
        self._pid = str(DockerApiClient.inspect_container(self._container_obj.id)["State"]["Pid"])
        cmd = "ln -s " + "/proc/" + self._pid + "/ns/net /var/run/netns/" + self._pid
        run(cmd)
        super().__init__(self._pid, skip_init=True)
        
    def BringUp(self):
        pass
    
    def RunCommand(self, cmd, timeout=None, background=False, tty=True):
        print ("Running command ", cmd)
        cmd_to_run = ["sh", "-c", cmd]
        cmd_out = self._container_obj.exec_run(cmd_to_run, detach=background, stdout=True, tty=tty)
        if not background:
            sys.stdout.buffer.write(cmd_out.output)
        return cmd_out.exit_code

    def RemoveImage(self):
        try:
            print ("Removing image ", self._image)
            retcode = subprocess.call(["docker", "rmi", self._image])
        except:
            pass
 
    def Stop(self):
        self.DetachInterfaces()
        print ("Stopping container %s please wait....." % self._container_name)
        self._container_obj.stop(timeout=30)
        if self._pid:
            run("rm -f /var/run/netns/" + self._pid)

    def PrintAppInformation(self):
        print("\t Container Name      : ", self._container_name)
        super().PrintAppInformation()

AppTypeContainer = Container
AppTypeNameSpace = NS

class AppFactory():
    
    @staticmethod
    def Get(name, config_file = consts.E2E_APP_CONFIG_FILE, id=None, app_type=AppTypeContainer):
        return app_type(name, config_file, id)
             
    
if __name__ == "__main__":
    c = AppFactory.Get("sudhi-test", "e2etests/e2e.json")
    c.AttachInterface("EP0007")
    c.AddVlan("EP0007", 3001)
    c.SetMacAddress("EP0007", "8:00:00:00:00:02")
    c.SetMacAddress("EP0007", "8:00:00:00:00:03", 3001)
    c.SetIpAddress("EP0007", "2.2.2.2", 24, 3001)
    print ( c.RunCommand("ls"))
    time.sleep(1)
    c.DeleteVlan("EP0007", 3001)
    c.Stop()
    
