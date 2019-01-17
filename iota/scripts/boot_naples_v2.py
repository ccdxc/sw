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
import paramiko

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
parser.add_argument('--gold-drivers-pkg', dest='gold_drivers_pkg',
                    default=None, help='Gold Driver Package.')
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
parser.add_argument('--esx-bld-vm-username', dest='esx_bld_vm_username',
                    default="root", help='esx build vm username')
parser.add_argument('--esx-bld-vm-password', dest='esx_bld_vm_password',
                    default="vmware", help='esx build vm password')
parser.add_argument('--esx-bld-vm', dest='esx_bld_vm',
                    default="esx-vib-wb1", help='esx build vm')



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
    time.sleep(60)
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
        date_command = "%s %s \"date\"" % (self.ssh_pfx, self.ssh_host)
        os.system(date_command)
        full_command = "%s %s \"%s\"" % (self.ssh_pfx, self.ssh_host, command)
        print(full_command)
        retcode = os.system(full_command)
        if ignore_failure is False and retcode != 0:
            print("ERROR: Failed to run command: %s" % command)
            sys.exit(1)
        return retcode

    def RunNaplesCmd(self, command, ignore_failure = False):
        assert(ignore_failure == True or ignore_failure == False)
        full_command = "sshpass -p %s ssh -o StrictHostKeyChecking=no root@%s %s" %\
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
        if ret == 1: self.SendlineExpect("", "#")

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

        nodeinit_args = ""
        if cleanup:
            nodeinit_args = "--cleanup"
            self.RunSshCmd("%s/nodeinit.sh %s" % (HOST_NAPLES_DIR, nodeinit_args))

        if driver_pkg:
            nodeinit_args = ""
            self.RunSshCmd("rm -rf /naples && mkdir /naples")
            self.CopyIN("scripts/%s/nodeinit.sh" % GlobalOptions.os, HOST_NAPLES_DIR)
            self.CopyIN(driver_pkg, HOST_NAPLES_DIR)
            self.RunSshCmd("%s/nodeinit.sh %s" % (HOST_NAPLES_DIR, nodeinit_args))
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

    def InstallMainFirmware(self, mount_data = True, copy_fw = True):
        assert(self.RunSshCmd("lspci | grep 1dd8") == 0)
        if mount_data:
            self.RunNaplesCmd("/nic/tools/fwupdate -r | grep goldfw")
            self.RunNaplesCmd("mkdir -p /data && sync")
            self.RunNaplesCmd("mount /dev/mmcblk0p10 /data/")

        if copy_fw:
            self.CopyIN(GlobalOptions.image, host_dir = HOST_NAPLES_DIR, naples_dir = "/data")

        self.RunNaplesCmd("/nic/tools/sysupdate.sh -p /data/naples_fw.tar")
        if mount_data:
            self.RunNaplesCmd("sync && sync && sync")
            self.RunNaplesCmd("umount /data/")

        self.RunNaplesCmd("/nic/tools/fwupdate -l")
        return

    def InitForUpgrade(self):
        pass

    def InitForReboot(self):
        pass

class EsxHostManagement(HostManagement):
    def __init__(self, ipaddr):
        HostManagement.__init__(self, ipaddr)
        ssh=paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh.connect(GlobalOptions.esx_bld_vm,  '22', GlobalOptions.esx_bld_vm_username,GlobalOptions.esx_bld_vm_password)
        self.__bld_vm_ssh_handle = ssh
        self.__bld_vm_ssh_host = "%s@%s" % (GlobalOptions.esx_bld_vm_username, GlobalOptions.esx_bld_vm)
        self.__bld_vm_scp_pfx = "sshpass -p %s scp -o StrictHostKeyChecking=no " % GlobalOptions.esx_bld_vm_password
        self.__bld_vm_ssh_pfx = "sshpass -p %s ssh -o StrictHostKeyChecking=no " % GlobalOptions.esx_bld_vm_password
        return

    def ctrl_vm_copyin(self, src_filename, host_dir, naples_dir = None):
        dest_filename = host_dir + "/" + os.path.basename(src_filename)
        cmd = "%s %s %s:%s" % (self.__ctr_vm_scp_pfx, src_filename,
                               self.__ctr_vm_ssh_host, dest_filename)
        print(cmd)
        ret = os.system(cmd)
        assert(ret == 0)

        self.ctrl_vm_run("sync")
        ret = self.ctrl_vm_run("ls -l %s" % dest_filename)
        assert(ret == 0)

        if naples_dir:
            naples_dest_filename = naples_dir + "/" + os.path.basename(src_filename)
            ret = self.ctrl_vm_run("sshpass -p %s scp -o StrictHostKeyChecking=no %s root@%s:%s" %\
                           (GlobalOptions.password, dest_filename, GlobalOptions.mnic_ip, naples_dest_filename))
            assert(ret == 0)

        return 0

    def RunNaplesCmd(self, command, ignore_failure = False):
        assert(ignore_failure == True or ignore_failure == False)
        full_command = "sshpass -p %s ssh -o StrictHostKeyChecking=no root@%s %s" %\
                       (GlobalOptions.password, GlobalOptions.mnic_ip, command)
        return self.ctrl_vm_run(full_command, ignore_failure)

    def ctrl_vm_run(self, command, background = False, ignore_result = False):
        if background:
            cmd = "%s -f %s \"%s\"" % (self.__ctr_vm_ssh_pfx, self.__ctr_vm_ssh_host, command)
        else:
            cmd = "%s %s \"%s\"" % (self.__ctr_vm_ssh_pfx, self.__ctr_vm_ssh_host, command)
        print(cmd)
        retcode = os.system(cmd)
        if not ignore_result:
            assert(retcode == 0)
        return retcode

    def bld_vm_copyin(self, src_filename, dst_dir):
        dest_filename = dst_dir + "/" + os.path.basename(src_filename)
        cmd = "%s %s %s:%s" % (self.__bld_vm_scp_pfx, src_filename,
                               self.__bld_vm_ssh_host, dest_filename)
        print(cmd)
        ret = os.system(cmd)
        assert(ret == 0)

        self.bld_vm_run("sync")
        ret = self.bld_vm_run("ls -l %s" % dest_filename)
        assert(ret == 0)
        return 0

    def bld_vm_copyout(self, src_filename, dst_dir):
        dest_filename = dst_dir + "/" + os.path.basename(src_filename)
        cmd = "%s %s:%s %s" % (self.__bld_vm_scp_pfx, self.__bld_vm_ssh_host, src_filename,
                            dst_dir)
        print(cmd)
        ret = os.system(cmd)
        assert(ret == 0)
        return 0

    def bld_vm_run(self, command, background = False, ignore_result = False):
        if background:
            cmd = "%s -f %s \"%s\"" % (self.__bld_vm_ssh_pfx, self.__bld_vm_ssh_host, command)
        else:
            cmd = "%s %s \"%s\"" % (self.__bld_vm_ssh_pfx, self.__bld_vm_ssh_host, command)
        print(cmd)
        retcode = os.system(cmd)
        if not ignore_result:
            assert(retcode == 0)
        return retcode

    def __check_naples_deivce(self):
        assert(self.RunSshCmd("lspci | grep Pensando") == 0)

    def __esx_host_init(self):
        outFile = "/tmp/esx_" +  GlobalOptions.host_ip + ".json"
        self.WaitForSsh(port=443)
        time.sleep(30)
        esx_startup_cmd = ["timeout", "1200"]
        esx_startup_cmd.extend([GlobalOptions.esx_script])
        esx_startup_cmd.extend(["--esx-host", GlobalOptions.host_ip])
        esx_startup_cmd.extend(["--esx-username", GlobalOptions.host_username])
        esx_startup_cmd.extend(["--esx-password", GlobalOptions.host_password])
        esx_startup_cmd.extend(["--esx-outfile", outFile])
        proc_hdl = subprocess.Popen(esx_startup_cmd)
        while proc_hdl.poll() is None:
            time.sleep(5)
            continue
        assert(proc_hdl.returncode == 0)
        with open(outFile) as f:
            data = json.load(f)
            self.__esx_ctrl_vm_ip = data["ctrlVMIP"]
            self.__esx_ctrl_vm_username = data["ctrlVMUsername"]
            self.__esx_ctrl_vm_password = data["ctrlVMPassword"]

    def Init(self, driver_pkg = None, cleanup = True):
        self.WaitForSsh()
        os.system("date")
        self.__check_naples_deivce()
        self.__esx_host_init()
        self.__ctr_vm_ssh_host = "%s@%s" % (self.__esx_ctrl_vm_username, self.__esx_ctrl_vm_ip)
        self.__ctr_vm_scp_pfx = "sshpass -p %s scp -o StrictHostKeyChecking=no " % self.__esx_ctrl_vm_password
        self.__ctr_vm_ssh_pfx = "sshpass -p %s ssh -o StrictHostKeyChecking=no " % self.__esx_ctrl_vm_password

    def __install_drivers(self, pkg):
        pkg = self.__build_drivers(pkg)
        # Install IONIC driver package.
        #ESX removes folder after reboot, add it again
        self.RunSshCmd("rm -rf %s" % HOST_NAPLES_DIR)
        self.RunSshCmd("mkdir -p %s" % HOST_NAPLES_DIR)

        self.CopyIN(pkg, HOST_NAPLES_DIR)
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
        assert(install_success)
        #After installing drivers, no need to do esx startup as we are done installing firmware and driver
        #Skip naples check as reboot is not done
        #self.__check_naples_deivce()
        #Sleep for some time to get all other services up
        #time.sleep(30)

    def InstallMainFirmware(self, mount_data = True, copy_fw = True):
        if mount_data:
            self.RunNaplesCmd("/nic/tools/fwupdate -r | grep goldfw")
            self.RunNaplesCmd("mkdir -p /data && sync")
            self.RunNaplesCmd("mount /dev/mmcblk0p10 /data/")

        #Ctrl VM reboot might have removed the image
        self.ctrl_vm_copyin(GlobalOptions.image,
                    host_dir = HOST_ESX_NAPLES_IMAGES_DIR,
                    naples_dir = "/tmp")

        self.RunNaplesCmd("/nic/tools/sysupdate.sh -p /tmp/naples_fw.tar")
        if mount_data:
            self.RunNaplesCmd("sync && sync && sync")
            self.RunNaplesCmd("umount /data/")

        self.RunNaplesCmd("/nic/tools/fwupdate -l")
        return

    def InitForUpgrade(self):
        self.__install_drivers(GlobalOptions.gold_drivers_pkg)

    def InitForReboot(self):
        self.__install_drivers(GlobalOptions.drivers_pkg)

    def __host_connect(self):
        ip=GlobalOptions.host_ip
        port='22'
        username='root'
        password=GlobalOptions.host_password
        ssh=paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh.connect(ip,port,username,password)
        self.__ssh_handle = ssh


    def __build_drivers(self, pkg):
        #First copy the driver to tmp location
        tmp_driver = "/tmp/" + os.path.basename(pkg) + "_" + GlobalOptions.host_ip
        cp_driver_cmd = ["cp", pkg, tmp_driver]
        proc_hdl = subprocess.Popen(cp_driver_cmd)
        proc_hdl.wait()
        assert(proc_hdl.returncode == 0)
        stdin, stdout, stderr  = self.__bld_vm_ssh_handle.exec_command("find  /opt/vmware/  -type d  -name   nativeddk-6.5*")
        exit_status = stdout.channel.recv_exit_status()
        outlines=stdout.readlines()
        if exit_status != 0 or len(outlines) != 1:
            print ("Invalid output when discovering native ddk", outlines)
            sys.exit(1)
        dst_dir = outlines[0].strip("\n") + "/src/" + os.path.basename(tmp_driver) + "_dir"

        self.__bld_vm_ssh_handle.exec_command("rm -rf " + dst_dir + " && mkdir -p " + dst_dir + " && sync ")
        # Copy the driver package
        self.bld_vm_copyin(tmp_driver, dst_dir = dst_dir)
        stdin, stdout, stderr  = self.__bld_vm_ssh_handle.exec_command("cd " + dst_dir + " && tar -xvf " + os.path.basename(tmp_driver))
        exit_status = stdout.channel.recv_exit_status()
        outlines=stdout.readlines()
        if exit_status != 0:
            print ("Failed to extract drivers", outlines)
            sys.exit(1)

        stdin, stdout, stderr  = self.__bld_vm_ssh_handle.exec_command("cd " + dst_dir + "/drivers-esx-eth && ./build.sh" )
        exit_status = stdout.channel.recv_exit_status()
        outlines=stdout.readlines()
        if exit_status != 0:
            print ("Driver build failed ", ''.join(outlines))
            sys.exit(1)

        stdin, stdout, stderr  = self.__bld_vm_ssh_handle.exec_command("cd " + dst_dir + " && tar -cJf " + os.path.basename(tmp_driver) + " drivers-esx-eth")
        exit_status = stdout.channel.recv_exit_status()
        outlines=stdout.readlines()
        if exit_status != 0:
            print ("Failed to create tar file ", outlines)
            sys.exit(1)

        dst_file = dst_dir + "/" + os.path.basename(tmp_driver)
        self.bld_vm_copyout(dst_file, dst_dir = os.path.dirname(tmp_driver))
        return tmp_driver

    def Reboot(self, dryrun = False):
        os.system("date")
        self.RunSshCmd("sync")
        self.RunSshCmd("uptime")
        if dryrun == False:
            self.RunSshCmd("reboot", ignore_failure = True)
        time.sleep(60)
        print("Rebooting Host : %s" % GlobalOptions.host_ip)
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

    # Reset the setup:
    # If the previous run left it in bad state, we may not get ssh or console.
    if GlobalOptions.only_mode_change == False and GlobalOptions.only_init == False:
        #First do a reset as naples may be in screwed up state.
        IpmiReset()
        time.sleep(10)
        naples.Connect()
        naples.InitForUpgrade(goldfw = True)
        #Do a reset again as old fw might lock up host boot
        IpmiReset()

    global host
    host = HostManagement(GlobalOptions.host_ip)
    if GlobalOptions.os == 'esx':
        host = EsxHostManagement(GlobalOptions.host_ip)
    else:
        host = HostManagement(GlobalOptions.host_ip)

    host.WaitForSsh()


    if GlobalOptions.only_init == True:
        # Case 3: Only INIT option. Unload drivers, Install drivers, Configure Int Mgmt IP,  and return.
        host.Init(driver_pkg = GlobalOptions.drivers_pkg, cleanup = True)
        return

    # Connect to Naples console.
    naples.Connect()

    # Check if we need to switch to GoldFw or not.
    if GlobalOptions.only_mode_change:
        # Case 2: Only change mode, reboot and install drivers
        naples.InitForUpgrade(goldfw = False)
        naples.Close()
    else:
        # Case 1: Main firmware upgrade.
        naples.InitForUpgrade(goldfw = True)
        host.InitForUpgrade()
        host.Reboot()
        naples.Close()
        host.Init(driver_pkg =  GlobalOptions.gold_drivers_pkg, cleanup = False)
        host.InstallMainFirmware()

    #Script that might have to run just before reboot
    # ESX would require drivers to be installed here to avoid
    # onr more reboot
    host.InitForReboot()
    #Reboot is common for both mode change and upgrade
    # Reboot host again, this will reboot naples also
    host.Reboot()
    # Common to Case 2 and Case 1.
    # Initialize the Node, this is needed in all cases.
    host.Init(driver_pkg = GlobalOptions.drivers_pkg, cleanup = False)

    # Update MainFwB also to same image - TEMP CHANGE
    host.InstallMainFirmware(mount_data = False, copy_fw = False)
    return

if __name__ == '__main__':
    atexit.register(AtExitCleanup)
    Main()
