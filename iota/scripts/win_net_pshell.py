import pdb
import pprint
import re
import string
import subprocess

class SshClient(object):

    def __init__(self, ip, username, password):
        self.ip = ip
        self.username = username
        self.password = password
        self.scpPfx = "sshpass -p %s scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password
        self.sshPfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no "%password
        self.sshHost = "%s@%s" % (username, ip)

    def sshCmd(self,cmd,verbose=False):
        fullCmd = "%s %s %s" % (self.sshPfx, self.sshHost, cmd)
        if verbose:
            print("running ssh command: {0}".format(fullCmd))
        output = subprocess.check_output(fullCmd, shell=True, stderr=subprocess.PIPE)
        output = re.split('\n',output.decode("utf-8"))
        if verbose:
            print("output was: {0}".format(output))
        return output

    def scpGetFile(self,srcFile,dstFile):
        fullCmd = "%s %s:%s %s" % (self.scpPfx, self.sshHost, srcFile, dstFile)
        output = subprocess.check_output(re.split('\s', fullCmd), stderr=subprocess.PIPE)
        return re.split('\n',output.decode("utf-8"))

def getInterface(intId, remoteIp=None, username=None, password=None):
    """
        if remoteIp/username/password provided, attempt to run this command via ssh
        on remote host specified by remoteIp. 
        else if remoteIp is None, this command is run locally.

        return value is dictionary of interface fields
    """
    params = {}
    cmd = "\"/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe 'Get-WmiObject win32_networkadapterconfiguration -Filter \\\"InterfaceIndex = " + str(intId) + "\\\" | Select-Object -Property InterfaceIndex , MacAddress, serviceName, dhcpEnabled, ipaddress | format-table -autosize | out-string -width 512'\""
    if remoteIp:
        ssh = SshClient(remoteIp, username, password)
        output = ssh.sshCmd(cmd)
    else:
        output = subprocess.check_output(cmd, shell=True, stderr=subprocess.PIPE)
        output = output.split('\r\n') 
    for line in output:
        found = re.search("^[\s]*([\d]+)[\s]+([\S]+)[\s]+([\S]+)[\s]+([\S]+)[\s]+\{(.*)\}.*$",line)
        if found:
            ips = found.group(5).split(',')
            params = {"mac":found.group(2), "service":found.group(3), "dhcp":bool(found.group(4)), "ips":ips}
            name = getInterfaceName(intId, remoteIp, username, password)
            params['name'] = name
            break
    return params

def getInterfaces(remoteIp=None, username=None, password=None):
    """
        if remoteIp/username/password provided, attempt to run this command via ssh
        on remote host specified by remoteIp. 
        else if remoteIp is None, this command is run locally.

        return value is dictionary of interfaces. keys are interface index (string)
        and value is dictionary containing mac, service, dhcp state, ip list. 
    """
    ints = {}
    cmd = "\"/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe 'Get-WmiObject win32_networkadapterconfiguration | Select-Object -Property InterfaceIndex , MacAddress, serviceName, dhcpEnabled, ipaddress | format-table -autosize | out-string -width 512'\""
    if remoteIp:
        ssh = SshClient(remoteIp, username, password)
        output = ssh.sshCmd(cmd)
    else:
        output = subprocess.check_output(cmd, shell=True, stderr=subprocess.PIPE)
        output = output.split('\r\n') 
    for line in output:
        found = re.search("^[\s]*([\d]+)[\s]+([\S]+)[\s]+([\S]+)[\s]+([\S]+)[\s]+\{(.*)\}.*$",line)
        if found:
            ips = found.group(5).split(',')
            ints[found.group(1)] = {"mac":found.group(2), "service":found.group(3), "dhcp":bool(found.group(4)), "ips":ips}
    for intId in ints.keys():
        name = getInterfaceName(intId, remoteIp, username, password)
        ints[intId]['name'] = name
    return ints

def getInterfaceName(intId, remoteIp=None, username=None, password=None):
    """
        if remoteIp/username/password provided, attempt to run this command via ssh
        on remote host specified by remoteIp. 
        else if remoteIp is None, this command is run locally.

        returns interface name based on interface index. 
    """
    cmd = "\"/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe 'Get-NetAdapter -InterfaceIndex " + str(intId) + " | Select-Object -Property Name | format-list'\""
    if remoteIp:
        ssh = SshClient(remoteIp, username, password)
        output = ssh.sshCmd(cmd)
    else:
        output = subprocess.check_output(cmd, shell=True, stderr=subprocess.PIPE)
        output = output.split('\r\n')
    found = re.search("Name : (.*?)\r", string.join(output), re.M)
    if found:
        return found.group(1)
    return ""

def setInterfaceName(intId, name, remoteIp=None, username=None, password=None):
    """
        if remoteIp/username/password provided, attempt to run this command via ssh
        on remote host specified by remoteIp. 
        else if remoteIp is None, this command is run locally.

        sets interface name based on interface index. 
        return value is None
    """
    cmd = "\"/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe 'Get-NetAdapter -InterfaceIndex " + str(intId) + " | Rename-NetAdapter -NewName \\\"" + str(name) + "\\\"'\""
    if remoteIp:
        ssh = SshClient(remoteIp, username, password)
        output = ssh.sshCmd(cmd)
    else:
        output = subprocess.check_output(cmd, shell=True, stderr=subprocess.PIPE)
        output = output.split('\r\n')
    return output

def setInterfaceIp(intId, ip, maskbits, remoteIp=None, username=None, password=None):
    """
        if remoteIp/username/password provided, attempt to run this command via ssh
        on remote host specified by remoteIp. 
        else if remoteIp is None, this command is run locally.

        sets ip address/mask based on interface index. 
        return value is None
    """
    if not isinstance(maskbits,int):
        raise ValueError("maskbits must be integer. user passed in {0}({1})".format(maskbits,type(maskbits)))
    cmd = "\"/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe 'New-NetIPAddress -InterfaceIndex " + str(intId) + " -IPAddress " + ip + " -PrefixLength " + str(maskbits) + "'\""
    if remoteIp:
        ssh = SshClient(remoteIp, username, password)
        output = ssh.sshCmd(cmd)
    else:
        output = subprocess.check_output(cmd, shell=True, stderr=subprocess.PIPE)
        output = output.split('\r\n')

def removeInterfaceIp(intId, ip, maskbits, remoteIp=None, username=None, password=None):
    """
        if remoteIp/username/password provided, attempt to run this command via ssh
        on remote host specified by remoteIp. 
        else if remoteIp is None, this command is run locally.

        removes ip address/mask based on interface index. 
        return value is None
    """
    if not isinstance(maskbits,int):
        raise ValueError("maskbits must be integer. user passed in {0}({1})".format(maskbits,type(maskbits)))
    cmd = "\"/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe 'Remove-NetIPAddress -InterfaceIndex " + str(intId) + " -IPAddress " + ip + " -PrefixLength " + str(maskbits) + "'\""
    if remoteIp:
        ssh = SshClient(remoteIp, username, password)
        output = ssh.sshCmd(cmd)
    else:
        output = subprocess.check_output(cmd, shell=True, stderr=subprocess.PIPE)
        output = output.split('\r\n')

def setDhcpState(intId, state, remoteIp=None, username=None, password=None):
    """
        if remoteIp/username/password provided, attempt to run this command via ssh
        on remote host specified by remoteIp. 
        else if remoteIp is None, this command is run locally.

        sets dhcp state based on interface index. 
        return value is None
    """
    if state == True:
        state = "enabled"
    elif state == False:
        state = "disabled"
    else:
        raise ValueError("state must be True or False. user passed in {0}({1})".format(state,type(state)))
    cmd = "\"/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe 'Set-NetIPInterface -InterfaceIndex " + str(intId) + " -Dhcp " + state + "'\""
    if remoteIp:
        ssh = SshClient(remoteIp, username, password)
        output = ssh.sshCmd(cmd)
    else:
        output = subprocess.check_output(cmd, shell=True, stderr=subprocess.PIPE)
        output = output.split('\r\n')


