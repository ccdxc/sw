import defines
import time
import pexpect
from datetime import datetime

TIMEOUT = 60
CIMC_USERNAME = 'admin'
CIMC_PASSWORD = 'N0isystem$'
HOST_USERNAME = 'root'
HOST_PASSWORD = 'docker'

class cimc(object):
    def __init__(self, cimc):
        self.cimc   = cimc
        self.username = CIMC_USERNAME
        self.password = CIMC_PASSWORD
        return 
    
    @property
    def __log_pfx(self):
        return "[%s][%s]" % (str(datetime.now()), str(self))

    def __str__(self):
        return "[%s]" % (self.cimc)

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
