#! /usr/bin/python3
import argparse
import pexpect
import sys
import os
import re
import time
import socket
import pdb
import requests
import subprocess
import json
import atexit
import paramiko
import threading
import traceback
import ipaddress
import random
from enum import auto, Enum, unique

HOST_NAPLES_DIR                 = "/naples"
NAPLES_TMP_DIR                  = "/data"
HOST_ESX_NAPLES_IMAGES_DIR      = "/home/vm"
UPGRADE_TIMEOUT                 = 600
NAPLES_CONFIG_SPEC_LOCAL        = "/tmp/system-config.json"

parser = argparse.ArgumentParser(description='Naples Boot Script')
# Mandatory parameters
parser.add_argument('--testbed', dest='testbed', required = True,
                    default=None, help='testbed json file - warmd.json.')
parser.add_argument('--instance-name', dest='instance_name', required = True,
                    default=None, help='instance id.')
parser.add_argument('--naples', dest='naples_type', required = True,
                    default="", help='Naples type : capri/equinix')

# Optional parameters
parser.add_argument('--wsdir', dest='wsdir', default='/sw',
                    help='Workspace folder')
parser.add_argument('--mac-hint', dest='mac_hint',
                    default="", help='Mac hint')
parser.add_argument('--username', dest='naples_username',
                    default="root", help='Naples Username.')
parser.add_argument('--password', dest='naples_password',
                    default="pen123", help='Naples Password.')
parser.add_argument('--timeout', dest='timeout',
                    default=180, help='Naples Password.')
parser.add_argument('--image-manifest', dest='image_manifest',
                    default='/sw/images/latest.json', help='Image manifest file')
parser.add_argument('--mode', dest='mode', default='hostpin',
                    choices=["classic", "hostpin", "bitw", "hostpin_dvs", "unified"],
                    help='Naples mode - hostpin / classic.')
parser.add_argument('--debug', dest='debug',
                    action='store_true', help='Enable Debug Mode')
parser.add_argument('--uuid', dest='uuid',
                    default="", help='Node UUID (Base MAC Address).')
parser.add_argument('--only-mode-change', dest='only_mode_change',
                    action='store_true', help='Only change mode and reboot.')
parser.add_argument('--only-init', dest='only_init',
                    action='store_true', help='Only Initialize the nodes and start tests')
parser.add_argument('--no-mgmt', dest='no_mgmt',
                    action='store_true', help='Do not ping test mgmt interface on host')
parser.add_argument('--mnic-ip', dest='mnic_ip',
                    default="169.254.0.1", help='Mnic IP.')
parser.add_argument('--mgmt-intf', dest='mgmt_intf',
                    default="oob_mnic0", help='Management Interface (oob_mnic0 or bond0).')
parser.add_argument('--naples-mem-size', dest='mem_size',
                    default=None, help='Naples memory size')
parser.add_argument('--skip-driver-install', dest='skip_driver_install',
                    action='store_true', help='Skips host driver install')
parser.add_argument('--naples-only-setup', dest="naples_only_setup",
                    action='store_true', help='Setup only naples')
parser.add_argument('--esx-script', dest='esx_script',
                    default=None, help='ESX start up script')
parser.add_argument('--use-gold-firmware', dest='use_gold_firmware',
                    action='store_true', help='Only use gold firmware')
parser.add_argument('--fast-upgrade', dest='fast_upgrade',
                    action='store_true', help='update firmware only')
parser.add_argument('--auto-discover-on-install', dest='auto_discover',
                    action='store_true', help='On install do auto discovery')


GlobalOptions = parser.parse_args()
GlobalOptions.timeout = int(GlobalOptions.timeout)
ws_top = os.path.dirname(sys.argv[0]) + '/../../'
ws_top = os.path.abspath(ws_top)
sys.path.insert(0, ws_top)
import iota.harness.infra.utils.parser as jparser

ESX_CTRL_VM_BRINGUP_SCRIPT = "%s/iota/bin/iota_esx_setup" % (GlobalOptions.wsdir)

# Create system config file to enable console with out triggering
# authentication.
def CreateConfigConsoleNoAuth():
    console_enable = {'console': 'enable'}
    with open(NAPLES_CONFIG_SPEC_LOCAL, 'w') as outfile:
        json.dump(console_enable, outfile, indent=4)

# Error codes for all module exceptions
@unique
class _errCodes(Enum):
    INCORRECT_ERRCODE                = 1

    #Host Error Codes
    HOST_INIT_FAILED                 = 2
    HOST_COPY_FAILED                 = 3
    HOST_CMD_FAILED                  = 5
    HOST_RESTART_FAILED              = 6
    HOST_DRIVER_INSTALL_FAILED       = 10
    HOST_INIT_FOR_UPGRADE_FAILED     = 11
    HOST_INIT_FOR_REBOOT_FAILED      = 12


    #Naples Error codes
    NAPLES_TELNET_FAILED             = 100
    NAPLES_LOGIN_FAILED              = 101
    NAPLES_OOB_SSH_FAILED            = 102
    NAPLES_INT_MNIC_SSH_FAILED       = 103
    NAPLES_GOLDFW_UNKNOWN            = 104
    NAPLES_FW_INSTALL_FAILED         = 105
    NAPLES_CMD_FAILED                = 106
    NAPLES_GOLDFW_REBOOT_FAILED      = 107
    NAPLES_INIT_FOR_UPGRADE_FAILED   = 108
    NAPLES_REBOOT_FAILED               = 109
    NAPLES_FW_INSTALL_FROM_HOST_FAILED = 110
    NAPLES_TELNET_CLEARLINE_FAILED     = 111
    NAPLES_MEMORY_SIZE_INCOMPATIBLE    = 112
    FAILED_TO_READ_FIRMWARE_TYPE       = 113

    #Entity errors
    ENTITY_COPY_FAILED               = 300
    NAPLES_COPY_FAILED               = 301
    ENTITY_SSH_CMD_FAILED            = 302
    ENTITY_NOT_UP                    = 303


    #ESX Host  Error codes
    HOST_ESX_CTRL_VM_COPY_FAILED     = 200
    HOST_ESX_CTRL_VM_RUN_CMD_FAILED  = 201
    HOST_ESX_BUILD_VM_COPY_FAILED    = 202
    HOST_ESX_INIT_FAILED             = 203
    HOST_ESX_CTRL_VM_INIT_FAILED     = 204
    HOST_ESX_REBOOT_FAILED           = 205
    HOST_ESX_DRIVER_BUILD_FAILED     = 206
    HOST_ESX_CTRL_VM_CMD_FAILED      = 207
    HOST_ESX_CTRL_VM_STARTUP_FAILED  = 208
    HOST_ESX_BUILD_VM_RUN_FAILED     = 209

FIRMWARE_TYPE_MAIN = 'mainfwa'
FIRMWARE_TYPE_GOLD = 'goldfw'
FIRMWARE_TYPE_UNKNOWN = 'unknown'


class bootNaplesException(Exception):

    def __init__(self, error_code, message='', *args, **kwargs):
        # Raise a separate exception in case the error code passed isn't specified in the _errCodes enum
        if not isinstance(error_code, _errCodes):
            msg = 'Error code passed in the error_code param must be of type {0}'
            raise bootNaplesException(_errCodes.INCORRECT_ERRCODE, msg, _errCodes.__class__.__name__)

        # Storing the error code on the exception object
        self.error_code = error_code

        # storing the traceback which provides useful information about where the exception occurred
        self.traceback = sys.exc_info()

          # Prefixing the error code to the exception message
        self.message = message
        try:
            self.ex_message = '{0} : {1}'.format(error_code.name, message.format(*args, **kwargs))
        except (IndexError, KeyError):
            self.ex_message = '{0} : {1}'.format(error_code.name, message)


        super(bootNaplesException, self).__init__(self.ex_message)

    def __str__(self):
        return self.ex_message


class _exceptionWrapper(object):
    def __init__(self,exceptCode, msg):
        self.exceptCode = exceptCode
        self.msg = msg
    def __call__(self, original_func):
        def wrappee( *args, **kwargs):
            try:
                return original_func(*args,**kwargs)
            except bootNaplesException as cex:
                print(traceback.format_exc())
                if  cex.error_code != self.exceptCode:
                    raise bootNaplesException(self.exceptCode, "\n" + cex.error_code.name + ":" + str(cex.message))
                raise cex
            except Exception as ex:
                print(traceback.format_exc())
                raise bootNaplesException(self.exceptCode, str(ex))
            except:
                print(traceback.format_exc())
                raise bootNaplesException(self.exceptCode, self.msg)

        return wrappee

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
    def __init__(self, ipaddr = None, username = None, password = None, fw_images = None):
        self.ipaddr = ipaddr
        self.mac_addr = None
        self.host = None
        self.hdl = None
        self.username = username
        self.password = password
        self.fw_images = fw_images
        self.console_logfile = None
        self.SSHPassInit()
        return

    def SetHost(self, host):
        self.host = host

    def SetIpmiHandler(self, handler):
        self.ipmi_handler = handler

    def SSHPassInit(self):
        self.ssh_host = "%s@%s" % (self.username, self.ipaddr)
        self.scp_pfx = "sshpass -p %s scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % self.password
        self.ssh_pfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % self.password


    def NaplesWait(self):
        midx = self.SendlineExpect("", ["#", "capri login:", "capri-gold login:"],
                               hdl = self.hdl, timeout = 30)
        if midx == 0: return
        # Got capri login prompt, send username/password.
        self.SendlineExpect(self.username, "Password:")
        ret = self.SendlineExpect(self.password, ["#", pexpect.TIMEOUT], timeout = 3)
        if ret == 1: self.SendlineExpect("", "#")

    def IpmiResetAndWait(self):
        print('calling IpmiResetAndWait')
        os.system("date")

        self.ipmi_handler()
        self.WaitAfterReset()
        return

    def WaitAfterReset(self):
        print("sleeping 120 seconds after IpmiReset")
        time.sleep(120)
        print("finished 120 second sleep. Looking for prompt now...")
        i = 0
        while True:
            try:
                self.NaplesWait()
                break
            except:
                if i > 3:
                    raise Exception("Naples prompt not observed")
                i = i + 1
                continue
        print("Waiting for host ssh..")
        self.host.WaitForSsh()

    def __syncLine(self, hdl):
        for i in range(3):
            try:
                syncTag = "SYNC{0}".format(random.randint(0,0xFFFF))
                syncCmd = "echo " + syncTag
                syncSearch = syncTag + "\r\n#"
                print("attempting to sync buffer with \"{0}\"".format(syncCmd))
                hdl.sendline(syncCmd)
                hdl.expect_exact(syncSearch,30)
                return
            except:
                print("failed to find sync message, trying again")
        raise Exception("buffer sync failed")

    def SyncLine(self, hdl = None):
        if hdl is None:
            hdl = self.hdl
        self.__syncLine(hdl)

    def __sendlineExpect(self, line, expect, hdl, timeout):
        os.system("date")
        hdl.sendline(line)
        return hdl.expect_exact(expect, timeout)

    def SendlineExpect(self, line, expect, hdl = None,
                       timeout = GlobalOptions.timeout, trySync=False):
        if hdl is None: hdl = self.hdl
        try:
            return self.__sendlineExpect(line, expect, hdl, timeout)
        except pexpect.TIMEOUT:
            if trySync:
                self.__syncLine(hdl)
                return self.__sendlineExpect(line, expect, hdl, timeout)
            else:
                raise

    def Spawn(self, command, dev_name=None):
        hdl = pexpect.spawn(command)
        hdl.timeout = GlobalOptions.timeout
        if dev_name:
            if not self.console_logfile:
                self.console_logfile = open(dev_name + ".log", 'w', buffering=1)
            hdl.logfile = self.console_logfile.buffer
        else:
            hdl.logfile = sys.stdout.buffer
        return hdl

    @_exceptionWrapper(_errCodes.ENTITY_NOT_UP, "Host not up")
    def WaitForSsh(self, port = 22):
        print("Waiting for IP:%s to be up." % self.ipaddr)
        for retry in range(180):
            if self.IsSSHUP():
                return
            time.sleep(5)
        print("Host not up")
        raise Exception("Host : {} did not up".format(self.ipaddr))

    def IsSSHUP(self, port = 22):
        if not self.ipaddr:
            print("No IP set , SSH not up")
            return False
        print("Waiting for IP:%s to be up." % self.ipaddr)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        ret = sock.connect_ex(('%s' % self.ipaddr, port))
        sock.settimeout(10)
        if ret == 0:
            return True
        print("Host not up. Ret:%d" % ret)
        return False

    @_exceptionWrapper(_errCodes.ENTITY_SSH_CMD_FAILED, "SSH cmd failed")
    def RunSshCmd(self, command, ignore_failure = False):
        date_command = "%s %s \"date\"" % (self.ssh_pfx, self.ssh_host)
        os.system(date_command)
        full_command = "%s %s \"%s\"" % (self.ssh_pfx, self.ssh_host, command)
        print(full_command)
        retcode = os.system(full_command)
        if ignore_failure is False and retcode != 0:
            print("ERROR: Failed to run command: %s (exit = %d)" % (command,retcode))
            raise Exception(full_command)
        return retcode

    @_exceptionWrapper(_errCodes.ENTITY_SSH_CMD_FAILED, "SSH cmd failed")
    def RunSshCmdWithOutput(self, command, ignore_failure = False):
        date_command = "%s %s \"date\"" % (self.ssh_pfx, self.ssh_host)
        os.system(date_command)
        full_command = "%s %s %s" % (self.ssh_pfx, self.ssh_host, command)
        print(full_command)
        cmd0 = list(filter(None, full_command.split(" ")))
        stdout, stderr = subprocess.Popen(cmd0, stdout=subprocess.PIPE).communicate()
        print ("Cmd output ", full_command, stdout, stderr)
        return str(stdout, "UTF-8"), ""

    def __run_cmd(self, cmd, trySync=False):
        os.system("date")
        try:
            self.hdl.sendline(cmd)
            self.hdl.expect("#")
        except pexpect.TIMEOUT:
            if trySync:
                self.__syncLine(self.hdl)
                self.hdl.sendline(cmd)
                self.hdl.expect("#")
            else:
                raise

    def clear_buffer(self):
        try:
            #Clear buffer
            self.hdl.read_nonblocking(1000000000, timeout = 3)
        except:
            pass

    def RunCommandOnConsoleWithOutput(self, cmd, trySync=False):
        self.clear_buffer()
        self.__run_cmd(cmd, trySync)
        return self.hdl.before.decode('utf-8')

    @_exceptionWrapper(_errCodes.ENTITY_COPY_FAILED, "Entity command failed")
    def CopyIN(self, src_filename, entity_dir):
        dest_filename = entity_dir + "/" + os.path.basename(src_filename)
        cmd = "%s %s %s:%s" % (self.scp_pfx, src_filename, self.ssh_host, dest_filename)
        print(cmd)
        ret = os.system(cmd)
        if ret:
            raise Exception("Enitity : {}, src : {}, dst {} ".format(self.ipaddr, src_filename, dest_filename))

        self.RunSshCmd("sync")
        ret = self.RunSshCmd("ls -l %s" % dest_filename)
        if ret:
            raise Exception("Enitity : {}, src : {}, dst {} ".format(self.ipaddr, src_filename, dest_filename))

    @_exceptionWrapper(_errCodes.ENTITY_COPY_FAILED, "Entity command failed")
    def CopyOut(self, src_filename, entity_dir=""):
        if entity_dir:
            if not entity_dir.endswith("/"):
                entity_dir = entity_dir + "/"
        dest_filename = entity_dir + os.path.basename(src_filename)
        cmd = "%s %s:%s %s" % (self.scp_pfx, self.ssh_host, src_filename, dest_filename)
        print(cmd)
        ret = os.system(cmd)
        if ret:
            raise Exception("Enitity : {}, src : {}:{}, dst {} ".format(self.ipaddr, self.ssh_host, src_filename, dest_filename))
        self.RunSshCmd("sync")
        ret = self.RunSshCmd("ls -l %s" % dest_filename)
        if ret:
            raise Exception("Enitity : {}, src : {}:{}, dst {} ".format(self.ipaddr, self.ssh_host, src_filename, dest_filename))

    @_exceptionWrapper(_errCodes.NAPLES_CMD_FAILED, "Naples command failed")
    def RunNaplesCmd(self, command, ignore_failure = False):
        ret = {}
        assert(ignore_failure == True or ignore_failure == False)
        for naples_inst in self.host.naples:
            full_command = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no root@%s %s" %\
                           (naples_inst.password, naples_inst.ipaddr, command)
            ret[naples_inst.GetName()] = self.RunSshCmd(full_command, ignore_failure)
        return ret

class NaplesManagement(EntityManagement):
    def __init__(self, nic_spec, fw_images = None):
        super().__init__(ipaddr = None, username = nic_spec.NaplesUsername, password = nic_spec.NaplesPassword, fw_images = fw_images)
        self.nic_spec = nic_spec
        self.gold_fw_latest = False
        return

    def GetName(self):
        return getattr(self.nic_spec, "NaplesName", "NA")

    def SetHost(self, host):
        self.host = host

    def IsNaplesGoldFWLatest(self):
        return self.gold_fw_latest

    @_exceptionWrapper(_errCodes.NAPLES_TELNET_CLEARLINE_FAILED, "Failed to clear line")
    def __clearline(self):
        try:
            print("Clearing Console Server Line")
            hdl = self.Spawn("telnet %s" % self.nic_spec.ConsoleIP, self.nic_spec.NaplesName)
            idx = hdl.expect(["Username:", "Password:"])
            if idx == 0:
                self.SendlineExpect(self.nic_spec.ConsoleUsername, "Password:", hdl = hdl)
            self.SendlineExpect(self.nic_spec.ConsolePassword, "#", hdl = hdl)

            for i in range(6):
                time.sleep(5)
                self.SendlineExpect("clear line %d" % (self.nic_spec.ConsolePort - 2000), "[confirm]", hdl = hdl)
                self.SendlineExpect("", " [OK]", hdl = hdl)
            hdl.close()
        except:
            raise Exception("Clear line failed ")

    def __run_dhclient(self):
        try:
            self.SendlineExpect("dhclient " + GlobalOptions.mgmt_intf, "#", timeout = 10)
        except:
            #Send Ctrl-c as we did not get IP
            self.SendlineExpect('\003', "#")

    @_exceptionWrapper(_errCodes.NAPLES_LOGIN_FAILED, "Failed to login to naples")
    def __login(self, force_connect=True):
        for _ in range(4):
            try:
                midx = self.SendlineExpect("", ["#", "capri login:", "capri-gold login:"],
                                    hdl = self.hdl, timeout = 30)
                if midx == 0: return
                # Got capri login prompt, send username/password.
                self.SendlineExpect(self.username, "Password:")
                ret = self.SendlineExpect(self.password, ["#", pexpect.TIMEOUT], timeout = 3)
                if ret == 1: self.SendlineExpect("", "#")
                #login successful
                self.SyncLine()
                return
            except:
                print("failed to login, trying again")
        #try ipmi reset as final option
        if force_connect:
            self.IpmiResetAndWait()
        else:
            raise

    @_exceptionWrapper(_errCodes.NAPLES_GOLDFW_REBOOT_FAILED, "Failed to login to naples")
    def RebootGoldFw(self):
        self.InitForUpgrade(goldfw = True)
        if not self.host.PciSensitive():
            self.SendlineExpect("reboot", "capri-gold login:")
        else:
            self.host.Reboot()
            self.host.WaitForSsh()
            self.SendlineExpect("", "capri-gold login:")
        self.__login()
        print("sleeping 60 seconds in RebootGoldFw")
        time.sleep(60)
        self.ReadExternalIP()
        self.WaitForSsh()

    def StartSSH(self):
        self.SendlineExpect("echo classic > /sysconfig/config0/app-start.conf", "#")
        self.SendlineExpect("/etc/init.d/S50sshd start", "#")

    def Reboot(self):
        if not self.host.PciSensitive():
            self.hdl.sendline('reboot')
            self.hdl.expect_exact('Starting kernel',120)
            self.hdl.expect_exact(["#", "capri login:", "capri-gold login:"],120)
        else:
            self.host.Reboot()
            self.host.WaitForSsh()
            self.hdl.expect_exact('Starting kernel',120)
            time.sleep(5)
        self.__login()

    def RebootAndLogin(self):
        if not self.host.PciSensitive():
            self.hdl.sendline('reboot')
            self.hdl.expect_exact('Starting kernel',120)
            self.hdl.expect_exact(["#", "capri login:", "capri-gold login:"],120)

        self.__login()

    def InstallPrep(self):
        self.SendlineExpect("mount -t ext4 /dev/mmcblk0p6 /sysconfig/config0", "#")
        self.SendlineExpect("mount -t ext4 /dev/mmcblk0p7 /sysconfig/config1", "#")
        self.SendlineExpect("mount -t ext4 /dev/mmcblk0p10 /data", "#")
        self.CleanUpOldFiles()
        self.SetUpInitFiles()
        self.SendlineExpect("umount /sysconfig/config0", "#")
        self.SendlineExpect("umount /sysconfig/config1", "#")
        self.SendlineExpect("umount /data", "#")

    @_exceptionWrapper(_errCodes.NAPLES_FW_INSTALL_FAILED, "Main Firmware Install failed")
    def InstallMainFirmware(self, copy_fw = True):
        self.InstallPrep()
        if copy_fw:
            self.CopyIN(os.path.join(GlobalOptions.wsdir, self.fw_images.image), entity_dir = NAPLES_TMP_DIR)
        self.SendlineExpect("", "#", trySync=True)
        self.SendlineExpect("", "#", trySync=True)
        self.SendlineExpect("/nic/tools/sysupdate.sh -p " + NAPLES_TMP_DIR + "/" + os.path.basename(self.fw_images.image),
                            "#", timeout = UPGRADE_TIMEOUT)
        self.SyncLine()
        #if self.ReadSavedFirmwareType() != FIRMWARE_TYPE_MAIN:
        #    raise Exception('failed to switch firmware to mainfwa')

    @_exceptionWrapper(_errCodes.NAPLES_FW_INSTALL_FAILED, "Gold Firmware Install failed")
    def InstallGoldFirmware(self):
        self.CopyIN(os.path.join(GlobalOptions.wsdir, self.fw_images.gold_fw_img), entity_dir = NAPLES_TMP_DIR)
        self.SendlineExpect("/nic/tools/sysupdate.sh -p " + NAPLES_TMP_DIR + "/" + os.path.basename(self.fw_images.gold_fw_img),
                            "#", timeout = UPGRADE_TIMEOUT)
        self.SendlineExpect("/nic/tools/fwupdate -l", "#", trySync=True)

    def __connect_to_console(self):
        for _ in range(3):
            try:
                self.hdl = self.Spawn("telnet %s %s" % ((self.nic_spec.ConsoleIP, self.nic_spec.ConsolePort)), self.nic_spec.NaplesName)
                midx = self.hdl.expect_exact([ "Escape character is '^]'.", pexpect.EOF])
                if midx == 1:
                    raise Exception("Failed to connect to Console %s %d" % (self.nic_spec.ConsoleIP, self.nic_spec.ConsolePort))
            except:
                try:
                    self.__clearline()
                except:
                    print("Expect Failed to clear line %s %d" % (self.nic_spec.ConsoleIP, self.nic_spec.ConsolePort))
                continue
            break
        else:
            #Did not break, so connection failed.
            msg = "Failed to connect to Console %s %d" % (self.nic_spec.ConsoleIP, self.nic_spec.ConsolePort)
            print(msg)
            raise Exception(msg)


    @_exceptionWrapper(_errCodes.NAPLES_TELNET_FAILED, "Telnet Failed")
    def Connect(self, bringup_oob=True, force_connect=True):
        self.__connect_to_console()
        self.Login(bringup_oob, force_connect)

    def _getMemorySize(self):
        mem_check_cmd = '''cat /proc/iomem | grep "System RAM" | grep "240000000" | cut  -d'-' -f 1'''
        try:
            self.SendlineExpect(mem_check_cmd, "240000000" + '\r\n' + '#', timeout = 1, trySync=True)
            return "8G"
        except:
            return "4G"

    @_exceptionWrapper(_errCodes.NAPLES_MEMORY_SIZE_INCOMPATIBLE, "Memroy size check failed")
    def CheckMemorySize(self, size):
        if self._getMemorySize().lower() != size.lower():
            msg = "Memory size check failed %s %d" % (self.nic_spec.ConsoleIP, self.nic_spec.ConsolePort)
            raise Exception(msg)


    def IsOOBUP(self):
        for _ in range(5):
            output = self.RunCommandOnConsoleWithOutput("ip link | grep " + GlobalOptions.mgmt_intf)
            ifconfig_regexp='state (.+?) mode'
            x = re.findall(ifconfig_regexp, output)
            if len(x) > 0:
                if x[0] == "UP":
                    return True
                if x[0] == "DOWN":
                    return False
            else:
                print("Not able to read oob link state")
        return False

    def ReadExternalIP(self):
        if not self.IsOOBUP():
            print("OOB is not up, not reading external IP")
            return

        self.__run_dhclient()
        for _ in range(5):
            output = self.RunCommandOnConsoleWithOutput("ifconfig " + GlobalOptions.mgmt_intf)
            ifconfig_regexp = "addr:(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})"
            x = re.findall(ifconfig_regexp, output)
            if len(x) > 0:
                self.ipaddr = x[0]
                print("Read OOB IP {0}".format(self.ipaddr))
                self.SSHPassInit()
                return
            else:
                print("Did not Read OOB IP  {0}".format(self.ipaddr))
        print("Not able read OOB IP after 5 retries")

    #if oob is not available read internal IP
    def ReadInternalIP(self):
        for _ in range(5):
            output = self.RunCommandOnConsoleWithOutput("ifconfig int_mnic0")
            ifconfig_regexp = "addr:(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})"
            x = re.findall(ifconfig_regexp, output)
            if len(x) > 0:
                self.ipaddr = x[0]
                print("Read internal IP {0}".format(self.ipaddr))
                self.SSHPassInit()
                return
            else:
                print("Did not Read Internal IP  {0}".format(self.ipaddr))
        raise Exception("Not able read internal IP")


    def __read_mac(self):
        for _ in range(10):
            output = self.RunCommandOnConsoleWithOutput("ip link | grep oob_mnic0 -A 1 | grep ether")
            mac_regexp = '(?:[0-9a-fA-F]:?){12}'
            x = re.findall(mac_regexp, output)
            if len(x) > 0:
                self.mac_addr = x[0]
                print("Read MAC {0}".format(self.mac_addr))
                return
            else:
                print("Did not Read MAC  : o/p {0}, pattern {1}".format(output, x))
            time.sleep(2)
        raise Exception("Not able to read oob mac")

    @_exceptionWrapper(_errCodes.NAPLES_LOGIN_FAILED, "Login Failed")
    def Login(self, bringup_oob=True, force_connect=True):
        self.__login(force_connect)
        self.__read_mac()
        if bringup_oob:
            self.ReadExternalIP()

    @_exceptionWrapper(_errCodes.NAPLES_GOLDFW_UNKNOWN, "Gold FW unknown")
    def ReadGoldFwVersion(self):
        gold_fw_cmd = '''fwupdate -l | jq '.goldfw' | jq '.kernel_fit' | jq '.software_version' | tr -d '"\''''
        try:
            self.SendlineExpect(gold_fw_cmd, self.fw_images.gold_fw_latest_ver + '\r\n' + '#')
            self.gold_fw_latest = True
            print ("Matched gold fw latest")
        except:
            try:
                self.SendlineExpect(gold_fw_cmd, self.fw_images.gold_fw_old_ver)
                self.gold_fw_latest = False
                print ("Matched gold fw older")
            except:
                msg = "Did not match any available gold fw"
                print(msg)
                if self.IsSSHUP():
                    print("SSH working, skipping gold fw version check")
                    self.gold_fw_latest = False
                    return
                raise Exception(msg)

    @_exceptionWrapper(_errCodes.FAILED_TO_READ_FIRMWARE_TYPE, "Failed to read firmware type")
    def ReadRunningFirmwareType(self):
        fwType = self.RunCommandOnConsoleWithOutput("fwupdate -r")
        if re.search('\nmainfw',fwType):
            print('determined running firmware to be type MAIN')
            return FIRMWARE_TYPE_MAIN
        elif re.search('\ngoldfw',fwType):
            print('determined running firmware to be type GOLD')
            return FIRMWARE_TYPE_GOLD
        else:
            print("failed to determine running firmware type from output: {0}".format(fwType))
            return FIRMWARE_TYPE_UNKNOWN

    @_exceptionWrapper(_errCodes.FAILED_TO_READ_FIRMWARE_TYPE, "Failed to read firmware type")
    def ReadSavedFirmwareType(self):
        fwType = self.RunCommandOnConsoleWithOutput("fwupdate -S")
        if re.search('\nmainfw',fwType):
            print('determined saved firmware to be type MAIN')
            return FIRMWARE_TYPE_MAIN
        elif re.search('\ngoldfw',fwType):
            print('determined saved firmware to be type GOLD')
            return FIRMWARE_TYPE_GOLD
        else:
            print("failed to determine saved firmware type from output: {0}".format(fwType))
            return FIRMWARE_TYPE_UNKNOWN

    def CleanUpOldFiles(self):
        self.SendlineExpect("clear_nic_config.sh remove-config", "#")
        self.SendlineExpect("rm -rf /sysconfig/config0/*.db", "#")
        self.SendlineExpect("rm -rf /sysconfig/config0/*.conf", "#")
        self.SendlineExpect("rm -rf /sysconfig/config1/*.db", "#")
        self.SendlineExpect("rm -rf /sysconfig/config1/*.conf", "#")
        self.SendlineExpect("rm -f /sysconfig/config0/clusterTrustRoots.pem", "#")
        self.SendlineExpect("rm -f /sysconfig/config1/clusterTrustRoots.pem", "#")
        self.SendlineExpect("rm -f /sysconfig/config0/frequency.json", "#")

        self.SendlineExpect("rm -rf /data/log && sync", "#")
        self.SendlineExpect("rm -rf /data/sysmgr.json && sync", "#")
        self.SendlineExpect("rm -rf /data/core/* && sync", "#")
        self.SendlineExpect("rm -rf /data/*.dat && sync", "#")
        self.SendlineExpect("rm -rf /obfl/asicerrord_err*", "#")

        CreateConfigConsoleNoAuth()
        self.CopyIN(NAPLES_CONFIG_SPEC_LOCAL,
                    entity_dir = "/sysconfig/config0")

    def SetUpInitFiles(self):
        CreateConfigConsoleNoAuth()
        self.CopyIN(NAPLES_CONFIG_SPEC_LOCAL,
                    entity_dir = "/sysconfig/config0")

    @_exceptionWrapper(_errCodes.NAPLES_INIT_FOR_UPGRADE_FAILED, "Init for upgrade failed")
    def InitForUpgrade(self, goldfw = True, mode = True, uuid = True):

        if goldfw:
            self.SendlineExpect("fwupdate -s goldfw", "#", trySync=True)
        #if self.ReadSavedFirmwareType() != FIRMWARE_TYPE_GOLD:
        #    raise Exception('failed to switch firmware to goldfw')
        self.SyncLine()

    def Close(self):
        if self.hdl:
            self.hdl.close()

        if self.console_logfile:
            self.console_logfile.close()

        return

    def __get_capri_prompt(self):
        self.ipmi_handler()
        match_idx = self.hdl.expect(["Autoboot in 0 seconds", pexpect.TIMEOUT], timeout = 180)
        if match_idx == 1:
            print("WARN: sysreset.sh script did not reset the system. Trying CIMC")
            self.ipmi_handler()
            self.hdl.expect_exact("Autoboot in 0 seconds", timeout = 180)
        self.hdl.sendcontrol('C')
        self.hdl.expect_exact("Capri#")
        return

    @_exceptionWrapper(_errCodes.NAPLES_INIT_FOR_UPGRADE_FAILED, "Force switch to gold fw failed")
    def ForceSwitchToGoldFW(self):
        self.__connect_to_console()
        self.__get_capri_prompt()
        self.hdl.sendline("boot goldfw")
        self.hdl.expect_exact("capri-gold login", timeout = 180)
        self.Login()

    @_exceptionWrapper(_errCodes.NAPLES_INIT_FOR_UPGRADE_FAILED, "Switch to gold fw failed")
    def SwitchToGoldFW(self):
        self.SendlineExpect("fwupdate -s goldfw", "#", trySync=True)
        self.SyncLine()
        #if self.ReadSavedFirmwareType() != FIRMWARE_TYPE_GOLD:
        #    raise Exception('failed to switch firmware to goldfw')

    def Close(self):
        if self.hdl:
            self.hdl.close()
        return

class HostManagement(EntityManagement):
    def __init__(self, ipaddr, server_type, host_username, host_password, fw_images):
        super().__init__(ipaddr, host_username, host_password, fw_images)
        self.naples = None
        self.server = server_type
        self.__host_os = None

    def SetNaples(self, naples):
        self.naples = naples
        self.SetHost(self)

    def PciSensitive(self):
        #return self.server == "hpe"
        return True

    def SetNodeOs(self, os):
        self.__host_os = os

    def GetPrimaryIntNicMgmtIpNext(self):
        nxt = str((int(re.search('\.([\d]+)$',GlobalOptions.mnic_ip).group(1))+1)%255)
        return re.sub('\.([\d]+)$','.'+nxt,GlobalOptions.mnic_ip)

    def GetPrimaryIntNicMgmtIp(self):
        return GlobalOptions.mnic_ip

    @_exceptionWrapper(_errCodes.HOST_INIT_FAILED, "Host Init Failed")
    def Init(self, driver_pkg = None, cleanup = True, gold_fw = False):
        self.WaitForSsh()
        os.system("date")
        nodeinit_args = " --own_ip " + self.GetPrimaryIntNicMgmtIpNext() + " --trg_ip " + self.GetPrimaryIntNicMgmtIp()

        if GlobalOptions.skip_driver_install:
            nodeinit_args += " --skip-install"

        node_init_script = os.path.join(GlobalOptions.wsdir, 'iota', 'scripts', self.__host_os, 'nodeinit.sh')
        pen_nics_script = os.path.join(GlobalOptions.wsdir, 'iota', 'scripts', 'pen_nics.py')
        if cleanup:
            nodeinit_args += " --cleanup"
            self.RunSshCmd("sudo rm -rf /naples &&  sudo mkdir -p /naples && sudo chown vm:vm /naples")
            self.RunSshCmd("sudo mkdir -p /pensando && sudo chown vm:vm /pensando")
            self.CopyIN(node_init_script, HOST_NAPLES_DIR)
            print('running nodeinit.sh cleanup with args: {0}'.format(nodeinit_args))
            self.RunSshCmd("sudo %s/nodeinit.sh %s" % (HOST_NAPLES_DIR, nodeinit_args))

        if GlobalOptions.skip_driver_install:
            print('user requested to skip driver install')
            return

        if driver_pkg:
            print('running nodeinit.sh cleanup with args: {0}'.format(nodeinit_args))
            self.RunSshCmd("sudo rm -rf /naples &&  sudo mkdir -p /naples && sudo chown vm:vm /naples")
            self.RunSshCmd("sudo mkdir -p /pensando && sudo chown vm:vm /pensando")
            self.CopyIN(pen_nics_script,  HOST_NAPLES_DIR)
            self.CopyIN(node_init_script, HOST_NAPLES_DIR)
            self.CopyIN(os.path.join(GlobalOptions.wsdir, driver_pkg), HOST_NAPLES_DIR)

            nodeinit_args = ""
            #Run with not mgmt first
            if gold_fw or not GlobalOptions.no_mgmt:
                self.RunSshCmd("sudo %s/nodeinit.sh --no-mgmt" % (HOST_NAPLES_DIR))
                #mgmtIPCmd = "sudo python5  %s/pen_nics.py --mac-hint %s --intf-type int-mnic --op mnic-ip --os %s" % (HOST_NAPLES_DIR, self.naples.mac_addr, self.__host_os)
                #output, errout = self.RunSshCmdWithOutput(mgmtIPCmd)
                #print("Command output ", output)
                #mnic_ip = ipaddress.ip_address(output.split("\n")[0])
                #own_ip = str(mnic_ip + 1)
                #nodeinit_args = " --own_ip " + own_ip + " --trg_ip " + str(mnic_ip)
                nodeinit_args = " --own_ip " + self.GetPrimaryIntNicMgmtIpNext() + " --trg_ip " + self.GetPrimaryIntNicMgmtIp()
            else:
                nodeinit_args += " --no-mgmt"
            self.RunSshCmd("sudo %s/nodeinit.sh %s" % (HOST_NAPLES_DIR, nodeinit_args))
        return

    @_exceptionWrapper(_errCodes.HOST_COPY_FAILED, "Host Init Failed")
    def CopyIN(self, src_filename, entity_dir, naples_dir = None):
        dest_filename = entity_dir + "/" + os.path.basename(src_filename)
        super(HostManagement, self).CopyIN(src_filename, entity_dir)
        if naples_dir:
            naples_dest_filename = naples_dir + "/" + os.path.basename(src_filename)
            for naples_inst in self.naples:
                ret = self.RunSshCmd("sshpass -p %s scp -o UserKnownHostsFile=/dev/null  -o StrictHostKeyChecking=no %s %s@%s:%s" %\
                               (naples_inst.password, dest_filename, naples_inst.username, naples_inst.ipaddr, naples_dest_filename))
                if ret:
                    raise Exception("Copy to Naples failed")
        return 0

    @_exceptionWrapper(_errCodes.HOST_RESTART_FAILED, "Host restart Failed")
    def Reboot(self, dryrun = False):
        os.system("date")
        self.RunSshCmd("sync")
        self.RunSshCmd("ls -l /tmp/")
        self.RunSshCmd("uptime")
        print("Rebooting Host : %s" % self.ipaddr)
        if dryrun == False:
            self.RunSshCmd("sudo shutdown -r now", ignore_failure = True)
            print("sleeping 60 after shutdown -r in Reboot")
            time.sleep(60)
            self.WaitForSsh()
        self.RunSshCmd("uptime")
        return

    def InstallPrep(self):
        self.RunNaplesCmd("/nic/tools/fwupdate -r | grep goldfw")
        self.RunNaplesCmd("mkdir -p /data && sync")
        self.RunNaplesCmd("mount -t ext4 /dev/mmcblk0p6 /sysconfig/config0")
        self.RunNaplesCmd("mount -t ext4 /dev/mmcblk0p7 /sysconfig/config1")
        #Clean up old files as we are starting fresh.
        self.CleanUpOldFiles()
        self.SetUpInitFiles()
        #unmount
        self.RunNaplesCmd("umount /sysconfig/config0")
        self.RunNaplesCmd("umount /sysconfig/config1")

    @_exceptionWrapper(_errCodes.NAPLES_FW_INSTALL_FROM_HOST_FAILED, "FW install Failed")
    def InstallMainFirmware(self, mount_data = True, copy_fw = True):
        try: self.RunSshCmd("sudo lspci -d 1dd8:")
        except:
            print('lspci failed to find nic. calling ipmi power cycle')
            self.IpmiResetAndWait()
        self.InstallPrep()

        if copy_fw:
            self.CopyIN(os.path.join(GlobalOptions.wsdir, self.fw_images.image), entity_dir = HOST_NAPLES_DIR, naples_dir = NAPLES_TMP_DIR)

        self.RunNaplesCmd("/nic/tools/sysupdate.sh -p /%s/%s"%(NAPLES_TMP_DIR, os.path.basename(self.fw_images.image)))
        self.RunNaplesCmd("/nic/tools/fwupdate -l")
        return


    @_exceptionWrapper(_errCodes.NAPLES_FW_INSTALL_FROM_HOST_FAILED, "Gold FW install Failed")
    def InstallGoldFirmware(self):
        self.CopyIN(os.path.join(GlobalOptions.wsdir, self.fw_images.gold_fw_img), entity_dir = HOST_NAPLES_DIR, naples_dir = "/data")
        self.RunNaplesCmd("/nic/tools/sysupdate.sh -p /data/" +  os.path.basename(self.fw_images.gold_fw_img))
        self.RunNaplesCmd("/nic/tools/fwupdate -l")

    def InitForUpgrade(self):
        pass

    def InitForReboot(self):
        pass

    def UnloadDriver(self):
        pass

    def CleanUpOldFiles(self):
        #clean up db that was setup by previous
        self.RunNaplesCmd("rm -rf /sysconfig/config0/*.db")
        self.RunNaplesCmd("rm -rf /sysconfig/config0/*.conf")
        self.RunNaplesCmd("rm -rf /sysconfig/config1/*.db")
        self.RunNaplesCmd("rm -rf /sysconfig/config1/*.conf")
        self.RunNaplesCmd("rm -f /sysconfig/config0/clusterTrustRoots.pem")
        self.RunNaplesCmd("rm -f /sysconfig/config1/clusterTrustRoots.pem")
        self.RunNaplesCmd("rm -f /sysconfig/config0/frequency.json")

        self.RunNaplesCmd("rm -rf /data/log && sync")
        self.RunNaplesCmd("rm -rf /data/core/* && sync")
        self.RunNaplesCmd("rm -rf /data/*.dat && sync")

    def SetUpInitFiles(self):
        CreateConfigConsoleNoAuth()
        self.CopyIN(NAPLES_CONFIG_SPEC_LOCAL,
                    entity_dir = "/tmp", naples_dir = "/sysconfig/config0")


class EsxHostManagement(HostManagement):
    def __init__(self, ipaddr, server_type, host_username, host_password, fw_images, driver_images):
        HostManagement.__init__(self, ipaddr, server_type, host_username, host_password, fw_images)
        self.driver_images = driver_images
        if GlobalOptions.esx_script is None:
            GlobalOptions.esx_script = ESX_CTRL_VM_BRINGUP_SCRIPT

    @_exceptionWrapper(_errCodes.HOST_ESX_CTRL_VM_COPY_FAILED, "ESX ctrl vm copy failed")
    def ctrl_vm_copyin(self, src_filename, entity_dir, naples_dir = None):
        dest_filename = entity_dir + "/" + os.path.basename(src_filename)
        cmd = "%s %s %s:%s" % (self.__ctr_vm_scp_pfx, src_filename,
                               self.__ctr_vm_ssh_host, dest_filename)
        print(cmd)
        ret = os.system(cmd)
        if ret:
            raise Exception("Cmd failed : " + cmd)

        self.ctrl_vm_run("sync")
        self.ctrl_vm_run("ls -l %s" % dest_filename)

        if naples_dir:
            naples_dest_filename = naples_dir + "/" + os.path.basename(src_filename)
            for naples_inst in self.naples:
                ret = self.ctrl_vm_run("sshpass -p %s scp -o UserKnownHostsFile=/dev/null  -o StrictHostKeyChecking=no %s %s@%s:%s" %\
                               (naples_inst.password, dest_filename, naples_inst.username, naples_inst.ipaddr, naples_dest_filename))
                if ret:
                    raise Exception("Cmd failed : " + cmd)

        return 0

    @_exceptionWrapper(_errCodes.NAPLES_CMD_FAILED, "Naples command failed")
    def RunNaplesCmd(self, command, ignore_failure = False):
        ret = []
        assert(ignore_failure == True or ignore_failure == False)
        for naples_inst in self.naples:
            full_command = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null  -o StrictHostKeyChecking=no %s@%s %s" %\
                           (naples_inst.password, naples_inst.username, naples_inst.ipaddr, command)
            ret.append(self.ctrl_vm_run(full_command, ignore_failure))
        return ret

    @_exceptionWrapper(_errCodes.HOST_ESX_CTRL_VM_RUN_CMD_FAILED, "ESX ctrl vm run failed")
    def ctrl_vm_run(self, command, background = False, ignore_result = False):
        if background:
            cmd = "%s -f %s \"%s\"" % (self.__ctr_vm_ssh_pfx, self.__ctr_vm_ssh_host, command)
        else:
            cmd = "%s %s \"%s\"" % (self.__ctr_vm_ssh_pfx, self.__ctr_vm_ssh_host, command)
        print(cmd)
        retcode = os.system(cmd)
        if retcode and not ignore_result:
            raise Exception("Cmd run failed "  + cmd)
        return retcode

    @_exceptionWrapper(_errCodes.HOST_COPY_FAILED, "Host Init Failed")
    def CopyIN(self, src_filename, entity_dir, naples_dir = None):
        if naples_dir:
            self.ctrl_vm_copyin(src_filename, entity_dir, naples_dir = naples_dir)
        else:
            super(HostManagement, self).CopyIN(src_filename, entity_dir)

    def __check_naples_deivce(self):
        try: self.RunSshCmd("lspci -d 1dd8:")
        except:
            print('lspci failed to find nic. calling ipmi power cycle')
            self.IpmiResetAndWait()

    @_exceptionWrapper(_errCodes.HOST_ESX_CTRL_VM_INIT_FAILED, "Ctrl VM init failed")
    def __esx_host_init(self):
        self.WaitForSsh(port=443)
        time.sleep(30)
        if all(n.IsSSHUP() for n in self.naples):
            print ("All Naples OOB is up, skipping ctrl vm initialization.")
            return
        # Use first instance of naples
        naples_inst = self.naples[0]
        outFile = "/tmp/esx_" +  self.ipaddr + ".json"
        esx_startup_cmd = ["timeout", "2400"]
        esx_startup_cmd.extend([GlobalOptions.esx_script])
        esx_startup_cmd.extend(["--esx-host", self.ipaddr])
        esx_startup_cmd.extend(["--esx-username", self.username])
        esx_startup_cmd.extend(["--esx-password", self.password])
        esx_startup_cmd.extend(["--esx-outfile", outFile])
        esx_startup_cmd.extend(["--mac-hint", naples_inst.mac_addr])
        proc_hdl = subprocess.Popen(esx_startup_cmd, stdout=sys.stdout.f, stderr=sys.stderr)
        while proc_hdl.poll() is None:
            time.sleep(5)
            continue
        if proc_hdl.returncode:
            raise Exception("Failed to setup control VM on ESX")
        with open(outFile) as f:
            data = json.load(f)
            self.__esx_ctrl_vm_ip = data["ctrlVMIP"]
            self.__esx_ctrl_vm_username = data["ctrlVMUsername"]
            self.__esx_ctrl_vm_password = data["ctrlVMPassword"]
        self.__ctr_vm_ssh_host = "%s@%s" % (self.__esx_ctrl_vm_username, self.__esx_ctrl_vm_ip)
        self.__ctr_vm_scp_pfx = "sshpass -p %s scp -o UserKnownHostsFile=/dev/null  -o StrictHostKeyChecking=no " % self.__esx_ctrl_vm_password
        self.__ctr_vm_ssh_pfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null  -o StrictHostKeyChecking=no " % self.__esx_ctrl_vm_password

    @_exceptionWrapper(_errCodes.HOST_ESX_INIT_FAILED, "Host init failed")
    def Init(self, driver_pkg = None, cleanup = True, gold_fw = False):
        self.WaitForSsh()
        os.system("date")
        self.__check_naples_deivce()
        self.__esx_host_init()

    @_exceptionWrapper(_errCodes.HOST_DRIVER_INSTALL_FAILED, "ESX Driver install failed")
    def __install_drivers(self, pkg):
        if GlobalOptions.skip_driver_install:
            print('user requested to skip driver install')
            return
        # Install IONIC driver package.
        #ESX removes folder after reboot, add it again
        self.RunSshCmd("rm -rf %s" % HOST_NAPLES_DIR)
        self.RunSshCmd("mkdir -p %s" % HOST_NAPLES_DIR)

        self.CopyIN(os.path.join(GlobalOptions.wsdir, pkg), HOST_NAPLES_DIR)
        assert(self.RunSshCmd("cd %s && tar xf %s" %\
                 (HOST_NAPLES_DIR, os.path.basename(pkg))) == 0)
        install_success = False
        self.__host_connect()
        for _ in range(0, 5):

            stdin, stdout, stderr  = self.__ssh_handle.exec_command("cd %s/drivers-esx-eth/ && chmod +x ./build.sh && ./build.sh --install" % HOST_NAPLES_DIR)
            #ret = self.run("cd %s/drivers-esx/ && chmod +x ./build.sh && ./build.sh" % HOST_NAPLES_DRIVERS_DIR, ignore_result = True)
            exit_status = stdout.channel.recv_exit_status()
            outlines=stdout.readlines()
            print (''.join(outlines))
            if  exit_status == 0:
            #if ret == 0:
                install_success = True
                break
            print("Installed failed , trying again..")
            time.sleep(5)
            #self._connect()
            self.__host_connect()
        if not install_success:
            raise Exception("Driver install failed")


    @_exceptionWrapper(_errCodes.ENTITY_NOT_UP, "Host not up")
    def WaitForSsh(self, port = 22):
        super().WaitForSsh(port)
        super().WaitForSsh(443)
        print("sleeping 30 seconds after WaitForSsh")
        time.sleep(30)

    @_exceptionWrapper(_errCodes.NAPLES_FW_INSTALL_FROM_HOST_FAILED, "FW install Failed")
    def InstallMainFirmware(self, mount_data = True, copy_fw = True):
        self.InstallPrep()

        #Ctrl VM reboot might have removed the image
        self.ctrl_vm_copyin(os.path.join(GlobalOptions.wsdir, self.fw_images.image),
                    entity_dir = HOST_ESX_NAPLES_IMAGES_DIR,
                    naples_dir = "/data")

        self.RunNaplesCmd("/nic/tools/sysupdate.sh -p /data/%s"%os.path.basename(self.fw_images.image))

        self.RunNaplesCmd("/nic/tools/fwupdate -l")
        return

    @_exceptionWrapper(_errCodes.NAPLES_FW_INSTALL_FAILED, "Gold Firmware Install failed")
    def InstallGoldFirmware(self):
        self.ctrl_vm_copyin(os.path.join(GlobalOptions.wsdir, self.fw_images.gold_fw_img),
                    entity_dir = HOST_ESX_NAPLES_IMAGES_DIR,
                    naples_dir = "/data")
        self.RunNaplesCmd("/nic/tools/sysupdate.sh -p /data/" +  os.path.basename(self.fw_images.gold_fw_img))
        self.RunNaplesCmd("/nic/tools/fwupdate -l")

    @_exceptionWrapper(_errCodes.HOST_INIT_FOR_UPGRADE_FAILED, "Init for upgrade failed")
    def InitForUpgrade(self):
        if all(n.IsNaplesGoldFWLatest() for n in self.naples):
            gold_pkg = self.driver_images.gold_drv_latest_pkg
        else:
            gold_pkg = self.driver_images.gold_drv_old_pkg
        self.__install_drivers(gold_pkg)

    @_exceptionWrapper(_errCodes.HOST_INIT_FOR_REBOOT_FAILED, "Init for reboot failed")
    def InitForReboot(self):
        self.__install_drivers(self.driver_images.drivers_pkg)


    def UnloadDriver(self):
        def __unload_driver(self):
            self.__host_connect()
            self.__ssh_handle.get_transport().set_keepalive(10)
            stdin, stdout, stderr = self.__ssh_handle.exec_command("vmkload_mod -u ionic_en", timeout=10)
            exit_status = stdout.channel.recv_exit_status()
        t=threading.Thread(target=__unload_driver, args=[self])
        t.start()
        t.join(timeout=10)
        if t.isAlive():
            print ("Unload Driver failed...")
            raise Exception("Unload failed")

    def __host_connect(self):
        ip=self.ipaddr
        port='22'
        username=self.username
        password=self.password
        ssh=paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh.connect(ip,port,username,password)
        self.__ssh_handle = ssh

    @_exceptionWrapper(_errCodes.HOST_ESX_REBOOT_FAILED, "ESX reboot failed")
    def Reboot(self, dryrun = False):
        os.system("date")
        self.RunSshCmd("sync")
        self.RunSshCmd("uptime")
        if dryrun == False:
            self.RunSshCmd("reboot", ignore_failure = True)
        print("sleeping 120 seconds after run ssh reboot")
        time.sleep(120)
        print("Rebooting Host : %s" % self.ipaddr)
        return


class PenOrchestrator:

    def __init__(self):
        self.__naples = list()
        self.__host = None
        self.__img_manifest = None
        self.__driver_images = None
        self.__fw_images = None
        self.__server_type = None
        self.__testbed = None
        self.__node_os = "linux"
        self.__host_username = None
        self.__host_password = None

        # This variable introduced to control successive ipmi-reset in case of multiple Naples
        self.__ipmi_reboot_allowed = False  

        self.__load_testbed_json()
        self.__load_image_manifest()

    def __load_image_manifest(self):
        self.__img_manifest = jparser.JsonParse(GlobalOptions.image_manifest)
        self.__driver_images = list(filter(lambda x: x.OS == self.__node_os, self.__img_manifest.Drivers))[0]
        self.__fw_images = list(filter(lambda x: x.naples_type == GlobalOptions.naples_type, self.__img_manifest.Firmwares))[0]
        if self.__driver_images is None or self.__fw_images is None:
            sys.stderr.write("Unable to load image manifest")
            sys.exit(1)

    def __load_testbed_json(self):
        warmd = jparser.JsonParse(GlobalOptions.testbed)
        self.__testbed = list(filter(lambda x: x.Name == GlobalOptions.instance_name, warmd.Instances))[0]
        if self.__testbed is None:
            sys.stderr.write("Unable to load warmd.json")
            sys.exit(1)

        # Precheck Nics for naples type
        if getattr(self.__testbed, 'Nics', None):
            # warmd.json has Nics
            if any(nic.Type == "naples" for nic in self.__testbed.Nics):
                print("Found naples-type NICs to initialize testbed")
            else:
                print("No Naples-type NICs found in testbed - No-Op")
                sys.exit(0)

            if getattr(self.__testbed.Resource, '__server_type', 'server-a') == 'hpe':
                self.__server_type = 'hpe'
            else:
                self.__server_type = getattr(self.__testbed, 'NodeServer', 'ucs')

            username = getattr(self.__testbed, 'NodeCimcUsername', "")
            if username == "":
                setattr(self.__testbed, 'NodeCimcUsername', 'admin')
            passwd = getattr(self.__testbed, 'NodeCimcPassword', "")
            if passwd == "":
                setattr(self.__testbed, 'NodeCimcPassword', 'N0isystem$')
            # Update standard values
            for nic in self.__testbed.Nics:
                setattr(nic, 'NaplesUsername', GlobalOptions.naples_username)
                setattr(nic, 'NaplesPassword', GlobalOptions.naples_password)
                username = getattr(nic, 'ConsoleUsername', "")
                if username == "":
                    setattr(nic, 'ConsoleUsername', 'admin')
                passwd = getattr(nic, 'ConsolePassword', "")
                if passwd == "": 
                    setattr(nic, 'ConsolePassword', 'N0isystem$')
                setattr(nic, 'ConsolePort', int(getattr(nic, 'ConsolePort')))

            # Derive NodeOs from the warmd.json Provision.Vars section
            if hasattr(warmd.Provision, "Vars") and hasattr(warmd.Provision.Vars, 'BmOs') and self.__testbed.Type == "bm":
                self.__node_os = warmd.Provision.Vars.BmOs
            if hasattr(warmd.Provision, "Vars") and hasattr(warmd.Provision.Vars, 'VmOs') and self.__testbed.Type == "vm":
                self.__node_os = warmd.Provision.Vars.VmOs

            # Derive Host Username and Password based on NodeOs
            if self.__node_os == "esx":
                self.__host_username = warmd.Provision.Vars.EsxUsername
                self.__host_password = warmd.Provision.Vars.EsxPassword
            else:
                self.__host_username = warmd.Provision.Username
                self.__host_password = warmd.Provision.Password

        else:
            # tbXY.json has None
            setattr(self.__testbed, 'NodeCimcUsername', 'admin') 
            setattr(self.__testbed, 'NodeCimcPassword', 'N0isystem$')

            nic = jparser.Dict2Object({})
            setattr(nic, 'NaplesUsername', GlobalOptions.naples_username)
            setattr(nic, 'NaplesPassword', GlobalOptions.naples_password)
            setattr(nic, 'ConsoleUsername', 'admin') 
            setattr(nic, 'ConsolePassword', 'N0isystem$')
            setattr(nic, 'ConsoleIP', self.__testbed.NicConsoleIP) 
            setattr(nic, 'ConsolePort', int(self.__testbed.NicConsolePort))
            setattr(nic, 'ConsolePassword', 'N0isystem$')
            self.__node_os = self.__testbed.NodeOs
            if self.__node_os == "esx": # some of the testbed json has no EsxUsername and EsxPassword
                self.__host_username = getattr(warmd.Provision, 'EsxUsername', 'root')
                self.__host_password = getattr(warmd.Provision, 'EsxPassword', 'pen123!')
            else:
                self.__host_username = warmd.Provision.Username
                self.__host_password = warmd.Provision.Password
            self.__testbed.Nics = [nic]

        print("Found %d Naples with host %s in testbed" % (len(self.__testbed.Nics), GlobalOptions.instance_name))
        for nic in self.__testbed.Nics: 
            name = "naples_%s_%s" % (nic.ConsoleIP, nic.ConsolePort)
            setattr(nic, 'NaplesName', name)

    def AtExitCleanup(self):
        if not self.__naples:
            return
        for naples_inst in self.__naples:
            try: 
                naples_inst.Connect(bringup_oob=(not GlobalOptions.auto_discover)) # Make sure it is connected
                naples_inst.SendlineExpect("/nic/tools/fwupdate -l", "#", trySync=True)
                naples_inst.Close()
            except: 
                print("failed to read firmware. error was: {0}".format(traceback.format_exc()))

    def __doNaplesReboot(self):

        if self.__host.PciSensitive():
            self.__host.Reboot()
            self.__host.WaitForSsh()
            time.sleep(5)

        for naples_inst in self.__naples:
            # Naples would have rebooted to, login again.
            naples_inst.Connect()
            naples_inst.RebootAndLogin()

            if GlobalOptions.mem_size:
                naples_inst.CheckMemorySize(GlobalOptions.mem_size)

    def NaplesOnlySetup(self):

        if GlobalOptions.only_init == True:
            return

        if self.__node_os == 'esx':
            self.__host = EsxHostManagement(self.__testbed.NodeMgmtIP, self.__server_type,
                                            self.__host_username, self.__host_password,
                                            self.__fw_images, self.__driver_images)
        else:
            self.__host = HostManagement(self.__testbed.NodeMgmtIP, self.__server_type, 
                                         self.__host_username, self.__host_password, 
                                         self.__fw_images)
        self.__host.SetIpmiHandler(self.IpmiReset)
        self.__host.SetNodeOs(self.__node_os)

        for nic in self.__testbed.Nics:
            naples_inst = NaplesManagement(nic, fw_images = self.__fw_images) 
            naples_inst.SetHost(self.__host)
            naples_inst.SetIpmiHandler(self.IpmiReset)
            self.__naples.append(naples_inst)

        self.__host.SetNaples(self.__naples)

        if GlobalOptions.only_mode_change == True:
            # Case 3: Only INIT option.
            self.__doNaplesReboot()
            return

        #First do a reset as naples may be in screwed up state.
        run_init_for_ugrade = False
        self.__ipmi_reboot_allowed = True
        for naples_inst in self.__naples:
            try:
                naples_inst.Connect()
                if not self.__host.IsSSHUP():
                    raise Exception("Host not up.")
            except:
                #Do Reset only if we can't connect to naples.
                self.IpmiReset()
                self.__ipmi_reboot_allowed = False
                time.sleep(10)
                run_init_for_ugrade = True
                break
        self.__ipmi_reboot_allowed = True

        if run_init_for_ugrade:
            # TODO: Since we are going to do IpmiReset second time. call InitForUpgrade for all naples
            for naples_inst in self.__naples:
                naples_inst.Connect()
                naples_inst.InitForUpgrade(goldfw = True)
            #Do a reset again as old fw might lock up host boot
            self.__ipmi_reboot_allowed = True
            self.IpmiReset()
            if not self.__host.WaitForSsh():
                raise Exception("Host not up after 2nd IpmiReset")

        for naples_inst in self.__naples:
            if GlobalOptions.mem_size:
                naples_inst.CheckMemorySize(GlobalOptions.mem_size)

            #Read Naples Gold FW version.
            naples_inst.ReadGoldFwVersion()

            # Case 1: Main firmware upgrade.
            naples_inst.InitForUpgrade(goldfw = True)
            #OOb is present and up install right away,
            naples_inst.RebootGoldFw()
            naples_inst.InstallMainFirmware()
            if not naples_inst.IsNaplesGoldFWLatest():
                naples_inst.InstallGoldFirmware()

        self.__doNaplesReboot()


    # This function is used for 3 cases.
    # 1) Full firmware upgrade
    # 2) Change mode from Classic <--> Hostpin
    # 3) Only initialize the node and start tests.

    def Main(self):
        if self.__node_os == 'esx':
            self.__host = EsxHostManagement(self.__testbed.NodeMgmtIP, self.__server_type, 
                                            self.__host_username, self.__host_password, 
                                            self.__fw_images, self.__driver_images)
        else:
            self.__host = HostManagement(self.__testbed.NodeMgmtIP, self.__server_type, 
                                         self.__host_username, self.__host_password, 
                                         self.__fw_images)

        self.__host.SetIpmiHandler(self.IpmiReset)
        self.__host.SetNodeOs(self.__node_os)

        for nic in self.__testbed.Nics:
            naples_inst = NaplesManagement(nic, fw_images = self.__fw_images)
            naples_inst.SetHost(self.__host)
            naples_inst.SetIpmiHandler(self.IpmiReset)
            self.__naples.append(naples_inst)
        self.__host.SetNaples(self.__naples)

        if GlobalOptions.only_mode_change:
            # Case 2: Only change mode, reboot and install drivers
            #naples.InitForUpgrade(goldfw = False)
            self.__host.Reboot()
            self.__host.WaitForSsh()
            return

        if GlobalOptions.only_init == True:
            # Case 3: Only INIT option.
            self.__host.Init(driver_pkg = self.__driver_images.drivers_pkg, cleanup = True)
            return

        # Reset the setup:
        # If the previous run left it in bad state, we may not get ssh or console.
        #First do a reset as naples may be in screwed up state.

        if GlobalOptions.only_mode_change == False and GlobalOptions.only_init == False:
            try:
                for naples_inst in self.__naples:
                    naples_inst.Connect(force_connect=False)
                    #Read Naples Gold FW version if system in good state.
                    #If not able to read then we will reset
                    naples_inst.ReadGoldFwVersion()
               
                    naples_inst.ReadInternalIP()
                    #Read External IP to try oob path first
                    naples_inst.ReadExternalIP()

                self.__host.WaitForSsh()
                #need to unload driver as host might crash in ESX case.
                #unloading of driver should not fail, else reset to goldfw
                self.__host.UnloadDriver()

            except:
                # Because ForceSwitchToGoldFW is time-sensetive operation (sending Ctrl-c), allowing both IpmiReset
                self.__ipmi_reboot_allowed = True
                for naples_inst in self.__naples:
                    naples_inst.ForceSwitchToGoldFW()

                try:
                    for naples_inst in self.__naples:
                        naples_inst.InitForUpgrade()
                except:
                    pass

                self.__host.WaitForSsh()
                self.__host.UnloadDriver()
        else:
            for naples_inst in self.__naples:
                naples_inst.Connect(bringup_oob=(not GlobalOptions.auto_discover))
                self.__host.WaitForSsh()

        self.__ipmi_reboot_allowed = True
        install_mainfw_via_host = False
        for naples_inst in self.__naples:
            naples_inst.StartSSH()
            naples_inst.ReadGoldFwVersion()
            fwType = naples_inst.ReadRunningFirmwareType()
            
            # Case 1: Main firmware upgrade.
            #naples.InitForUpgrade(goldfw = True)
            if naples_inst.IsSSHUP():
                #OOb is present and up install right away,
                print("installing and running tests with firmware {0} without checking goldfw".format(self.__fw_images.image))
                try:
                    naples_inst.InstallMainFirmware()
                    if not naples_inst.IsNaplesGoldFWLatest():
                        naples_inst.InstallGoldFirmware()
                except:
                    print("failed to upgrade main firmware only. error was:")
                    print(traceback.format_exc())
                    print("attempting gold + main firmware update")
                    self.__fullUpdate(naples_inst)
            else:
                naples_inst.ReadInternalIP()
                if fwType != FIRMWARE_TYPE_GOLD:
                    naples_inst.InitForUpgrade(goldfw = True)
                    self.__host.InitForUpgrade()
                    if GlobalOptions.no_mgmt:
                        #If non mgmt, do ipmi reset to make sure we reboot
                        # Since GlobalOptions.no_mgmt would apply to both Naples (?), reset only once
                        naples_inst.IpmiResetAndWait()
                        self.__ipmi_reboot_allowed = False
                    else:
                        self.__host.Reboot()

                if naples_inst.IsNaplesGoldFWLatest():
                    gold_pkg = self.__driver_images.gold_drv_latest_pkg 
                else:
                    gold_pkg = self.__driver_images.gold_drv_old_pkg
                self.__host.Init(driver_pkg =  gold_pkg, cleanup = False, gold_fw = True)
                if GlobalOptions.use_gold_firmware:
                    if fwType != FIRMWARE_TYPE_GOLD:
                        naples_inst.InstallGoldFirmware()
                    else:
                        print('firmware already gold, skipping gold firmware installation')
                else:
                    install_mainfw_via_host = True

        if install_mainfw_via_host:
            # Since we are updating from host-side, this is a one-time operation
            self.__ipmi_reboot_allowed = True
            self.__host.InstallMainFirmware()
            #Install gold fw if required.
            for naples_inst in self.__naples:
                if not naples_inst.IsNaplesGoldFWLatest():
                    self.__host.InstallGoldFirmware()

        #Script that might have to run just before reboot
        # ESX would require drivers to be installed here to avoid
        # onr more reboot
        self.__host.InitForReboot()
        #Do and IP reset to make sure naples and Host are in sync

        self.__ipmi_reboot_allowed=True
        self.IpmiReset() # Do IpmiReset once
        for naples_inst in self.__naples:
            naples_inst.WaitAfterReset()

        #Naples would have rebooted to, login again.
        for naples_inst in self.__naples:
            naples_inst.Connect(bringup_oob=(not GlobalOptions.auto_discover))

        # Common to Case 2 and Case 1.
        # Initialize the Node, this is needed in all cases.
        self.__host.Init(driver_pkg = self.__driver_images.drivers_pkg, cleanup = False)

        #if naples.IsSSHUP():
            # Connect to serial console too
            #naples.Connect()
            #naples.InstallMainFirmware()
        #else:
            # Update MainFwB also to same image - TEMP CHANGE
            # host.InstallMainFirmware(mount_data = False, copy_fw = False)

    def __fullUpdate(self, naples_inst):
        fwType = naples_inst.ReadRunningFirmwareType()
        if fwType != FIRMWARE_TYPE_GOLD:
            naples_inst.ForceSwitchToGoldFW()
            naples_inst.RebootGoldFw()

        if GlobalOptions.use_gold_firmware:
            print("installing and running tests with gold firmware {0}".format(self.__fw_images.gold_fw_img))
            naples_inst.InstallGoldFirmware()
        else:
            print("installing and running tests with firmware {0}".format(self.__fw_images.image))
            naples_inst.InstallMainFirmware()
            if not naples_inst.IsNaplesGoldFWLatest():
                naples_inst.InstallGoldFirmware()
        naples_inst.Reboot()

    def IpmiReset(self):
        if self.__ipmi_reboot_allowed:
            print('calling ipmitool power cycle')
            cmd="ipmitool -I lanplus -H %s -U %s -P %s power cycle" % (
                    self.__testbed.NodeCimcIP, self.__testbed.NodeCimcUsername, 
                    self.__testbed.NodeCimcPassword)
            subprocess.check_call(cmd, shell=True)
        else:
            print("Skipping IPMI Reset")


if __name__ == '__main__':
    start_time = time.time()

    orch = PenOrchestrator()
    atexit.register(orch.AtExitCleanup)
    try:
        if GlobalOptions.naples_only_setup:
            orch.NaplesOnlySetup()
        else:
            orch.Main()
    except bootNaplesException as ex:
        sys.stderr.write(str(ex))
        sys.exit(1)
    elapsed_time = time.strftime("Naples Upgrade/boot time : %H:%M:%S", time.gmtime(time.time() - start_time))
    print(elapsed_time + "\n")
