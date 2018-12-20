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

    # check if memtun is already present on the host
    logging.info("Checking if memtun is already present on {0}\n".format(args.host))
    cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host + ' -t ls -lrt /tmp/memtun'
    ret = os.system(cmd)
    if (ret != 0):
        copyToHost('/home/haps/memtun/memtun', '/tmp', args.host)

    # check if tcl tar file is already present on the host
    logging.info("Checking if tcl tar file is already present on {0}\n".format(args.host))
    cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host + ' -t ls -lrt /tmp/nic.tar.gz'
    ret = os.system(cmd)
    if (ret != 0):
        copyToHost('/home/sanshanb/util/nic.tar.gz', '/tmp', args.host)

    # ssh to host
    cmd = 'ssh root@' + args.host
    ch = pexpect.spawn (cmd)
    fileName = getLogFileName()
    fout = open(fileName,'a')
    ch.logfile = fout
    ch.expect('Password: ')
    sendCmd(ch, 'docker', '#')

    # check if memtun is running on the host
    op = sendCmd(ch, 'ps -ef | grep memtun | grep -v grep', '#')
    if ("memtun 1.0.0.1" not in op):
        logging.info("memtun is not running on the host\n")

        # try to start memtun
        logging.info("Starting memtun. Please wait...")
        op = sendCmd(ch, '/tmp/memtun 1.0.0.1 &', '#')
        time.sleep(5)

        # check if memtun was started successfully
        op = sendCmd(ch, 'ps -ef | grep memtun | grep -v grep', '#')
        if ("memtun 1.0.0.1" not in op):
            logging.info("ERROR: failed to start memtun on the host\n")
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
    if ("No such file or directory" in op):
        logging.info("Copying nic.tar.gz\n")
        sendCmd(ch, 'scp -o StrictHostKeyChecking=no root@1.0.0.1:/home/sanshanb/util/nic.tar.gz /', 'Password: ')
        sendCmd(ch, 'docker', '#')
        sendCmd(ch, 'mkdir /asic_tclsh', '#')
        sendCmd(ch, 'cd /asic_tclsh', '#')
        sendCmd(ch, 'mv /nic.tar.gz .', '#')
    else:
        logging.info("Found existing nic.tar.gz\n")

    op = sendCmd(ch, 'ls -lrt /asic_tclsh/nic.tar', '#')
    if ("No such file or directory" in op):
        logging.info("Extracting nic.tar\n")
        sendCmd(ch, 'gunzip nic.tar.gz', '#')
    else:
        logging.info("Found existing nic.tar\n")

    op = sendCmd(ch, 'ls -lrt /asic_tclsh/nic', '#')
    if ("No such file or directory" in op):
        logging.info("Extracting nic\n")
        sendCmd(ch, 'tar xvf nic.tar', '#')
    else:
        logging.info("Found existing nic\n")

    sendCmd(ch, 'cd /asic_tclsh/nic/', '#')
    sendCmd(ch, 'cd fake_root_target/nic/', '#')
    sendCmd(ch, 'export ASIC_LIB_BUNDLE=`pwd`', '#')
    sendCmd(ch, 'export ASIC_SRC=$ASIC_LIB_BUNDLE/asic_src', '#')
    sendCmd(ch, 'cd asic_lib/', '#')
    sendCmd(ch, 'source ./source_env_path', '#')

    sendCmd(ch, './diag.exe', '%')
    sendCmd(ch, 'source $::env(ASIC_SRC)/ip/cosim/tclsh/.tclrc.diag', '%')
    logging.info("Loaded tclsh successfully\n")
    ch.interact()
