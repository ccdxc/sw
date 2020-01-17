#! /usr/bin/python3
import pdb
import pprint
import traceback
import re
import sys
import pexpect
import time

from google.protobuf import text_format
class Dict2Enum(object):
    def __init__(self, entries):
        self.str_enums = entries
        entries = dict((v.replace(" ", "_"), k) for k, v in entries.items())
        self.__dict__.update(entries)

    def valid(self, name):
        return name in self.__dict__

    def id(self, name):
        return self.__dict__[name]

    def str(self, v):
        return self.str_enums[v]

def List2Enum(entries):
    db = {}
    value = 0
    for entry in entries:
        db[value] = entry
        value += 1
    return Dict2Enum(db)

def LogMessageContents(hdrstr, msg, logfn):
    logfn("%s %s:" % (msg.__class__.__name__, hdrstr))
    msgstr = text_format.MessageToString(msg).split('\n')
    for s in msgstr:
        if s != '': logfn("- %s: " % s)
    return

def LogException(lg):
    exc_type, exc_value, exc_traceback = sys.exc_info()
    lg.info("Exception: %s-- %s" % (exc_type, exc_value))
    lg.info('-' * 60)
    backtrace = pprint.pformat(traceback.format_tb(exc_traceback))
    for line in backtrace.split('\n'):
        lg.info("%s" % line)
    lg.info('-' * 60)

class NaplesConsoleManagement():
    def __init__(self, console_ip = None, console_port = None, console_password = 'N0isystem$', username = 'root', password = 'pen123', timeout = 180):
        self.host = None
        self.hdl = None
        self.username = username
        self.password = password
        self.console_ip = console_ip
        self.console_port = int(console_port)
        self.console_password = console_password
        self.timeout = timeout
        return

    def SendlineExpect(self, line, expect, hdl = None):
        if hdl is None: hdl = self.hdl
        hdl.sendline(line)
        return hdl.expect_exact(expect, self.timeout)

    def Spawn(self, command):
        hdl = pexpect.spawn(command)
        hdl.timeout = self.timeout
        return hdl

    def __run_cmd(self, cmd, timeout = 180):
        self.hdl.sendline(cmd)
        self.hdl.expect("#",timeout=timeout)

    def RunCommandOnConsoleWithOutput(self, cmd, timeout = 180):
        self.__run_cmd(cmd, timeout)
        return self.hdl.before

    def __clearline(self):
        try:
            print("Clearing Console Server Line")
            hdl = self.Spawn("telnet %s" % self.console_ip)
            idx = hdl.expect(["Username:", "Password:"])
            if idx == 0:
                self.SendlineExpect(self.console_username, "Password:", hdl = hdl)
            self.SendlineExpect(self.console_password, "#", hdl = hdl)

            for i in range(6):
                time.sleep(5)
                self.SendlineExpect("clear line %d" % (self.console_port - 2000), "[confirm]", hdl = hdl)
                print(hdl.before)
                self.SendlineExpect("", " [OK]", hdl = hdl)
                print(hdl.before)
            hdl.close()
        except:
            raise Exception("Clear line failed ")

    def __login(self):
        midx = self.SendlineExpect("", ["#", "capri login:", "capri-gold login:"],
                                   hdl = self.hdl)
        if midx == 0: return
        # Got capri login prompt, send username/password.
        self.SendlineExpect(self.username, "Password:")
        ret = self.SendlineExpect(self.password, ["#", pexpect.TIMEOUT])
        if ret == 1: self.SendlineExpect("", "#")

    def Connect(self):
        for _ in range(3):
            try:
                self.hdl = self.Spawn("telnet %s %s" % ((self.console_ip, self.console_port)))
                midx = self.hdl.expect_exact([ "Escape character is '^]'.", pexpect.EOF])
                if midx == 1:
                    raise Exception("Failed to connect to Console %s %d" % (self.console_ip, self.console_port))
            except:
                try:
                    self.__clearline()
                except:
                    print("Expect Failed to clear line %s %d" % (self.console_ip, self.console_port))
                continue
            break
        else:
            #Did not break, so connection failed.
            msg = "Failed to connect to Console %s %d" % (self.console_ip, self.console_port)
            print(msg)
            raise Exception(msg)

        self.__login()

    def Close(self):
        if self.hdl:
            self.hdl.close()
        return
