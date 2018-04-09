import os
import time
from subprocess import Popen
import consts

HNTAP_CMD = '../bazel-bin/nic/e2etests/nic_infra_hntap'

def preexec(): # Don't forward signals.
    os.setpgrp()

class Hntap(object):
    
    def __init__(self, conf_file):
        self._conf_file = conf_file
        self._process = None

    def Run(self, nomodel=False):
        log = open(consts.hntap_log, "w")
        cmd = [HNTAP_CMD, '-f', self._conf_file, '-n', '2']
        if nomodel:
            cmd.append("-s")
        os.chdir(consts.nic_dir)
        print (cmd)
        p = Popen(cmd, stdout=log, stderr=log, preexec_fn = preexec)
        self._process = p
        print("* Starting Host-Network tapper, pid (%s)" % str(p.pid))
        print("    - Log file: " + consts.hntap_log + "\n")

        log2 = open(consts.hntap_log, "r")
        loop = 1
        time.sleep(2)

        # Wait until tap devices setup is complete
        while loop == 1:
            for line in log2.readlines():
                if "listening on" in line:
                    loop = 0
        log2.close()
        return
    
    def Stop(self):
        os.kill(int(self._process.pid), 9)   