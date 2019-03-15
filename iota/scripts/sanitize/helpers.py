#! /usr/bin/python3
import re
import json
import time
import os

TIMEOUT = 60
NODE_TYPE_HOST = 'host'
NODE_TYPE_PXE  = 'pxe'
PXE_SVR_FILENAME = './pxe_server.json'
from helpers import *

def JsonToDict(filename=PXE_SVR_FILENAME):
    with open(filename) as fp:
        return(json.load(fp))

def SendlineExpect(line, expect, hdl, timeout = TIMEOUT):
    hdl.sendline(line)
    return hdl.expect_exact(expect, timeout)

class PxeServerHelper:
    server_info = None
    PXE_OP_CREATE = 'create'
    PXE_OP_DELETE = 'delete'

    @staticmethod
    def SetupPxeServer(server, operation, macs, osname):
        for mac in macs:
            mac = '01-' + re.sub(':', '-', mac)
            if operation == PxeServerHelper.PXE_OP_CREATE:
                ssh_cmd = 'cd /export/tftpboot/pxelinux.cfg && ln -sf %s %s' % (PxeServerHelper.GetOsPath(osname), mac.lower())
            if operation == PxeServerHelper.PXE_OP_DELETE:
                ssh_cmd = 'cd /export/tftpboot/pxelinux.cfg && rm -f %s' % (mac.lower())
            server.RunSshCmd(ssh_cmd)
    
    @staticmethod
    def InitServerInfo():
        if PxeServerHelper.server_info == None:
            PxeServerHelper.server_info = JsonToDict()
        return True 

    @staticmethod
    def GetServerIp():
        return PxeServerHelper.server_info['ipaddr']
    
    @staticmethod
    def GetOsPath(osname):
        if osname not in PxeServerHelper.server_info['images'].keys():
            raise Exception("[%s] Not a valid OS for PXE server" % osname)
        return PxeServerHelper.server_info['images'][osname]

class ServerHelper():
    @staticmethod
    def GetUserNameByType(devtype):
        if devtype == NODE_TYPE_PXE:
            return 'root'
        if devtype == NODE_TYPE_HOST:
            return 'root'
        raise Exception("Device type unknown")

    @staticmethod
    def GetPasswordByType(devtype):
        if devtype == NODE_TYPE_PXE:
            return 'N0isystem$'
        if devtype == NODE_TYPE_HOST:
            return 'docker'
        raise Exception("Device type unknown")

    @staticmethod
    def GetPromptByType(devtype, osname):
        if devtype == NODE_TYPE_PXE:
            return ']#'
        if devtype == NODE_TYPE_HOST:
            return '#'
        raise Exception("Device type unknown")

class HostHelper():
    
    @staticmethod
    def RunIpmi(host, command):
        cmd = "ipmitool -I lanplus -H %s -U %s -P %s %s" % (host.cimc, host.cimc_username, host.cimc_password, command)
        print ("%s: %s" % (str(host),command))
        return  os.system(cmd)

    @staticmethod
    def SetUserPassword(host, username, password):
        hdl = host.GetHandle()
        SendlineExpect("sudo passwd %s" % username, "password:",  hdl)
        SendlineExpect("%s" % password, "password:" , hdl)
        SendlineExpect("%s" % password, ServerHelper.GetPromptByType(NODE_TYPE_HOST),  hdl)
        return True

    @staticmethod
    def ShowVersion(host, cmd=None):
        if cmd == None:
            cmd = "cat /etc/os-release"
        hdl = host.GetHandle()
        SendlineExpect(cmd, ServerHelper.GetPromptByType(NODE_TYPE_HOST), hdl)
        op = hdl.before
        print ("%s: %s" % (str(host), op))
        return True
    
    @staticmethod
    def WaitForReachability(host, msg):
        i = 0 
        while i < 150:
            print ("%s: %s" % (str(host),msg))
            if host.IsPingable():
                break
            time.sleep(5)
            i += 1
        if i == 100:
            raise Exception("Node not booting after [%s] seconds -- Debug" % str(100*5))
        return True
        
    @staticmethod
    def WaitForReboot(host, msg):
        i = 0 
        while i < 150:
            print ("%s: %s" % (str(host), msg))
            if not host.IsPingable():
                break
            time.sleep(5)
            i += 1
        if i == 100:
            raise Exception("Node not booting after [%s] seconds -- Debug" % str(100*5))
        return True
        
    @staticmethod
    def SetHostName(host):
        cmd = "hostname %s" % host.ipaddr
        host.RunSshCmd(cmd)

class CimcHelper():
    @staticmethod
    def GetCimcPrompt():
        # Best to get from json
        return "#"
    
    @staticmethod
    def GetCimcUsername():
        # Best to get from json
        return "admin"

    @staticmethod
    def GetCimcPassword():
        # Best to get from json
        return "N0isystem$"

    @staticmethod
    def GetMacList(buf):
        reg_exp = "[0-9A-F]{2}[:-][0-9A-F]{2}[:-][0-9A-F]{2}[:-][0-9A-F]{2}[:-][0-9A-F]{2}[:-][0-9A-F]{2}"
        return re.findall(reg_exp, buf.upper())
    
    @staticmethod
    def GetOobMacFromCimc(host):
        cimc = host.GetCimcSshHandle()
        SendlineExpect("scope chassis", CimcHelper.GetCimcPrompt(),  cimc)
        SendlineExpect("scope network-adapter L", CimcHelper.GetCimcPrompt(),  cimc)
        SendlineExpect("show mac-list", CimcHelper.GetCimcPrompt(),  cimc)
        op = cimc.before
        return CimcHelper.GetMacList(op)

    @staticmethod
    def SetBootOrder(host, boot_order):
        cimc = host.GetCimcSshHandle()
        SendlineExpect("scope bios", CimcHelper.GetCimcPrompt(),  cimc)
        SendlineExpect("set boot-order %s" % boot_order, CimcHelper.GetCimcPrompt(),  cimc)
        SendlineExpect("commit", "y|N]",  cimc)
        SendlineExpect("y", CimcHelper.GetCimcPrompt(),  cimc)
        return

