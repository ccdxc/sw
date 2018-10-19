#!/usr/bin/python

import pexpect
import logging
import sys
import os
import argparse
import getpass

def getPid():
    return os.getpid()

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

def sendCmd(self, cmd, exp):
    self.sendline(cmd)
    self.expect(exp)
    return self.before

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

    cmd = 'ssh root@' + args.host
    ch = pexpect.spawn (cmd)
    fileName = getLogFileName()
    fout = open(fileName,'a')
    ch.logfile = fout
    ch.expect('Password: ')
    sendCmd(ch, 'docker', '#')

    op = sendCmd(ch, 'ps -ef | grep memtun | grep -v grep', '#')
    if ("memtun 1.0.0.1" not in op):
        logging.info("ERROR: memtun is not running on the host\n")
        exit()
    else:
        logging.info("memtun is running on the host\n")

    op = sendCmd(ch, 'rm /root/.ssh/known_hosts', '#')

    sendCmd(ch, 'ssh -o StrictHostKeyChecking=no root@1.0.0.2', 'root@1.0.0.2\'s password: ')
    sendCmd(ch, 'pen123', '#')

    op = sendCmd(ch, 'rm /root/.ssh/known_hosts', '#')
    op = sendCmd(ch, 'ps -ef | grep memtun | grep -v grep', '#')
    if ("/platform/bin/memtun" not in op):
        logging.info("ERROR: memtun is not running on the NIC\n")
        exit()
    else:
        logging.info("memtun is running on the NIC\n")

    op = sendCmd(ch, 'ls -lrt /asic_tclsh/nic.tar.gz', '#')
    if ("/asic_tclsh/nic.tar.gz not in op"):
        logging.info("Copying nic.tar.gz\n")
        sendCmd(ch, 'scp -o StrictHostKeyChecking=no root@1.0.0.1:/home/sanshanb/util/nic.tar.gz /', 'Password: ')
        sendCmd(ch, 'docker', '#')
        sendCmd(ch, 'mkdir /asic_tclsh', '#')
        sendCmd(ch, 'cd /asic_tclsh', '#')
        sendCmd(ch, 'mv /nic.tar.gz .', '#')
    else:
        logging.info("Found existing nic.tar.gz\n")

    op = sendCmd(ch, 'ls -lrt /asic_tclsh/nic.tar', '#')
    if ("/asic_tclsh/nic.tar not in op"):
        logging.info("Extracting nic.tar\n")
        sendCmd(ch, 'gunzip nic.tar.gz', '#')
    else:
        logging.info("Found existing nic.tar\n")

    op = sendCmd(ch, 'ls -lrt /asic_tclsh/nic', '#')
    if ("/asic_tclsh/nic not in op"):
        logging.info("Extracting nic\n")
        sendCmd(ch, 'tar xvf nic.tar', '#')
    else:
        logging.info("Found existing nic\n")

    sendCmd(ch, 'cd nic/', '#')
    sendCmd(ch, 'cd fake_root_target/nic/', '#')
    sendCmd(ch, 'export ASIC_LIB_BUNDLE=`pwd`', '#')
    sendCmd(ch, 'export ASIC_SRC=$ASIC_LIB_BUNDLE/asic_src', '#')
    sendCmd(ch, 'cd asic_lib/', '#')
    sendCmd(ch, 'source ./source_env_path', '#')

    sendCmd(ch, './diag.exe', '%')
    sendCmd(ch, 'source $::env(ASIC_SRC)/ip/cosim/tclsh/.tclrc.diag', '%')
    logging.info("Loaded tclsh successfully\n")
    ch.interact()
