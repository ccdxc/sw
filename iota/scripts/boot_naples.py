#! /usr/bin/python3
import argparse
import pexpect
import sys
import os
import time
import socket
import pdb
import requests

HOST_NAPLES_DIR         = "/naples"
HOST_NAPLES_DRIVERS_DIR = "%s/drivers" % HOST_NAPLES_DIR
HOST_NAPLES_IMAGES_DIR  = "%s/images" % HOST_NAPLES_DIR

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
                    default='/sw/nic/naples_fw.tar', help='Naples Image.')
parser.add_argument('--drivers-ionic-pkg', dest='drivers_ionic_pkg',
                    default=None, help='IONIC Driver Package.')
parser.add_argument('--drivers-sonic-pkg', dest='drivers_sonic_pkg',
                    default=None, help='IONIC Driver Package.')
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

GlobalOptions = parser.parse_args()

GlobalOptions.console_port = int(GlobalOptions.console_port)
GlobalOptions.timeout = int(GlobalOptions.timeout)

class NaplesManagement:
    def __init__(self):
        return

    def __login(self):
        time.sleep(3)
        self.hdl.sendline(GlobalOptions.username)
        self.hdl.expect("Password:")
        self.hdl.sendline(GlobalOptions.password)
        ret = self.hdl.expect(["#", pexpect.TIMEOUT], timeout = 3)
        if ret == 1:
            self.hdl.sendline("")
            self.hdl.expect("#")
        return

    def __clearline(self):
        print("Clearing Console Server Line")
        hdl = pexpect.spawn("telnet %s" % GlobalOptions.console_ip)
        hdl.logfile = sys.stdout.buffer
        hdl.timeout = GlobalOptions.timeout
        idx = hdl.expect(["Username:", "Password:"])
        if idx == 0:
            hdl.sendline(GlobalOptions.console_username)
            hdl.expect("Password:")
            hdl.sendline(GlobalOptions.console_password)
        if idx == 1:
            hdl.sendline(GlobalOptions.console_password)
        hdl.expect("#")
        for i in range(6):
            hdl.sendline("clear line %d" % (GlobalOptions.console_port - 2000))
            hdl.expect_exact("[confirm]")
            hdl.sendline("")
            hdl.expect_exact(" [OK]")
        hdl.close()
        return


    def connect(self):
        while True:
            self.hdl = pexpect.spawn("telnet %s %s" %\
                                     (GlobalOptions.console_ip, 
                                      GlobalOptions.console_port))
            self.hdl.timeout = GlobalOptions.timeout
            self.hdl.logfile = sys.stdout.buffer
            idx = self.hdl.expect_exact(["Escape character is '^]'.",
                                         "Connection refused"])
            if idx == 0:
                break
            else:
                self.__clearline()
        self.hdl.sendline("")
        return
   
    def __umount_mnt(self):
        self.hdl.sendline("rm -rf /mnt/*.json")
        self.hdl.expect_exact("#")
        self.hdl.sendline("sync")
        self.hdl.expect_exact("#")
        self.hdl.sendline("sync")
        self.hdl.expect_exact("#")
        self.hdl.sendline("/etc/init.d/S22mount stop")
        self.hdl.expect_exact("#")
        return

    def __reset(self):
        self.hdl.sendline("/nic/tools/sysreset.sh")
        return

    def __cimc_reset_restapi(self):
        session = requests.Session()
        session.auth = (GlobalOptions.cimc_username, GlobalOptions.cimc_password)
        resp = session.get("https://%s/redfish/v1/Systems" % GlobalOptions.cimc_ip, verify=False)
        obj = resp.json()
        print("Login Response =", obj)
        sysurl = "https://%s%s/Actions/System.Reset" % (GlobalOptions.cimc_ip, obj['Members'][0]['@odata.id'])
        print("SysURL = ", sysurl)

        ret = os.system("curl -vv %s -d \'{\"ResetType\":\"On\"}\' --insecure -u %s:%s" % (sysurl, GlobalOptions.cimc_username, GlobalOptions.cimc_password))
        assert(ret == 0)
        time.sleep(5)

        ret = os.system("curl -vv %s -d \'{\"ResetType\":\"ForceOff\"}\' --insecure -u %s:%s" % (sysurl, GlobalOptions.cimc_username, GlobalOptions.cimc_password))
        assert(ret == 0)
        time.sleep(5)

        ret = os.system("curl -vv %s -d \'{\"ResetType\":\"On\"}\' --insecure -u %s:%s" % (sysurl, GlobalOptions.cimc_username, GlobalOptions.cimc_password))
        assert(ret == 0)
        time.sleep(5)
        return

    def __get_capri_prompt(self):
        match_idx = self.hdl.expect_exact(["capri login:", "#", "Capri#"], timeout = 120)
        if match_idx == 2:
            return
        if match_idx == 0:
            self.__login()
        #self.__reset()
        self.__umount_mnt()
        self.__cimc_reset_restapi()
        match_idx = self.hdl.expect(["Autoboot in 2 seconds; Press CTRL-B to stop",
                                      pexpect.TIMEOUT], timeout = 120)
        if match_idx == 1:
            print("WARN: sysreset.sh script did not reset the system. Trying CIMC")
            self.__cimc_reset_restapi()
            self.hdl.expect_exact("Autoboot in 2 seconds; Press CTRL-B to stop", timeout = 120)
        self.hdl.sendcontrol('B')
        self.hdl.expect_exact("Capri#")
        return

    def boot_altfw(self):
        self.__get_capri_prompt()
        self.hdl.sendline("bootalt")
        self.hdl.expect_exact("capri login:", timeout = 120)
        self.__login()
        return

    def install_fw(self):
        self.hdl.sendline("/nic/tools/sysupdate.sh -p /tmp/naples_fw.tar")
        self.hdl.expect_exact("Removing package directory /tmp/sysupdate", timeout = 600)
        self.hdl.sendline("echo %s > /mnt/app-start.conf && sync" % GlobalOptions.mode)
        #self.hdl.sendline("echo off > /mnt/app-start.conf && sync")
        self.hdl.expect_exact("#")
        self.__umount_mnt()
        idx = 0
        while idx == 0:
            self.__reset()
            idx = self.hdl.expect_exact(["#", "capri login:"], timeout = 120)
        self.__login()
        #self.hdl.sendline("/nic/tools/sysinit.sh %s hw" % GlobalOptions.mode)
        #self.hdl.expect_exact("All processes brought up, please check ...", timeout = 600)
        return


class HostManagement:
    def __init__(self):
        self.__ssh_host = "%s@%s" % (GlobalOptions.host_username, GlobalOptions.host_ip)
        self.__scp_pfx = "sshpass -p %s scp -o StrictHostKeyChecking=no " % GlobalOptions.host_password
        self.__ssh_pfx = "sshpass -p %s ssh -o StrictHostKeyChecking=no " % GlobalOptions.host_password
        self.__wait_for_ssh()
        self.__connect()
        return

    def __connect(self):
        self.hdl = pexpect.spawn("ssh -o StrictHostKeyChecking=no %s" % self.__ssh_host)
        self.hdl.timeout = GlobalOptions.timeout
        self.hdl.logfile = sys.stdout.buffer
        self.hdl.expect_exact("Password:")
        self.hdl.sendline(GlobalOptions.host_password)
        self.hdl.expect_exact("#")
        return
 

    def __wait_for_ssh(self):
        print("Waiting for host to be up.")
        for retry in range(60):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            ret = sock.connect_ex(('%s' % GlobalOptions.host_ip, 22))
            sock.settimeout(1)
            if ret == 0: 
                return
            else:
                time.sleep(5)

        print("Host not up. Ret:%d" % ret)
        sys.exit(1)
        return

    def copyin(self, src_filename, host_dir, naples_dir = None):
        dest_filename = host_dir + "/" + os.path.basename(src_filename)
        cmd = "%s %s %s:%s" % (self.__scp_pfx, src_filename,
                               self.__ssh_host, dest_filename)
        print(cmd)
        ret = os.system(cmd)
        assert(ret == 0)

        self.run("sync")
        ret = self.run("ls -l %s" % dest_filename)
        assert(ret == 0)

        if naples_dir:
            naples_dest_filename = naples_dir + "/" + os.path.basename(src_filename)
            ret = self.run("sshpass -p %s scp -o StrictHostKeyChecking=no %s root@1.0.0.2:%s" %\
                           (GlobalOptions.password, dest_filename, naples_dest_filename))
            assert(ret == 0)

        return 0

    def run(self, command, background = False, ignore_result = False):
        if background:
            cmd = "%s -f %s \"%s\"" % (self.__ssh_pfx, self.__ssh_host, command)
        else:
            cmd = "%s %s \"%s\"" % (self.__ssh_pfx, self.__ssh_host, command)
        print(cmd)
        retcode = os.system(cmd)
        if not ignore_result: 
            assert(retcode == 0)
        return retcode

    def reboot(self):
        self.hdl.sendline("sync")
        self.hdl.expect_exact("#")
        self.hdl.sendline("ls -l /root/")
        self.hdl.expect_exact("#")
        self.hdl.sendline("reboot")
        self.hdl.expect_exact(["#",pexpect.TIMEOUT, pexpect.EOF], timeout=10)
        self.hdl.close()
        
        # Wait for the host to start rebooting.
        time.sleep(30)
        self.__wait_for_ssh()
        self.__connect()
        return

    def init(self):
        assert(self.run("lspci | grep 1dd8") == 0)
        self.hdl.sendline("%s/start_memtun.sh" % HOST_NAPLES_DIR)
        self.hdl.expect_exact("#")
        self.run("ping -c 5 1.0.0.2")
        self.run("rm -f /root/.ssh/known_hosts")
        return

    def install_drivers(self):
        if GlobalOptions.drivers_ionic_pkg:
            # Install IONIC driver package.
            self.copyin(GlobalOptions.drivers_ionic_pkg, HOST_NAPLES_DRIVERS_DIR)
            self.run("cd %s && tar xaf %s" %\
                     (HOST_NAPLES_DRIVERS_DIR, os.path.basename(GlobalOptions.drivers_ionic_pkg)))
            self.run("cd %s/drivers-linux/ && ./setup_apt.sh" % HOST_NAPLES_DRIVERS_DIR)
            self.run("cd %s/drivers-linux/ && ./build.sh" % HOST_NAPLES_DRIVERS_DIR)
            self.run("rmmod ionic", ignore_result = True)
            self.run("rmmod ionic_rdma", ignore_result = True)
            self.run("cd %s/drivers-linux/ && insmod drivers/eth/ionic/ionic.ko" % HOST_NAPLES_DRIVERS_DIR)
            self.run("modprobe ib_uverbs")
            self.run("cd %s/drivers-linux/ && insmod drivers/rdma/drv/ionic/ionic_rdma.ko xxx_haps=1" % HOST_NAPLES_DRIVERS_DIR)

        if GlobalOptions.drivers_sonic_pkg:
            # Install SONIC driver package.
            self.copyin(GlobalOptions.drivers_sonic_pkg, HOST_NAPLES_DRIVERS_DIR)
            self.run("cd %s && tar xaf %s" %\
                     (HOST_NAPLES_DRIVERS_DIR, os.path.basename(GlobalOptions.drivers_sonic_pkg)))
            self.run("cd %s/storage-offload/ && make modules" % HOST_NAPLES_DRIVERS_DIR)
            self.run("rmmod pencake", ignore_result = True)
            self.run("rmmod sonic", ignore_result = True)
            self.run("cd %s/storage-offload/ && insmod sonic.ko core_count=2" % HOST_NAPLES_DRIVERS_DIR)
            self.run("cd %s/storage-offload/ && insmod pencake.ko repeat=1" % HOST_NAPLES_DRIVERS_DIR)
        return

def Main():
    nap = NaplesManagement()
    
    nap.connect()
    nap.boot_altfw()
    
    host = HostManagement()
    host.run("rm -rf %s" % HOST_NAPLES_DIR)
    host.run("mkdir -p %s" % HOST_NAPLES_DRIVERS_DIR)
    host.run("mkdir -p %s" % HOST_NAPLES_IMAGES_DIR)
    host.copyin("/home/haps/memtun/memtun", HOST_NAPLES_DIR)
    host.copyin("scripts/linux/start_memtun.sh", HOST_NAPLES_DIR)
    host.reboot()
    host.init()

    # Copy the firmare
    host.copyin(GlobalOptions.image, 
                host_dir = HOST_NAPLES_IMAGES_DIR,
                naples_dir = "/tmp")
    
    # Install the firmware
    nap.install_fw()
    host.reboot()
    host.init()
    
    # Install the drivers.
    host.install_drivers()
    return

if __name__ == '__main__':
    Main()
