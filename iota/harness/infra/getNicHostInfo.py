import json
import pdb
import re
import subprocess
import sys

class SshClient(object):

    def __init__(self, username, password, ip):
        self.username = username
        self.password = password
        self.ip = ip
        self.scpPfx = "sshpass -p %s scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password
        self.sshPfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password
        self.sshHost = "%s@%s" % (self.username, self.ip)

    def sshCmd(self,cmd):
        fullCmd = "%s %s %s" % (self.sshPfx, self.sshHost, cmd)
        params = re.split('\s', fullCmd)
        params = [p for p in params if len(p) > 0]
        output = subprocess.check_output(params, stderr=subprocess.PIPE)
        return output.decode("utf-8")


def getMatch(nic, eth_dict):
    hnic = {}
    hnic['Ports'] = []
    for port in nic['Ports']:
        for eth in eth_dict:
            num1 = int(eth_dict[eth]['mac'].replace(':', ''), 16)
            num2 = int(port['MAC'].replace(':', ''), 16)
            delta= num2 - num1
            #print("%s mac=%x, wmac=%x, delta=%d" % (eth, num1, num2, delta))
            if delta == 21: # This simple logic need to be confirmed or improved.
                hport = dict(Name=eth, MAC=eth_dict[eth]['mac'])
                hport['IP'] = eth_dict[eth].get('ipv4', '')
                hnic['Ports'].append(hport)
    return hnic

def getNicHostInfo(warmdFile):
    warm = json.loads(open(warmdFile).read())
    username = warm['Provision']['Username']
    password = warm['Provision']['Password']

    for hh in warm['Instances']:
        ip = hh['NodeMgmtIP']
        sshClient = SshClient(username,password,ip)

        output = sshClient.sshCmd("sudo grep -r '1dd8' /sys/class/net/*/device/vendor")
        eth_dict = {}
        for line in output.splitlines():
            #print(line)
            eth=line.split('/')[4]
            #print(eth)
            cmd1 = "sudo ifconfig %s" % eth
            cmd2 = "sudo cat /sys/class/net/%s/device/device" % eth
            eth_dict[eth]=dict(intf_detail=sshClient.sshCmd(cmd1), dev_id=sshClient.sshCmd(cmd2).rstrip())
            for line1 in eth_dict[eth]['intf_detail'].splitlines():
                if (line1.find('ether') != -1):
                    eth_dict[eth]['mac'] = line1.split()[1]
                if (line1.find('netmask') != -1):
                    eth_dict[eth]['ipv4'] = line1.split()[1]
                    eth_dict[eth]['netmask'] = line1.split()[3]
                if (line1.find('inet6') != -1):
                    eth_dict[eth]['ipv6'] = line1.split()[1]
            del(eth_dict[eth]['intf_detail'])

        #print(eth_dict)

        cool = dict(ID=warm['ID'], Instances=[])
        nh=dict(ID=hh['ID'], NodeMgmtIP=hh['NodeMgmtIP'], NodeCimcIP=hh['NodeCimcIP'])
        nh['NodeCimcUsername'] = hh['NodeCimcUsername']
        nh['NodeCimcPassword'] = hh['NodeCimcPassword']
        nh['Type'] = hh['Type']
        nh['Resource'] = hh['Resource'].copy()
        nh['Nics'] = []
        for nic in hh['Nics']:
            nh['Nics'].append(getMatch(nic, eth_dict))
        cool['Instances'].append(nh)
        #print(cool)
        json.dump(cool, fp=open('coold.json', 'w'), indent=2)
        return cool['Instances']


if __name__ == '__main__':
    print(getNicHostInfo(sys.argv[1]))
    
