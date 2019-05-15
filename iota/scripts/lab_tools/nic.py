import pexpect
import sys
from console import Console
import os

TIMEOUT = 30
NAPLES_OOB_USERNAME = 'root'
NAPLES_OOB_PASSWORD = 'pen123'
NAPLES_CONSOLE_USERNAME = 'root'
NAPLES_CONSOLE_PASSWORD = 'pen123'

class Naples(object):
    def __init__(self, console, console_port):
        self.console_ip = console
        self.console_port = console_port
        self.console_username = NAPLES_CONSOLE_USERNAME
        self.console_password = NAPLES_CONSOLE_PASSWORD
        self.console = Console(self.console_ip, self.console_port)
        self.oob_username = NAPLES_OOB_USERNAME
        self.oob_password = NAPLES_OOB_PASSWORD
        #self.oob_ip = self.__init_oob()
        self.oob_ip = None
        self.ssh_host = "%s@%s" % (self.oob_username, self.oob_ip)
        self.scp_pfx = "sshpass -p %s scp -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % self.oob_password
        self.ssh_pfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % self.oob_password

    def __str__(self):
        return "[%s:%s]" % (self.console, str(self.console_port))

    @property
    def __log_pfx(self):
        return "[%s:%s]" % (self.console, str(self.console_port))

    def __spawn(self, command):
        hdl = pexpect.spawn(command)
        hdl.timeout = TIMEOUT
        hdl.logfile = sys.stdout
        return hdl

    def SendlineExpect(self, line, expect, hdl, timeout = TIMEOUT):
        hdl.sendline(line)
        return hdl.expect_exact(expect, timeout)

    def __is_oob_present(self):
        if self.oob_ip == None or self.oob_ip == []:
            return False
        return True

    def __is_oob_pingable(self):
        if not self.__is_oob_present():
            return False
        ping = 'ping -c 5 ' + self.oob_ip + ' > /dev/null 2>&1'
        stderr = os.system(ping)
        if stderr != 0:
            return False
        return True

    def __init_oob(self):
        self.console.EnableDhcpOnOob()
        self.oob_ip = self.console.GetOobIp()

    def __is_oob_up(self):
        return self.__is_oob_pingable()

    def GetOobIp(self):
        if self.__is_oob_present():
            return self.oob_ip
        return None

    def GetMemorySize(self):
        mem_check_cmd = '''cat /proc/iomem | grep "System RAM" | grep "200000000" | cut  -d'-' -f 1'''
        try:
            self.console.SendlineExpect(mem_check_cmd, "200000000" + '\r\n' + '#', timeout = 1)
            return "8G"
        except:
            return "4G"

    def RunCmdGetOp(self, command):
        if not self.__is_oob_up():
            print ("[%s]: Not Pingable - Cant run ssh command running on console")
            return self.console.RunCmdGetOp(command)
        else:
            return self.console.RunCmdGetOp(command)

    def RunCmd(self, command):
        if not self.__is_oob_up():
            print ("[%s]: Not Pingable - Cant run ssh command running on console")
            self.console.RunCmd(command)
        else:
            self.RunSshCmd(command)
        return

    def RunSshCmd(self, command, ignore_failure = False):
        if not self.__is_oob_up():
            return "[%s]: Not Pingable - Cant run ssh command"

        date_command = "%s %s \"date\"" % (self.ssh_pfx, self.ssh_host)
        os.system(date_command)
        full_command = "%s %s \"%s\"" % (self.ssh_pfx, self.ssh_host, command)
        retcode = os.system(full_command)
        if ignore_failure is False and retcode != 0:
            print("ERROR: Failed to run command: %s" % command)
            raise Exception(full_command)
        return retcode

    def RunSshCmd(self, command, ignore_failure = False):
        if not self.__is_oob_up():
            return "[%s]: Not Pingable - Cant run ssh command"

        date_command = "%s %s \"date\"" % (self.ssh_pfx, self.ssh_host)
        os.system(date_command)
        full_command = "%s %s \"%s\"" % (self.ssh_pfx, self.ssh_host, command)
        retcode = os.system(full_command)
        if ignore_failure is False and retcode != 0:
            print("ERROR: Failed to run command: %s" % command)
            raise Exception(full_command)
        return retcode

    def CopyIN(self, src_filename, dest_dir):
        if not self.__is_oob_up():
            return "[%s]: Not Pingable - Cant run ssh command"

        dest_filename = dest_dir + "/" + os.path.basename(src_filename)
        cmd = "%s %s %s:%s" % (self.scp_pfx, src_filename, self.ssh_host, dest_filename)
        ret = os.system(cmd)
        if ret:
            raise Exception("Enitity : {}, src : {}, dst {} ".format(self.ipaddr, src_filename, dest_filename))
        self.RunSshCmd("sync")
        ret = self.RunSshCmd("ls -l %s" % dest_filename)
        if ret:
            raise Exception("Enitity : {}, src : {}, dst {} ".format(self.ipaddr, src_filename, dest_filename))


if __name__ == '__main__':
    nic = Naples("chamber-ts2", 2028)

    print ("Memory size : ", nic.GetMemorySize())
