#!/usr/bin/python3
import subprocess
import argparse
import sys
import json


parser = argparse.ArgumentParser(description='ESX pen nic finder')
parser.add_argument('--intf-type', dest='intf_type', default='data-nic', choices=['int-mnic','data-nic'])
parser.add_argument('--op', dest='op', default='intfs', choices=['intfs','mnic-ip'])
parser.add_argument('--os', dest='os', default='linux', choices=['linux','esx','freebsd'])
parser.add_argument('--mac-hint', dest='mac_hint')
args = parser.parse_args()


_MAC_DIFF = 24

def __get_nics_output_esx():
    out = subprocess.Popen(['esxcfg-nics', '-l', '|', "grep", "Pensando Systems"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output, _ = out.communicate()
    return output

esx_interface_types = {
    "int-mnic" : "Pensando Ethernet Management",
    "data-nic" : "Pensando Ethernet PF"
}


def MacInRange(mac, mac_hint):
        num1 = int(mac_hint.replace(':', ''), 16)
        num2 = int(mac.replace(':', ''), 16)
        return abs(num2 - num1) <= _MAC_DIFF

def __print_mnic_ip_esx(mac_hint, intf_type):
    output = __get_nics_output_esx()
    exit_code = 1
    for line in output.splitlines():
        if line not in ["", "\n"] and esx_interface_types[intf_type] in str(line):
                strline = str(line)
                pci = strline.split()[1]
                mac = strline.split()[6]
                if MacInRange(mac, mac_hint):
                    addr = str(pci)
                    addr = int((addr.split(":")[1]), 16)
                    print("169.254.{}.1".format(int(addr)))
                    exit_code = 0
    return exit_code


def __print_intfs_esx(mac_hint, intf_type):
    output = __get_nics_output_esx()
    exit_code = 1
    for line in output.splitlines():
        if line not in ["", "\n"] and esx_interface_types[intf_type] in str(line):
                strline = str(line, 'UTF8')
                strline.strip()
                intf = strline.split()[0]
                mac = strline.split()[6]
                if MacInRange(mac, mac_hint):
                    print(str(intf))
                    exit_code = 0
    return exit_code


def __get_nics_output_linux():
    out = subprocess.Popen(['lshw', '-json'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output, _ = out.communicate()
    return output



def __get_devices_linux(mac_hint):
    children = []
    def populateChildren(entry, children):
        children.append(entry)
        for child in entry.get("children", []):
           populateChildren(child, children)


    entries=json.loads(__get_nics_output_linux())
    populateChildren(entries, children)
    devs=[]
    for child in children:
        if child.get("id") == "network" and ("serial" in child) and ("businfo" in child) and ("logicalname" in child):
            if MacInRange(child["serial"], mac_hint):
                addr=str(child["businfo"].split(":")[1])
                devs.append((child["logicalname"], int(addr, 16)))

    devs.sort(key = lambda x: x[1])
    return devs


def __print_intfs_linux(mac_hint, intf_type):
    devs = __get_devices_linux(mac_hint)
    if intf_type == "data-nic":
        if len(devs) < 2:
            print("Not able to find Naples Data ports", file=sys.stderr)
            return 1
        devs.pop()
        for dev in devs:
            print(dev[0])
    else:
        if len(devs) == 0:
            print("Not able to find Naples OOB port", file=sys.stderr)
            return 1
        print(devs[-1][0])


def __print_mnic_ip_linux(mac_hint, intf_type):
    devs=__get_devices_linux(mac_hint)
    print("169.254.{}.1".format(devs[-1][1]))


def __get_devices_freebsd(mac_hint):
    cmd0='pciconf  -l | grep ion'
    output=subprocess.check_output(cmd0, shell=True)
    devs=[]
    for out in output.splitlines():
        intf=str(out, "UTF-8") 
        pci=str(out, "UTF-8") 
        intf=intf.split("@")[0] 
        pciID=pci.split(":")[1] 
        intf=intf.replace("ion", "ionic")
        macAddrCmd = "ifconfig " + intf + " | grep -e ether -e hwaddr | cut -d ' ' -f 2"
        output=subprocess.check_output(macAddrCmd, shell=True)
        macAddr = str(output, "UTF-8").split()[0]
        if MacInRange(macAddr, mac_hint):
            devs.append((intf, pciID))
    devs.sort(key = lambda x: x[1])
    return devs

def __print_intfs_freebsd(mac_hint, intf_type):
    devs = __get_devices_freebsd(mac_hint)

    if intf_type == "data-nic":
        if len(devs) < 2:
            print("Not able to find Naples Data ports", file=sys.stderr)
            return 1
        devs.pop()
        for dev in devs:
            print(dev[0])
    else:
        if len(devs) == 0:
            print("Not able to find Naples OOB port", file=sys.stderr)
            return 1
        print(devs[-1][0])


def __print_mnic_ip_freebsd(mac_hint, intf_type):
    devs=__get_devices_freebsd(mac_hint)
    print("169.254.{}.1".format(devs[-1][1]))

os_operation = {
"esx" : {
    	"intfs" :  __print_intfs_esx,
    	"mnic-ip" : __print_mnic_ip_esx
    },
"linux" : {
    	"intfs" :  __print_intfs_linux,
    	"mnic-ip" : __print_mnic_ip_linux
    },
"freebsd" : {
    	"intfs" :  __print_intfs_freebsd,
    	"mnic-ip" : __print_mnic_ip_freebsd
    }
}

if __name__ == '__main__':                                        
    exit_code = os_operation[args.os][args.op](args.mac_hint, args.intf_type)                                        
    sys.exit(exit_code)                                                               
