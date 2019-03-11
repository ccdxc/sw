#! /usr/bin/python3
import re

NODE_TYPE_HOST = 'host'
NODE_TYPE_PXE  = 'pxe'

def GetMacList(buf):
    reg_exp = "[0-9A-F]{2}[:-][0-9A-F]{2}[:-][0-9A-F]{2}[:-][0-9A-F]{2}[:-][0-9A-F]{2}[:-][0-9A-F]{2}"
    return re.findall(reg_exp, buf.upper())

def GetUserNameByType(devtype):
    if devtype == NODE_TYPE_PXE:
        return 'root'
    if devtype == NODE_TYPE_HOST:
        return 'root'
    raise Exception("Device type unknown")

def GetPasswordByType(devtype):
    if devtype == NODE_TYPE_PXE:
        return 'N0isystem$'
    if devtype == NODE_TYPE_HOST:
        return 'docker'
    raise Exception("Device type unknown")

def GetPromptByType(devtype, osname):
    if devtype == NODE_TYPE_PXE:
        return ']#'
    if devtype == NODE_TYPE_HOST:
        return 'docker'
    raise Exception("Device type unknown")

def GetCimcUsername():
    return "admin"

def GetCimcPassword():
    return "N0isystem$"


class PxeServer:
    PEXPECT_TIMEOUT = 120
    PXE_SVR_EXPECT_PROMPT = "]#"
    # This can be done as analytics later with server login and reading files on the server
    CentOS75                = "centos"
    ESXi65                  = "esxi"
    Ubuntu1804              = "linux_ubuntu"
    FreeBSD112              = "linux_freebsd"
    AlpineLinux38           = "linux_alpine"
    AlpineLinux38vanilla    = "linux_alpine_venilla"
    RedhatEnterpriseLinux73 = "linux_redhat_73"
    OracleLinux75           = "linux_oracle"
    Debian901Live           = "debian"
    RedhatEnterpriseLinux75 = "linux_redhat_75"
    OracleLinux76           = "linux_oracle_76"

    PXE_SVR = "pxe"
    PXE_SVR_USERNAME = "root"
    PXE_SVR_PASSWORD = "N0isystem$"
    PXE_OS_FILE_MAP = {
        CentOS75 : 'pxe-centos-7.5',
        ESXi65   : 'pxe-esxi-6.5' ,
        Ubuntu1804 : 'pxe-ubuntu-18.04', 
        FreeBSD112 : 'pxe-freebsd11.2',
        RedhatEnterpriseLinux73 : 'pxe-linux-rh-7.3',
        AlpineLinux38 : 'pxe-linux-alpine-3.8', 
        AlpineLinux38vanilla : 'pxe-linux-alpine-venilla', 
        OracleLinux75 : 'pxe-linux-oracle-7.5',
        Debian901Live : 'pxe-dabian-live' ,
        RedhatEnterpriseLinux75 : 'pxe-linux-rh-7.5' ,
        OracleLinux76 : 'pxe-linux-oracle-7.6' ,
    }
    
        #RedhatEnterpriseLinux73 : 'pxe-linux-rh-7.3',
    @staticmethod
    def GetServerIp():
        return PxeServer.PXE_SVR
    
    @staticmethod
    def GetOsPath(osname):
        if osname not in PxeServer.PXE_OS_FILE_MAP.keys():
            raise Exception("[%s] Not a valid OS for PXE server" % osname)
        return PxeServer.PXE_OS_FILE_MAP[osname]

def GetPxeServerIp():
    return PxeServer.GetServerIp()

def GetOsPath(osname):
    return PxeServer.GetOsPath(osname)

def CheckPxeSupport(osname):
    if osname in PxeServer.PXE_OS_FILE_MAP.keys():
        return True
    return False
