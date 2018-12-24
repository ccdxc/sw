#! /usr/bin/python3
import argparse
import pexpect
import sys
import os
import time
import socket
import pdb
import requests
import subprocess
import json
import atexit

HOST_NAPLES_DIR                 = "/naples"
HOST_ESX_NAPLES_IMAGES_DIR      = "/home/vm"

parser = argparse.ArgumentParser(description='Naples Boot Script')
# Mandatory parameters
parser.add_argument('--console-ip', dest='console_ip', required = True,
                    default=None, help='Console Server IP Address.')
parser.add_argument('--console-port', dest='console_port', required = True,
                    default=None, help='Console Server Port.')
parser.add_argument('--host-ip', dest='host_ip', required = True,
                    default=None, help='Host IP Address.')
parser.add_argument('--cimc-ip', dest='cimc_ip', required = True,
                    default=None, help='CIMC IP Address.')
parser.add_argument('--os', dest='os', required = True,
                    default="", help='Node OS (Freebsd or Linux).')
# Optional parameters
parser.add_argument('--console-username', dest='console_username',
                    default="admin", help='Console Server Username.')
parser.add_argument('--console-password', dest='console_password',
                    default="N0isystem$", help='Console Server Password.')
parser.add_argument('--username', dest='username',
                    default="root", help='Naples Username.')
parser.add_argument('--password', dest='password',
                    default="pen123", help='Naples Password.')
parser.add_argument('--timeout', dest='timeout',
                    default=60, help='Naples Password.')
parser.add_argument('--image', dest='image',
                    default=None, help='Naples Image.')
parser.add_argument('--drivers-pkg', dest='drivers_pkg',
                    default=None, help='Driver Package.')
parser.add_argument('--host-username', dest='host_username',
                    default="root", help='Host Username')
parser.add_argument('--host-password', dest='host_password',
                    default="docker", help='Host Password.')
parser.add_argument('--mode', dest='mode', default='hostpin',
                    choices=["classic", "hostpin"],
                    help='Naples mode - hostpin / classic.')
parser.add_argument('--cimc-username', dest='cimc_username',
                    default="admin", help='CIMC Username')
parser.add_argument('--cimc-password', dest='cimc_password',
                    default="N0isystem$", help='CIMC Password.')
parser.add_argument('--debug', dest='debug',
                    action='store_true', help='Enable Debug Mode')
parser.add_argument('--uuid', dest='uuid',
                    default="", help='Node UUID (Base MAC Address).')
parser.add_argument('--only-mode-change', dest='only_mode_change',
                    action='store_true', help='Only change mode and reboot.')
parser.add_argument('--only-init', dest='only_init',
                    action='store_true', help='Only Initialize the nodes and start tests')
parser.add_argument('--mnic-ip', dest='mnic_ip',
                    default="169.254.0.1", help='Mnic IP.')
parser.add_argument('--skip-driver-install', dest='skip_driver_install',
                    action='store_true', help='Skips host driver install')
parser.add_argument('--esx-script', dest='esx_script',
                    default="", help='ESX start up script')

GlobalOptions = parser.parse_args()
GlobalOptions.console_port = int(GlobalOptions.console_port)
GlobalOptions.timeout = int(GlobalOptions.timeout)
ws_top = os.path.dirname(sys.argv[0]) + '/../../'
ws_top = os.path.abspath(ws_top)

if GlobalOptions.image is None:
    GlobalOptions.image = "%s/nic/naples_fw.tar" % ws_top
if GlobalOptions.drivers_pkg is None:
    GlobalOptions.drivers_pkg = "%s/platform/gen/drivers-%s.tar.xz" % (ws_top, GlobalOptions.os)

ROOT_EXP_PROMPT="~#"
if GlobalOptions.os == 'freebsd':
    ROOT_EXP_PROMPT="~]#"

if GlobalOptions.os == 'esx':
    ROOT_EXP_PROMPT="~]"

def IpmiReset():
    os.system("ipmitool -I lanplus -H %s -U %s -P %s power cycle" %\
              (GlobalOptions.cimc_ip, GlobalOptions.cimc_username, GlobalOptions.cimc_password))
    return

class FlushFile(object):
    def __init__(self, f):
        self.f = f
        self.buffer = sys.stdout.buffer

    def write(self, x):
        self.f.write(x)
        self.f.flush()

    def flush(self):
        self.f.flush()
sys.stdout = FlushFile(sys.stdout)

class EntityManagement:
    def __init__(self, ipaddr = None, username = None, password = None):
        if ipaddr:
            self.ipaddr = ipaddr
            self.ssh_host = "%s@%s" % (username, ipaddr)
            self.scp_pfx = "sshpass -p %s scp -o StrictHostKeyChecking=no " % password
            self.ssh_pfx = "sshpass -p %s ssh -o StrictHostKeyChecking=no " % password
        return
    
    def SendlineExpect(self, line, expect, hdl = None,
                       timeout = GlobalOptions.timeout):
        if hdl is None: hdl = self.hdl
        hdl.sendline(line)
        return hdl.expect_exact(expect, timeout)

    def Spawn(self, command):
        hdl = pexpect.spawn(command)
        hdl.timeout = GlobalOptions.timeout
        hdl.logfile = sys.stdout.buffer
        return hdl

    def WaitForSsh(self, port = 22):
        print("Waiting for IP:%s to be up." % self.ipaddr)
        for retry in range(150):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            ret = sock.connect_ex(('%s' % self.ipaddr, port))
            sock.settimeout(1)
            if ret == 0:
                return
            else:
                time.sleep(5)
        print("Host not up. Ret:%d" % ret)
        sys.exit(1)
        return

    def RunSshCmd(self, command, ignore_failure = False):
        full_command = "%s %s \"%s\"" % (self.ssh_pfx, self.ssh_host, command)
        print(full_command)
        retcode = os.system(full_command)
        if ignore_failure is False and retcode != 0:
            print("ERROR: Failed to run command: %s" % command)
            sys.exit(1)
        return retcode

    def RunNaplesCmd(self, command, ignore_failure = False):
        full_command = "sshpass -p %s ssh root@%s %s" %\
                       (GlobalOptions.password, GlobalOptions.mnic_ip, command)
        return self.RunSshCmd(full_command, ignore_failure)

class NaplesManagement(EntityManagement):
    def __init__(self):
        super().__init__()
        self.hdl = None
        return

    def __clearline(self):
        print("Clearing Console Server Line")
        hdl = self.Spawn("telnet %s" % GlobalOptions.console_ip)
        idx = hdl.expect(["Username:", "Password:"])
        if idx == 0:
            self.SendlineExpect(GlobalOptions.console_username, "Password:", hdl = hdl)
        self.SendlineExpect(GlobalOptions.console_password, "#", hdl = hdl)
        
        for i in range(6):
            time.sleep(5)
            self.SendlineExpect("clear line %d" % (GlobalOptions.console_port - 2000), "[confirm]", hdl = hdl)
            self.SendlineExpect("", " [OK]", hdl = hdl)
        hdl.close()
        return

    def Connect(self):
        self.__clearline()
        self.hdl = self.Spawn("telnet %s %s" % ((GlobalOptions.console_ip, GlobalOptions.console_port)))
        midx = self.hdl.expect_exact([ "Escape character is '^]'.", pexpect.EOF])
        if midx == 1:
            print("Failed to connect to Console %s %d" % (GlobalOptions.console_ip, GlobalOptions.console_port))
            sys.exit(1)

        midx = self.SendlineExpect("", ["#", "capri login:", "capri-gold login:"],
                                   hdl = self.hdl, timeout = 120)
        if midx == 0: return 
        # Got capri login prompt, send username/password.
        self.SendlineExpect(GlobalOptions.username, "Password:")
        ret = self.SendlineExpect(GlobalOptions.password, ["#", pexpect.TIMEOUT], timeout = 3)
        if ret == 1: SendlineExpect("", "#")

    def InitForUpgrade(self, goldfw = True, mode = True, uuid = True):
        if goldfw:
            self.SendlineExpect("fwupdate -s goldfw", "#")
        
        self.SendlineExpect("mkdir -p /sysconfig/config0", "#")
        self.SendlineExpect("mount /dev/mmcblk0p6 /sysconfig/config0", "#")
        if mode:
            self.SendlineExpect("echo %s > /sysconfig/config0/app-start.conf && sync" % GlobalOptions.mode, "#")
        if uuid:
            self.SendlineExpect("echo %s > /sysconfig/config0/sysuuid" % GlobalOptions.uuid, "#")

        self.SendlineExpect("umount /sysconfig/config0", "#")
        self.SendlineExpect("rm -rf /data/log && sync", "#")
        self.SendlineExpect("rm -rf /data/core/* && sync", "#")
        return

    def InstallFirmware(self):
        self.SendlineExpect("/nic/tools/sysupdate.sh -p /tmp/naples_fw.tar", 
                            "===> Setting startup firmware", timeout = 600,)
        return

    def Close(self):
        if self.hdl:
            self.hdl.close()
        return

class HostManagement(EntityManagement):
    def __init__(self, ipaddr):
        super().__init__(ipaddr, GlobalOptions.host_username, GlobalOptions.host_password)
        return

    def Init(self, driver_pkg = None, cleanup = True):
        self.WaitForSsh()
        os.system("date")
        if cleanup:
            self.RunSshCmd("rm -rf /naples && mkdir /naples")

        if driver_pkg:
            self.CopyIN("scripts/%s/nodeinit.sh" % GlobalOptions.os, HOST_NAPLES_DIR)
            self.CopyIN(driver_pkg, HOST_NAPLES_DIR)
            self.RunSshCmd("%s/nodeinit.sh" % HOST_NAPLES_DIR)
        return

    def CopyIN(self, src_filename, host_dir, naples_dir = None):
        dest_filename = host_dir + "/" + os.path.basename(src_filename)
        cmd = "%s %s %s:%s" % (self.scp_pfx, src_filename, self.ssh_host, dest_filename)
        print(cmd)
        ret = os.system(cmd)
        assert(ret == 0)

        self.RunSshCmd("sync")
        ret = self.RunSshCmd("ls -l %s" % dest_filename)
        assert(ret == 0)

        if naples_dir:
            naples_dest_filename = naples_dir + "/" + os.path.basename(src_filename)
            ret = self.RunSshCmd("sshpass -p %s scp -o StrictHostKeyChecking=no %s root@%s:%s" %\
                           (GlobalOptions.password, dest_filename, GlobalOptions.mnic_ip, naples_dest_filename))
            assert(ret == 0)
        return 0

    def Reboot(self, dryrun = False):
        os.system("date")
        self.RunSshCmd("sync")
        self.RunSshCmd("ls -l /root/")
        self.RunSshCmd("uptime")
        if dryrun == False:
            self.RunSshCmd("shutdown -r now", ignore_failure = True)
        time.sleep(60)
        print("Rebooting Host : %s" % GlobalOptions.host_ip)
        return

    def InstallMainFirmware(self):
        assert(self.RunSshCmd("lspci | grep 1dd8") == 0)
        self.CopyIN(GlobalOptions.image, host_dir = HOST_NAPLES_DIR, naples_dir = "/tmp")
        self.RunNaplesCmd("/nic/tools/sysupdate.sh -p /tmp/naples_fw.tar")
        return

def AtExitCleanup():
    global naples
    naples.Close()

# This function is used for 3 cases.
# 1) Full firmware upgrade
# 2) Change mode from Classic <--> Hostpin
# 3) Only initialize the node and start tests.

def Main():
    global naples
    naples = NaplesManagement()

    global host
    host = HostManagement(GlobalOptions.host_ip)
   
    if GlobalOptions.only_init == True:
        # Case 3: Only INIT option. Install drivers and return.
        host.Init(driver_pkg = GlobalOptions.drivers_pkg)
        return

    # Connect to Naples console.
    naples.Connect()

    # Check if we need to switch to GoldFw or not.
    if GlobalOptions.only_mode_change:
        # Case 2: Only change mode, reboot and install drivers
        naples.InitForUpgrade(goldfw = False)
        host.Reboot()
        naples.Close()
    else:
        # Case 1: Main firmware upgrade.
        naples.InitForUpgrade(goldfw = True)
        host.Reboot()
        naples.Close()
        host.Init(driver_pkg = "/vol/builds/goldfw/latest/drivers-%s.tar.xz" % GlobalOptions.os)
        host.InstallMainFirmware()
        # Reboot host again, this will reboot naples also
        host.Reboot()

    # Common to Case 2 and Case 1.
    # Initialize the Node, this is needed in all cases.
    host.Init(driver_pkg = GlobalOptions.drivers_pkg)
    return

if __name__ == '__main__':
    atexit.register(AtExitCleanup)
    Main()
