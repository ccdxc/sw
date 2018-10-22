#! /usr/bin/python3
import argparse
import pexpect
import sys
import os
import time
import socket
import pdb
import requests

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
                    default=5, help='Naples Password.')
parser.add_argument('--image', dest='image',
                    default='/sw/nic/naples_fw.tar', help='Naples Image.')
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
        self.hdl.sendline(GlobalOptions.username)
        self.hdl.expect("Password:")
        self.hdl.sendline(GlobalOptions.password)
        self.hdl.expect("#")
        return

    def connect(self):
        self.hdl = pexpect.spawn("telnet %s %s" %\
                                 (GlobalOptions.console_ip, 
                                  GlobalOptions.console_port))
        self.hdl.timeout = GlobalOptions.timeout
        self.hdl.logfile = sys.stdout.buffer
        self.hdl.expect_exact("Escape character is '^]'.")
        self.hdl.sendline("")
        return
    
    def __reset(self):
        self.hdl.sendline("/nic/tools/sysreset.sh")
        return

    def __cimc_reset_restapi(self):
        session = requests.Session()
        session.auth = (GlobalOptions.cimc_username, GlobalOptions.cimc_password)
        resp = session.get("https://%s/redfish/v1/Systems" % GlobalOptions.cimc_ip, verify=False)
        obj = resp.json()
        sysurl = "https://%s%s/Actions/System.Reset" % (GlobalOptions.cimc_ip, obj['Members'][0]['@odata.id'])
        session.post(sysurl, '{"ResetType":"ForceOff"}', verify=False)
        time.sleep(10)
        session.post(sysurl, '{"ResetType":"On"}', verify=False)
        return

    def __get_capri_prompt(self):
        match_idx = self.hdl.expect_exact(["capri login:", "#", "Capri#"], timeout = 120)
        if match_idx == 2:
            return
        if match_idx == 0:
            self.__login()
        self.__reset()
        match_idx = self.hdl.expect([ "Autoboot in 2 seconds; Press CTRL-B to stop",
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

    def clearline(self):
        hdl = pexpect.spawn("telnet %s" % GlobalOptions.console_ip)
        hdl.logfile = sys.stdout.buffer
        hdl.timeout = GlobalOptions.timeout
        hdl.expect("Username:")
        hdl.sendline(GlobalOptions.console_username)
        hdl.expect("Password:")
        hdl.sendline(GlobalOptions.console_password)
        hdl.expect("ts#")
        for i in range(6):
            hdl.sendline("clear line %d" % (GlobalOptions.console_port - 2000))
            hdl.expect_exact("[confirm]")
            hdl.sendline("")
            hdl.expect_exact(" [OK]")
        hdl.close()
        return

    def install_fw(self):
        self.hdl.sendline("/nic/tools/sysupdate.sh -p /tmp/naples_fw.tar")
        self.hdl.expect_exact("Removing package directory /tmp/sysupdate", timeout = 600)
        self.__reset()
        self.hdl.expect_exact("capri login:", timeout = 120)
        self.__login()
        self.hdl.sendline("/nic/tools/sysinit.sh %s hw" % GlobalOptions.mode)
        self.hdl.expect_exact("All processes brought up, please check ...", timeout = 600)
        return


class HostManagement:
    def __init__(self):
        self.__ssh_host = "%s@%s" % (GlobalOptions.host_username, GlobalOptions.host_ip)
        self.__scp_pfx = "sshpass -p %s scp -o StrictHostKeyChecking=no " % GlobalOptions.host_password
        self.__ssh_pfx = "sshpass -p %s ssh -o StrictHostKeyChecking=no " % GlobalOptions.host_password
        self.__wait_for_ssh()
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

    def copyin(self, src_filename, dest_dir, to_naples = False):
        dest_filename = dest_dir + "/" + os.path.basename(src_filename)
        cmd = "%s %s %s:%s" % (self.__scp_pfx, src_filename,
                               self.__ssh_host, dest_filename)
        print(cmd)
        ret = os.system(cmd)

        self.run("sync")
        self.run("ls -l %s" % dest_filename)
        if to_naples:
            return self.run("sshpass -p %s scp -o StrictHostKeyChecking=no %s root@1.0.0.2:%s" %\
                            (GlobalOptions.password, dest_filename, dest_filename))
        return 0

    def run(self, command, background = False):
        if background:
            cmd = "%s -f %s \"%s\"" % (self.__ssh_pfx, self.__ssh_host, command)
        else:
            cmd = "%s %s \"%s\"" % (self.__ssh_pfx, self.__ssh_host, command)
        print(cmd)
        retcode = os.system(cmd)
        if retcode != 0:
            print("Command Failed: %s" % command)
        return retcode

    def reboot(self):
        self.run("sync")
        self.run("reboot")
        # Wait for the host to start rebooting.
        time.sleep(30)
        self.__wait_for_ssh()
        return

    def init(self):
        if self.run("lspci | grep 1dd8") != 0:
            sys.exit(1)

        self.run("pkill memtun")

        self.run("/root/memtun 1.0.0.1", background = True)
        if self.run("ping -c 10 1.0.0.2") != 0:
            sys.exit(1)

        self.run("rm -f /root/.ssh/known_hosts")
        self.run("apt install -y sshpass")
        self.run("insmod /root/ionic.ko")
        return

def Main():
    nap = NaplesManagement()
    nap.clearline()
    nap.connect()
    nap.boot_altfw()

    host = HostManagement()
    host.reboot()
    host.init()
    host.copyin(GlobalOptions.image, "/tmp", to_naples = True)

    nap.install_fw()

    host.reboot()
    host.init()
    return

if __name__ == '__main__':
    Main()
