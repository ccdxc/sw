import pdb
import re
import sys
import telnetlib
import time
import traceback


class TelnetPromptException(Exception):
    pass


class TelnetClient(object):

    def __init__(self,host,username,password,userPrompt,passPrompt,cmdPrompt,timeout=30):
        self.host = host
        self.username = username
        self.password = password
        self.hdl = None
        self.userPrompt = userPrompt
        self.passPrompt = passPrompt
        self.cmdPrompt = cmdPrompt
        self.timeout = timeout

    def expect(self,prompts,timeout):
        res = self.hdl.expect(prompts,timeout)
        if res[0] == -1:
            raise TelnetPromptException('failed to find telnet prompts: {0}. found up to: {1}'.format(prompts, res[2]))
        return res[2]

    def login(self):
        if not self.hdl:
            self.hdl = telnetlib.Telnet(self.host,timeout=5)
        self.expect([self.userPrompt],5)
        self.hdl.write(self.username + "\r")
        self.expect([self.passPrompt],5)
        self.hdl.write(self.password + "\r")
        self.expect([self.cmdPrompt],5)

    def cmd(self,cmd):
        if not self.hdl:
            self.login()
        if not cmd.endswith('\r'):
            cmd += '\r'
        self.hdl.write(cmd)
        return self.expect([self.cmdPrompt],self.timeout)

    def close(self):
        if self.hdl:
            self.hdl.write('exit')
            self.hdl = None


class ApcControl(object):

    def __init__(self,host,username,password):
        self.host = host
        self.username = username
        self.password = password
        self.hdl = None

    def rawCmd(self,cmd):
        if not self.hdl:
            self.hdl = TelnetClient(self.host, self.username, self.password, 'User Name :', 'Password  :', 'apc>')
        res = self.hdl.cmd(cmd)
        self.hdl.close()
        return res

    def portOff(self,port):
        self.rawCmd('olOff ' + str(port))

    def portOn(self,port):
        self.rawCmd('olOn ' + str(port))

    def portStatus(self,port):
        res = self.rawCmd('olstatus ' + str(port))
        found = re.search(' '+str(port)+': Outlet ' + str(port) + ': ([\w]+)',res)
        if not found:
            raise Exception('failed to determine apc port status for port {0}'.format(port))
        return found.group(1)


def test_apc():
    apc = ApcControl('192.168.68.39', 'apc', 'apc')
    print(apc.portStatus('5'))
    apc.portOff('5')
    time.sleep(3)
    print(apc.portStatus('5'))
    apc.portOn('5')
    time.sleep(3)
    print(apc.portStatus('5'))



