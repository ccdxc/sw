import defines
import time
import pexpect
import os
import socket
from datetime import datetime
import sys

TIMEOUT = 60

class Server(object):
    def __init__(self, devtype, ipaddr, cimc = None):
        self.ipaddr   = ipaddr
        self.devtype  = devtype
        self.cimc     = cimc
        self.username = defines.GetUserNameByType(self.devtype)
        self.password = defines.GetPasswordByType(self.devtype)
        self.cimc_username = defines.GetCimcUsername()
        self.cimc_password = defines.GetCimcPassword()
        self.ssh_host = "%s@%s" % (self.username, self.ipaddr)
        self.scp_pfx = "sshpass -p %s scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % self.password
        self.ssh_pfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % self.password
        return 

    @property
    def __log_pfx(self):
        return "[%s][%s]" % (str(datetime.now()), self.ipaddr)

    def __str__(self):
        return "Host:[%s]-CIMC[%s]" % (self.ipaddr, self.cimc)

    def SetUserPassword(self, username, password):
        hdl = self.__get_host_ssh_handle()
        self.SendlineExpect("sudo passwd %s" % username, "password:",  hdl)
        self.SendlineExpect("%s" % password, "password:" , hdl)
        self.SendlineExpect("%s" % password, "#",  hdl)
        return
    
    def SendlineExpect(self, line, expect, hdl = None, timeout = 60):
        if hdl is None: hdl = self.hdl
        hdl.sendline(line)
        return hdl.expect_exact(expect, timeout)

    def Spawn(self, command):
        hdl = pexpect.spawn(command)
        hdl.timeout = TIMEOUT
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

    def RunSshCmd(self, command, ignore_failure = False):
        date_command = "%s %s \"date\"" % (self.ssh_pfx, self.ssh_host)
        os.system(date_command)
        full_command = "%s %s \"%s\"" % (self.ssh_pfx, self.ssh_host, command)
        print("%s: %s" % (self.__log_pfx, full_command))
        retcode = os.system(full_command)
        print ("%s retcode[%s]" % (self.__log_pfx, str(retcode)))
        if ignore_failure is False and retcode != 0:
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

    def __get_cimc_ssh_handle(self):
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

    def GetOobMacFromCimc(self):
        cimc = self.__get_cimc_ssh_handle()
        prompt = "#"
        self.SendlineExpect("scope chassis", prompt,  cimc)
        self.SendlineExpect("scope network-adapter L", prompt,  cimc)
        self.SendlineExpect("show mac-list", prompt,  cimc)
        op = cimc.before
        print ("%s: %s" % (self.__log_pfx, op))
        return defines.GetMacList(op)

    def SetBootOrder(self, boot_order):
        cimc = self.__get_cimc_ssh_handle()
        prompt = "#"
        self.SendlineExpect("scope bios", prompt,  cimc)
        self.SendlineExpect("set boot-order %s" % boot_order, prompt,  cimc)
        self.SendlineExpect("commit", "y|N]",  cimc)
        self.SendlineExpect("y", prompt,  cimc)
        return

    def RunIpmi(self, command):
        cmd = "ipmitool -I lanplus -H %s -U %s -P %s %s" % (self.cimc, self.cimc_username, self.cimc_password, command)
        print ("%s: %s" % (self.__log_pfx,command))
        return  os.system(cmd)

    def WaitForReachability(self, msg):
        i = 0 
        while i < 100:
            print ("%s: %s" % (self.__log_pfx,msg))
            if self.IsPingable():
                break
            time.sleep(5)
            i += 1
        if i == 100:
            raise Exception("Node not booting after [%s] seconds -- Debug" % str(100*5))
        return True
        
    def WaitForReboot(self, msg):
        i = 0 
        while i < 100:
            print ("%s: %s" % (self.__log_pfx, msg))
            if not self.IsPingable():
                break
            time.sleep(5)
            i += 1
        if i == 100:
            raise Exception("Node not booting after [%s] seconds -- Debug" % str(100*5))
        return True
        
    def SetHostName(self):
        cmd = "hostname %s" % self.ipaddr
        self.RunSshCmd(cmd)

    def __get_host_ssh_handle(self):
        prompt_list =[ '[Pp]assword: ?$', '#', 'yes\/no\)\? $', pexpect.TIMEOUT]
        host  = self.Spawn("ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no %s@%s" % (self.username, self.ipaddr))
        time.sleep(2)
        done=False
        while not done:
            i = host.expect(prompt_list,timeout=10)
            if i==0:
                host.sendline(self.password)
            elif i==1:
                done=True
            elif i==2:
                host.sendline('yes')
            elif i==3:
                print('%s: Did not receive any expected prompts' % self.__log_pfx)
                host.terminate(force=True)
                break
        return host

    def ShowVersion(self):
        """
        This has to be per OS type
        """
        cmd = "hostnamectl"
        host = self.__get_host_ssh_handle()
        prompt = "#"
        self.SendlineExpect(cmd, prompt, host)
        op = host.before
        print ("%s: %s" % (self.__log_pfx, op))
        return


