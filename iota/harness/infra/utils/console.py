import time
import pexpect
from datetime import datetime
import sys
import re

TIMEOUT = 180
CONSOLE_SVR_USERNAME = 'admin'
CONSOLE_SVR_PASSWORD = 'N0isystem$'


class Console(object):
    def __init__(self, console_ip, console_port, disable_log=False):
        self.console_ip = console_ip
        self.console_port = console_port
        self.console_svr_username = CONSOLE_SVR_USERNAME
        self.console_svr_password = CONSOLE_SVR_PASSWORD
        self.__disable_log = disable_log
        self.__clear_line()
        self.hdl = self.__get_handle()

    def __spawn(self, command):
        print (command)
        hdl = pexpect.spawn(command)
        hdl.timeout = TIMEOUT
        if not self.__disable_log:
           hdl.logfile = sys.stdout.buffer
        return hdl

    def __sendline_expect(self, line, expect, hdl, timeout = TIMEOUT):
        hdl.sendline(line)
        return hdl.expect_exact(expect, timeout)

    def SendlineExpect(self, line, expect, timeout = TIMEOUT):
        self.hdl.sendline(line)
        return self.hdl.expect_exact(expect, timeout)

    def __get_handle(self):
        self.hdl = self.__spawn("telnet %s %s" % (self.console_ip, self.console_port))
        return self.hdl

    def __run_cmd(self, cmd):
        if self.hdl == None or not self.hdl.isalive():
            self.hdl = self.__get_handle()
        self.hdl.sendline(cmd)
        self.hdl.expect("#")
        return

    def __get_output(self, command):
        self.__run_cmd(command)
        return self.hdl.before

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
            self.__sendline_expect("clear line %d" % (int(self.console_port) - 2000), "[confirm]", hdl = hdl)
            self.__sendline_expect("", " [OK]", hdl = hdl)
            time.sleep(1)
        hdl.close()
        time.sleep(1)

    def RunCmdGetOp(self, cmd):
        return self.__get_output(cmd)

    def RunCmd(self, cmd):
        return self.__run_cmd(cmd)
