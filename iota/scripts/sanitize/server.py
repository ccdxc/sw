import time
import pexpect
import os
import socket
from datetime import datetime
import sys
import helpers
from helpers import *

class Server(object):
    def __init__(self, devtype, ipaddr, cimc = None):
        self.ipaddr   = ipaddr
        self.devtype  = devtype
        self.cimc     = cimc
        self.username = ServerHelper.GetUserNameByType(self.devtype)
        self.password = ServerHelper.GetPasswordByType(self.devtype)
        self.cimc_username = CimcHelper.GetCimcUsername()
        self.cimc_password = CimcHelper.GetCimcPassword()
        self.ssh_host = "%s@%s" % (self.username, self.ipaddr)
        self.scp_pfx = "sshpass -p %s scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % self.password
        self.ssh_pfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % self.password
        return 

    @property
    def __log_pfx(self):
        return "[%s][%s]" % (str(datetime.now()), self.ipaddr)

    def __str__(self):
        return "Host:[%s]-CIMC[%s]" % (self.ipaddr, self.cimc)
    
    def GetHostSshHandle(self):
        prompt_list =[ '[Pp]assword:', '#', pexpect.TIMEOUT]
        host  = self.Spawn("ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no %s@%s" % (self.username, self.ipaddr))
        done=False
        while not done:
            i = host.expect(prompt_list,timeout=10)
            if i==0:
                host.sendline(self.password)
            elif i==1:
                done=True
            elif i==3:
                msg  = ('%s: Did not receive any expected prompts' % self.__log_pfx)
                print (msg)
                raise Exception(msg)
        return host

    def GetCimcSshHandle(self):
        prompt_list =[ '[Pp]assword: ?$', '# ?$', 'yes\/no\)\? $', pexpect.TIMEOUT]
        cimc  = self.Spawn("ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no %s@%s" % (self.cimc_username, self.cimc))
        time.sleep(2)
        done=False
        while not done:
            i = cimc.expect(prompt_list,timeout=10)
            if i==0:
                cimc.sendline(self.cimc_password)
            elif i==1:
                done=True
            elif i==2:
                cimc.sendline('yes')
            elif i==3:
                print('%s: Did not receive any expected prompts' % self.__log_pfx)
                cimc.terminate(force=True)
                break
        return cimc 

    def GetHandle(self):
        return self.__get_host_ssh_handle()

    def Spawn(self, command):
        hdl = pexpect.spawn(command)
        hdl.timeout = helpers.TIMEOUT
        hdl.logfile = sys.stdout
        return hdl

    def WaitForSsh(self, port = 22):
        print("%s: Waiting for IP:%s to be up." % (self.__log_pfx, self.ipaddr))
        for retry in range(150):
            if self.IsSSHUP():
                return
            time.sleep(5)
        print("%s: Host not up" % self.__log_pfx)
        raise Exception("Host : {} did not up".format(self.ipaddr))

    def IsSSHUP(self, port = 22):
        print("%s: Waiting for IP:%s to be up." % (self.__log_pfx, self.ipaddr))
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        ret = sock.connect_ex(('%s' % self.ipaddr, port))
        sock.settimeout(1)
        if ret == 0:
            return True
        print("%s: Host not up. Ret:%d" % (self.__log_pfx, ret))
        return False

    def RunSshCmd(self, command):
        full_command = "%s %s \"%s\"" % (self.ssh_pfx, self.ssh_host, command)
        print("%s: %s" % (self.__log_pfx, full_command))
        retcode = os.system(full_command)
        print ("%s retcode[%s]" % (self.__log_pfx, str(retcode)))
        if retcode%128 != 0:
            print("%s: ERROR: Failed to run command: %s" % (self.__log_pfx, command))
            raise Exception(full_command)
        return retcode

    def CopyIN(self, src_filename, entity_dir):
        dest_filename = entity_dir + "/" + os.path.basename(src_filename)
        cmd = "%s %s %s:%s" % (self.scp_pfx, src_filename, self.ssh_host, dest_filename)
        print("%s: %s" % (self.__log_pfx, cmd))
        ret = os.system(cmd)
        if ret:
            raise Exception("Enitity : {}, src : {}, dst {} ".format(self.ipaddr, src_filename, dest_filename))
        self.RunSshCmd("sync")
        ret = self.RunSshCmd("ls -l %s" % dest_filename)
        if ret:
            raise Exception("Enitity : {}, src : {}, dst {} ".format(self.ipaddr, src_filename, dest_filename))

    def IsPingable(self):
        ping = 'ping -c 5 ' + self.ipaddr + ' > /dev/null 2>&1'
        stderr = os.system(ping)
        if stderr != 0:
            return False
        return True


