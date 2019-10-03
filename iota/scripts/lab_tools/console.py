import time
import pexpect
from datetime import datetime
import sys
import re

TIMEOUT = 30
CONSOLE_USERNAME = 'root'
CONSOLE_PASSWORD = 'pen123'
CONSOLE_SVR_USERNAME = 'admin'
CONSOLE_SVR_PASSWORD = 'N0isystem$'


class Console(object):
    def __init__(self, console_ip, console_port):
        self.console_ip = console_ip
        self.console_port = console_port
        self.console_username = CONSOLE_USERNAME
        self.console_password = CONSOLE_PASSWORD
        self.console_svr_username = CONSOLE_SVR_USERNAME
        self.console_svr_password = CONSOLE_SVR_PASSWORD
        self.__clear_line()
        self.hdl = self.__get_handle()
        self.oob_enabled = False
        self.oob_ip = None
        #self.__enable_dhcp_on_oob()
        #self.oob_ip = self.__get_oob_ip()
        return

    def __spawn(self, command):
        print (command)
        hdl = pexpect.spawn(command)
        hdl.timeout = TIMEOUT
        hdl.logfile = sys.stdout
        return hdl

    def __sendline_expect(self, line, expect, hdl, timeout = TIMEOUT):
        hdl.sendline(line)
        return hdl.expect_exact(expect, timeout)

    def SendlineExpect(self, line, expect, timeout = TIMEOUT):
        self.hdl.sendline(line)
        return self.hdl.expect_exact(expect, timeout)

    def __login(self, dhl):
        midx = self.__sendline_expect("", ["#", "capri login:", "capri-gold login:"], hdl = self.hdl, timeout = 120)
        if midx == 0: return
        # Got capri login prompt, send username/password.
        self.__sendline_expect(self.console_username, "Password:",  hdl = self.hdl)
        ret = self.__sendline_expect(self.console_password, ["#", pexpect.TIMEOUT],  hdl = self.hdl, timeout = 3)
        if ret == 1: self.__sendline_expect("", "#",  hdl = self.hdl)

    def __get_handle(self):
        self.hdl = self.__spawn("telnet %s %s" % (self.console_ip, self.console_port))
        self.__login(self.hdl)
        return self.hdl

    def __run_cmd(self, cmd):
        if self.hdl == None or not self.hdl.isalive():
            self.hdl = self.__get_handle()
        self.hdl.sendline(cmd)
        self.hdl.expect("#")
        return

    def __enable_dhcp_on_oob(self):
        cmd = "dhclient oob_mnic0 &"
        self.__run_cmd(cmd)
        return True

    def __get_output(self, command):
        self.__run_cmd(command)
        return self.hdl.before

    def __get_oob_ip(self):
        if self.oob_ip != None:
            return self.oob_ip
        self.__enable_dhcp_on_oob()
        output = self.__get_output("ifconfig oob_mnic0")
        ifconfig_regexp = "addr:(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})"
        x = re.findall(ifconfig_regexp, output)
        if len(x) > 0:
            return x[0]
        return None

    def __str__(self):
        return "[%s]" % self.console_ip

    @property
    def __log_pfx(self):
        return "[%s][%s]" % (str(datetime.now()), str(self))

    def __clear_line(self):
        print("%sClearing Console Server Line" % self.__log_pfx)
        hdl = self.__spawn("telnet %s -l %s" % (self.console_ip, self.console_svr_username))
        hdl.expect_exact("Password:")
        self.__sendline_expect(self.console_svr_password, "#", hdl = hdl)

        for i in range(3):
            self.__sendline_expect("clear line %d" % (self.console_port - 2000), "[confirm]", hdl = hdl)
            self.__sendline_expect("", " [OK]", hdl = hdl)
            time.sleep(1)
        hdl.close()
        time.sleep(1)

    def EnableDhcpOnOob(self):
        return self.__enable_dhcp_on_oob()

    def GetOobIp(self):
        return self.__get_oob_ip()

    def RunCmdGetOp(self, cmd):
        return self.__get_output(cmd)

    def RunCmd(self, cmd):
        return self.__run_cmd(cmd)
