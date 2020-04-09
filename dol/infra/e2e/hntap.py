import os
import time
from subprocess import Popen

asic = os.environ.get('ASIC', 'capri')
HNTAP_CMD = '../nic/build/x86_64/iris/' + asic + '/bin/nic_infra_hntap'
hntap_log = os.environ['WS_TOP']  + "/nic/" + "/hntap.log"

class Hntap(object):
    
    def __init__(self, conf_file):
        self._conf_file = conf_file
        self._process = None

    def Run(self):
        log = open(hntap_log, "w")
        cmd = [HNTAP_CMD, '-f', self._conf_file, '-n', '2']
        p = Popen(cmd, stdout=log, stderr=log)
        self._process = p
        print (self._conf_file)
        print("* Starting Host-Network tapper, pid (%s)" % str(p.pid))
        print("    - Log file: " + hntap_log + "\n")

        log2 = open(hntap_log, "r")
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
