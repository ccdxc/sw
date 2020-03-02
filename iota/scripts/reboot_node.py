#! /usr/bin/python3
import argparse
import pexpect
import sys
import os
import time
import socket
import pdb
import requests

parser = argparse.ArgumentParser(description='Node Reboot Script')
# Mandatory parameters
parser.add_argument('--host-username', dest='host_username',
                    default="root", help='Host Username')
parser.add_argument('--host-password', dest='host_password',
                    default="docker", help='Host Password.')
parser.add_argument('--host-ip', dest='host_ip', required = True,
                    default=None, help='Host IP Address.')
parser.add_argument('--cimc-ip', dest='cimc_ip', required = True,
                    default=None, help='CIMC IP Address.')
parser.add_argument('--cimc-username', dest='cimc_username',
                    default="admin", help='CIMC Username')
parser.add_argument('--cimc-password', dest='cimc_password',
                    default="N0isystem$", help='CIMC Password.')
parser.add_argument('--os', dest='os',
                    default="", help='Node OS (Freebsd or Linux).')
parser.add_argument('--timeout', dest='timeout',
                    default=60, help='Naples Password.')
GlobalOptions = parser.parse_args()

USER_EXP_PROMPT="$"
if GlobalOptions.os == 'freebsd':
    USER_EXP_PROMPT="~]$"

if GlobalOptions.os == 'esx':
    USER_EXP_PROMPT="~]"



def IpmiReset():
    os.system("ipmitool -I lanplus -H %s -U %s -P %s power cycle" %\
              (GlobalOptions.cimc_ip, GlobalOptions.cimc_username, GlobalOptions.cimc_password))
    return

class HostManagement:
    def __init__(self):
        self.__ssh_host = "%s@%s" % (GlobalOptions.host_username, GlobalOptions.host_ip)
        self.__scp_pfx = "sshpass -p %s scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % GlobalOptions.host_password
        self.__ssh_pfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % GlobalOptions.host_password
        self._wait_for_ssh()
        self._connect()
        return

    def _connect(self):
        self.hdl = pexpect.spawn("ssh  -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no %s" % self.__ssh_host)
        self.hdl.timeout = GlobalOptions.timeout
        self.hdl.logfile = sys.stdout.buffer
        self.hdl.expect(["[Pp]assword", "Password for root@"])
        self.hdl.sendline(GlobalOptions.host_password)
        self.hdl.expect_exact(USER_EXP_PROMPT)
        self.hdl.sendline("uptime")
        self.hdl.expect_exact(USER_EXP_PROMPT)
        return


    def _wait_for_ssh(self):
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
        self.hdl.expect_exact(USER_EXP_PROMPT)
        self.hdl.sendline("sudo ls -l /root/")
        self.hdl.expect_exact(USER_EXP_PROMPT)
        self.hdl.sendline("uptime")
        self.hdl.expect_exact(USER_EXP_PROMPT)
        self.hdl.sendline("sudo rm -rf /pensando")
        self.hdl.expect_exact(USER_EXP_PROMPT)
        self.hdl.sendline("sudo mkdir /pensando")
        self.hdl.expect_exact(USER_EXP_PROMPT)
        self.hdl.sendline("sudo chown vm:vm /pensando")
        self.hdl.expect_exact(USER_EXP_PROMPT)

        self.hdl.sendline("sudo reboot && sleep 30")
        match = self.hdl.expect_exact([USER_EXP_PROMPT, pexpect.TIMEOUT, pexpect.EOF], timeout=10)
        self.hdl.close()

        # Wait for the host to start rebooting.
        time.sleep(30)
        self._wait_for_ssh()
        self._connect()
        return

class EsxHostManagement(HostManagement):
    def __init__(self):
        HostManagement.__init__(self)
        return


    def reboot(self):
        self.hdl.sendline("sync")
        self.hdl.expect_exact(USER_EXP_PROMPT)

        self.hdl.sendline("sudo reboot && sleep 30")
        match = self.hdl.expect_exact([USER_EXP_PROMPT, pexpect.TIMEOUT, pexpect.EOF], timeout=10)
        self.hdl.close()

        # Wait for the host to start rebooting.
        time.sleep(30)
        self._wait_for_ssh()
        self._connect()
        return

def Main():
    host = None
    if GlobalOptions.os == 'esx':
        host = EsxHostManagement()
    else:
        host = HostManagement()

    host.reboot()
    return

if __name__ == '__main__':
    Main()
