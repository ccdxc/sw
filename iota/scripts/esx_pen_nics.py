import subprocess
import argparse
import sys


parser = argparse.ArgumentParser(description='ESX pen nic finder')
parser.add_argument('--intf-type', dest='intf_type', default='data-nic', choices=['int-mnic','data-nic'])
parser.add_argument('--op', dest='op', default='vmnics', choices=['vmnics','mnic-ip'])
parser.add_argument('--mac-hint', dest='mac_hint')
args = parser.parse_args()


_MAC_DIFF = 21

def __get_nics_output():
    out = subprocess.Popen(['esxcfg-nics', '-l', '|', "grep", "Pensando Systems"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output, _ = out.communicate()
    return output

interface_types = {
    "int-mnic" : "Pensando Ethernet Management",
    "data-nic" : "Pensando Ethernet PF"
}


def MacInRange(mac, mac_hint):
        num1 = int(mac_hint.replace(':', ''), 16)
        num2 = int(mac.replace(':', ''), 16)
        return abs(num2 - num1) <= _MAC_DIFF

def __print_mnic_ip(mac_hint, intf_type):
    output = __get_nics_output()
    exit_code = 1
    for line in output.splitlines():
        if line not in ["", "\n"] and interface_types[intf_type] in str(line):
                strline = str(line)
                pci = strline.split()[1]
                mac = strline.split()[6]
                if MacInRange(mac, mac_hint):
                    addr = str(pci)
                    addr = int((addr.split(":")[1]), 16)
                    print("169.254.{}.2".format(int(addr)))
                    exit_code = 0
    return exit_code


def __print_vmnics(mac_hint, intf_type):
    output = __get_nics_output()
    exit_code = 1
    for line in output.splitlines():
        if line not in ["", "\n"] and interface_types[intf_type] in str(line):
                strline = str(line, 'UTF8')
                strline.strip()
                intf = strline.split()[0]
                mac = strline.split()[6]
                if MacInRange(mac, mac_hint):
                    print(str(intf))
                    exit_code = 0
    return exit_code

operation = {
    "vmnics" : __print_vmnics,
    "mnic-ip" : __print_mnic_ip
}

if __name__ == '__main__':                                        
    exit_code = operation[args.op](args.mac_hint, args.intf_type)                                        
    sys.exit(exit_code)                                                               