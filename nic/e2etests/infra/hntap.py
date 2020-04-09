import os
import time
import json
import subprocess
import pdb
import consts

from infra.app import AppFactory

asic = os.environ.get('ASIC', 'capri')
HNTAP_CMD = 'build/x86_64/iris/' + asic + '/bin/nic_infra_hntap'
CONTAINER_HNTAP_CMD = "naples/nic/tools/restart-hntap.sh"

def preexec(): # Don't forward signals.
    os.setpgrp()

class Hntap(object):
    
    def __init__(self, conf_file):
        self._conf_file = conf_file

    def Run(self, nomodel=False):
        pass
    
    def Stop(self):
        pass
    
    def _wait_for_hntap_up(self, log_file=consts.hntap_log):
        log2 = open(log_file, "r")
        loop = 1
        time.sleep(2)

        # Wait until tap devices setup is complete
        while loop == 1:
            for line in log2.readlines():
                if "listening on" in line:
                    loop = 0
        log2.close()
        
class HntapLocal(Hntap):
    
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
        p = subprocess.Popen(cmd, stdout=log, stderr=log, preexec_fn = preexec)
        self._process = p
        print("* Starting Host-Network tapper, pid (%s)" % str(p.pid))
        print("    - Log file: " + consts.hntap_log + "\n")
        self._wait_for_hntap_up(consts.hntap_log)
        return
    
    def Stop(self):
        os.kill(int(self._process.pid), 9)

        
class HntapInContainer(Hntap):
    
    def __init__(self, conf_file, nic_container,
                container_conf_path=consts.hntap_container_conf_path):
        self._conf_file = conf_file
        self._nic_container_hntap_cfg_file = container_conf_path 
        self._nic_container = nic_container
        self._process = None

    def Run(self, nomodel=False):
        log = open(consts.hntap_container_log, "w")
        log.close()
        #First copy the new hntap config file
        print ("Copying Hntap configuration file to nic container")
        copy_cmd = "docker cp " + self._conf_file + "  " + self._nic_container + ":" + self._nic_container_hntap_cfg_file
        p = subprocess.run(copy_cmd, shell=True)
        
        #Restart Hntap process
        print ("Restarting Hntap process in nic container")
        nic_container = AppFactory.Get("nic_container", id=self._nic_container)
        nic_container.RunCommand(CONTAINER_HNTAP_CMD, background=True, tty=False)
        self._wait_for_hntap_up(consts.hntap_container_log)
        
        #Move the interface to namespace 1
        print ("Moving interfaces to namespace 1")
        data = json.load(open(self._conf_file))
        for intf in data:
            print ("Moving interface " + intf["name"])
            nic_container.MoveInterface(intf["name"], 1)

    def Stop(self):
        pass
    

TypeHntapInContainer = HntapInContainer
TypeHntapLocal = HntapLocal

class HntapFactory():
    @staticmethod
    def Get(config_file, container=None):
        if container:
            return HntapInContainer(config_file, container)
        else:
            return HntapLocal(config_file)
