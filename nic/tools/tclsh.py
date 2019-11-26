#!/usr/bin/python

import pexpect
import logging
import sys
import os
import argparse
import getpass
import time
import json

def getPid():
    return os.getpid()

def getPwd():
    pwd = os.path.dirname(sys.argv[0])
    pwd = os.path.abspath(pwd)
    return pwd

def getLogFileName():
    pid = getPid()
    username = getpass.getuser()
    path = '/home/' + username + '/'
    fileName = path + "tclsh.log." + str(pid)
    return fileName

def initLogger():
    fileName = getLogFileName()
    # Log to file: 'logs_tclsh'
    logging.basicConfig(level=logging.INFO,
                        format='%(asctime)s - %(levelname)s - %(message)s',
                        filename=fileName,
                        filemode='a')
    # define a new Handler to log to console as well
    console = logging.StreamHandler()
    # optional, set the logging level
    console.setLevel(logging.INFO)
    # set a format which is the same for console use
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    # tell the handler to use this format
    console.setFormatter(formatter)
    # add the handler to the root logger
    logging.getLogger('').addHandler(console)

    # Now, we can log to both the file and console

def sendCmd(self, cmd, exp, **kwargs):
    if "timeout" in kwargs:
        timeout = kwargs["timeout"]
    else:
        timeout = 30

    self.sendline(cmd)
    self.expect(exp, timeout=timeout)
    return self.before

def copyToHost(src, dest, host):
    logging.info("Copying {0} to {1}\n".format(src, host))
    cmd = 'sshpass -p docker scp -o StrictHostKeyChecking=no ' + src + ' root@' + host + ':' + dest
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("ERROR: Failed to copy {0} to {1}".format(src, host))
        exit()
    else:
        logging.info("Copied {0} successfully to {1}".format(src, host))

    return

if __name__ == "__main__":
    initLogger()
    parser = argparse.ArgumentParser()

    parser.add_argument('--host', help='hostname of server where NIC is present. e.g. --host cap-rdma')

    args=parser.parse_args()

    if (len(sys.argv) == 1):
        parser.print_help()
        exit()
    else:
        print("Loading tclsh on {0}. Please wait...\n".format(args.host))

        pwd = getPwd()
        cfg = pwd + '/../conf/dev.json'
        with open(cfg, 'r') as f:
            dev_dict = json.load(f)
        
        found = False
        for dev in dev_dict:
            if (dev["host"] == args.host):
                logging.info(dev["host"])
                found = True
                break

        if found is False:
            logging.info("ERROR: device info not found in conf/dev.json. Please update and try again")
            exit()

    # check if host is running linux or freebsd
    logging.info("Checking if {0} is running linux or freebsd\n".format(args.host))
    cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host + ' -t uname'
    op = os.popen(cmd).read()

    if ("Linux" in op):
        hostOS = 'Linux'
    elif ("FreeBSD" in op):
        hostOS = 'FreeBSD'
    else:
        logging.info('Unknown OS type')
        exit()

    # check if tcl tar file is already present on the host
    logging.info("Checking if tcl tar file is already present on {0}\n".format(args.host))
    cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host + ' -t ls -lrt /tmp/nic.tar.gz'
    ret = os.system(cmd)
    if (ret != 0):
        copyToHost('/home/arun/tcl/nic.tar.gz', '/tmp', args.host)

    # verify that ping works between arm and host thru mnic intf
    mnic_intf = dev['if2']
    cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host + ' -t ping -c 5 169.254.0.1'
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("Ping failed between arm and host")

        # try to setup MNIC interface
        if hostOS == 'Linux':
            cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host + ' -t ip addr add 169.254.0.2/24 dev ' + mnic_intf
            ret = os.system(cmd)
            if (ret != 0):
                logging.info("ERROR: Failed to assign IP to MNIC interface")
                exit()
            cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host + ' -t ifconfig ' + mnic_intf + ' up'
            ret = os.system(cmd)
            if (ret != 0):
                logging.info("ERROR: Failed to bring up MNIC interface")
                exit()

        elif hostOS == 'FreeBSD':
            cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host + ' -t ifconfig ' + mnic_intf + ' 169.254.0.2/24 up'
            ret = os.system(cmd)
            if (ret != 0):
                logging.info("ERROR: Failed to assign IP to and/or bring up MNIC interface")
                exit()

        logging.info("Configured MNIC interface {0}".format(mnic_intf))
        # verify that ping works between arm and host thru mnic intf
        cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host + ' -t ping -c 5 169.254.0.1'
        ret = os.system(cmd)
        if (ret != 0):
            logging.info("ERROR: Ping failed between arm and host")
            exit()

    else:
        logging.info("ping successful through MNIC interface\n")

    # ssh to host
    cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host
    ch = pexpect.spawn (cmd)
    fileName = getLogFileName()
    fout = open(fileName,'a')
    ch.logfile = fout
    ch.expect('#')

    op = sendCmd(ch, 'rm /root/.ssh/known_hosts', '#')

    sendCmd(ch, 'ssh -o StrictHostKeyChecking=no root@169.254.0.1', '[Pp]assword.*')
    sendCmd(ch, 'pen123', '#')

    op = sendCmd(ch, 'rm /root/.ssh/known_hosts', '#')
    
    logging.info("Copying nic.tar.gz\n")
    sendCmd(ch, 'scp -o StrictHostKeyChecking=no root@169.254.0.2:/tmp/nic.tar.gz /data/', '[Pp]assword.*')
    sendCmd(ch, 'docker', '#')

    sendCmd(ch, 'rm -rf /data/asic_tclsh', '#')
    sendCmd(ch, 'mkdir /data/asic_tclsh', '#')
    sendCmd(ch, 'cp /data/nic.tar.gz /data/asic_tclsh', '#')
    sendCmd(ch, 'cd /data/asic_tclsh', '#')
    logging.info("Extracting nic.tar\n")
    sendCmd(ch, 'gunzip nic.tar.gz', '#')

    logging.info("Extracting nic\n")
    sendCmd(ch, 'tar xvf nic.tar', '#')

    sendCmd(ch, 'cd nic/fake_root_target/nic/', '#')
    sendCmd(ch, 'export ASIC_LIB_BUNDLE=`pwd`', '#')
    sendCmd(ch, 'export ASIC_SRC=$ASIC_LIB_BUNDLE/asic_src', '#')
    sendCmd(ch, 'cd asic_lib/', '#')
    sendCmd(ch, 'source ./source_env_path', '#')

    sendCmd(ch, './diag.exe', '%')
    sendCmd(ch, 'source $::env(ASIC_SRC)/ip/cosim/tclsh/.tclrc.diag', '%')
    logging.info("Loaded tclsh successfully\n")
    ch.interact()
