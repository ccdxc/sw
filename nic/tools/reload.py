#!/usr/bin/python

import pexpect
import logging
import sys
import os
import argparse
import json
import time
import getpass
import telnetlib

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
    fileName = path + "reload.log." + str(pid)
    return fileName

def initLogger():
    fileName = getLogFileName()
    # Log to file: 'logs_tclsh'
    logging.basicConfig(level=logging.INFO,
                        format='%(asctime)s - %(levelname)s - %(message)s',
                        filename=fileName,
                        filemode='w')
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

def sendTelnetCmd(self, cmd, exp, **kwargs):

    if "timeout" in kwargs:
        timeout = kwargs["timeout"]
    else:
        timeout = 30

    self.write(cmd)
    op = self.expect(exp, timeout=timeout)
    return op

def clearLine(console, port):
    tn = telnetlib.Telnet(console)
    logging.info('Clearing line for {0}'.format(console))
    tn.expect(["Password: "], timeout=5)

    sendTelnetCmd(tn, 'N0isystem$\n', ['#'])
    cmd = 'clear line ' + str(port) + '\n'
    sendTelnetCmd(tn, cmd, ['confirm'])
    sendTelnetCmd(tn, 'y\n', ['#'])

    tn.write("exit\n")
    tn.close()
    logging.info('Done')

def telnetToConsole(console, port):
    logging.info('telnet to {0} {1}'.format(console, port))
    tn = telnetlib.Telnet(console, port)
    time.sleep(2)
    tn.write("\n")
    op = tn.expect(['capri login: ', '#'], timeout=5)
    logging.info(op[2])
    if op[0] == 0:
        tn.write("root\n")
        op = tn.expect(['Password: '], timeout=5)
        logging.info(op[2])
        tn.write("pen123\n")
        op = tn.expect(['#'], timeout=5)
        logging.info(op[2])

    tn.write("ls -lrt /\n")
    op = tn.expect(['#'], timeout=5)
    logging.info(op[2])
    return tn

def update(ch):
    # check if memtun is running on the host
    op = sendCmd(ch, 'ps -ef | grep memtun | grep -v grep', '#')
    if ("memtun 1.0.0.1" not in op):
        logging.info("memtun is not running on the host\n")

        # try to start memtun
        logging.info("Starting memtun")
        op = sendCmd(ch, '/tmp/memtun 1.0.0.1 &', '#')
        time.sleep(5)

        # check if memtun was started successfully
        op = sendCmd(ch, 'ps -ef | grep memtun | grep -v grep', '#')
        if ("memtun 1.0.0.1" not in op):
            logging.info("ERROR: failed to start memtun on the host\n")
            exit()
    else:
        logging.info("memtun is running on the host\n")

    logging.info("Removing known_hosts")
    op = sendCmd(ch, 'rm /root/.ssh/known_hosts', '#')

    logging.info("Looking for naples_fw.tar in /tmp\n")
    op = sendCmd(ch, 'ls -lrt /tmp/naples_fw.tar', '#')
    if "No such file or directory" in op:
        logging.info("ERROR: /tmp/naples_fw.tar not found on the host")
        exit()

    logging.info("Copying naples_fw.tar to naples\n")
    sendCmd(ch, 'scp -o StrictHostKeyChecking=no /tmp/naples_fw.tar root@1.0.0.2:/tmp/', 'root@1.0.0.2\'s password: ')
    sendCmd(ch, 'pen123', '#')

    sendCmd(ch, 'ssh -o StrictHostKeyChecking=no root@1.0.0.2', 'root@1.0.0.2\'s password: ')
    sendCmd(ch, 'pen123', '#')

    logging.info("Updating naples_fw.tar on naples\n")
    op = sendCmd(ch, '/nic/tools/sysupdate.sh -p /tmp/naples_fw.tar', '#', timeout=360)
    sendCmd(ch, 'exit', '#')

def reset(ch, dev):
    clearLine(dev['console'], dev['port'])
    time.sleep(3)

    port = int(2000) + int(dev["port"])
    t_hdl = telnetToConsole(dev["console"], port)

    logging.info("Resetting naples. Please wait...\n")
    op = sendTelnetCmd(t_hdl, '/nic/tools/sysreset.sh\n', ['login: '], timeout=180)
    logging.info(op[2])
    op = sendTelnetCmd(t_hdl, 'root\n', ['Password: '])
    logging.info(op[2])
    op = sendTelnetCmd(t_hdl, 'pen123\n', ['#'])
    logging.info(op[2])

    t_hdl.write('exit\n')
    t_hdl.close()
    logging.info("Rebooting the host {0}\n".format(dev['host']))
    ch.sendline('reboot')

    logging.info("Waiting for the host to be back up...\n")
    time.sleep(300)
    ch.close()

    # copy memtun to the host    
    logging.info("Copying memtun to {0}\n".format(dev['host']))
    cmd = 'sshpass -p docker scp -o StrictHostKeyChecking=no /home/haps/memtun/memtun root@' + dev['host'] + ':/tmp/'
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("ERROR: Failed to copy memtun to {0}".format(dev['host']))
        exit()

    # check if the drivers tar file is already present
    drivers = pwd + '/../../platform/gen/drivers-linux.tar.xz'
    cmd = 'ls -lrt ' + drivers
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("drivers tar file is not present in the workspace. Generating it")
        cmd = pwd + '/../../platform/tools/drivers-linux.sh'
        ret = os.system(cmd)
        if (ret != 0):
            logging.info("ERROR: Failed to get the drivers tar file")
            exit()

    # copy the drivers to the host    
    logging.info("Copying drivers-linux.tar.xz to {0}\n".format(dev['host']))
    cmd = 'sshpass -p docker scp -o StrictHostKeyChecking=no ' + drivers + ' root@' + dev['host'] + ':/tmp/'
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("ERROR: Failed to copy drivers-linux.tar.xz to {0}".format(dev['host']))
        exit()

    ch = sshToHost(dev['host'])
    loadDrivers(ch, dev)
    ch.logfile = None
    logging.info("\nReloaded naples successfully\n")
    exit()

def sshToHost(host):
    logging.info("ssh to {0}".format(host))
    # SSH to the host
    cmd = 'ssh root@' + host
    ch = pexpect.spawn (cmd)
    ch.logfile = sys.stdout
    ch.expect('Password: ')
    sendCmd(ch, 'docker', '#')
    return ch

def loadDrivers(ch, dev):
    logging.info("Starting memtun\n")
    op = sendCmd(ch, '/tmp/memtun 1.0.0.1 &', '#')

    logging.info("Installing drivers")

    # verify that the drivers tar file is present on the host
    sendCmd(ch, 'cd /tmp', '#')
    op = sendCmd(ch, 'ls -lrt drivers-linux.tar.xz', '#')
    if "No such file or directory" in op:
        logging.info("ERROR: drivers-linux.tar.xz not found on the host")
        exit()

    sendCmd(ch, 'tar xaf drivers-linux.tar.xz', '#')
    sendCmd(ch, 'cd drivers-linux', '#')
    sendCmd(ch, './build.sh', '#', timeout=60)
    sendCmd(ch, 'insmod drivers/eth/ionic/ionic.ko', '#')
    sendCmd(ch, 'modprobe ib_uverbs', '#')
    sendCmd(ch, 'insmod drivers/rdma/drv/ionic/ionic_rdma.ko', '#')

    logging.info("Bringing up interface {0}\n".format(dev['if0']))
    cmd = 'ip addr add ' + dev['if0_ip'] + '/' + dev['if0_mask'] + ' dev ' + dev['if0']
    op = sendCmd(ch, cmd, '#')
    cmd = 'ifconfig ' + dev['if0'] + ' up'
    op = sendCmd(ch, cmd, '#')
    cmd = 'sudo ifconfig -s ' + dev['if0'] + ' mtu 8192'
    op = sendCmd(ch, cmd, '#')
    cmd = 'arp -s ' + dev['if0_peer_ip'] + ' ' + dev['if0_peer_mac']
    op = sendCmd(ch, cmd, '#')
    cmd = 'ping -c 5 ' + dev['if0_peer_ip']
    op = sendCmd(ch, cmd, '#', timeout=20)
    if "5 packets transmitted, 5 received" not in op:
        logging.info("===========================================")
        logging.info("ERROR: Packet loss with ping. Please check")
        logging.info("===========================================")
    else:
        logging.info("Ping passed")

    logging.info("Bringing up interface {0}\n".format(dev['if1']))
    cmd = 'ip addr add ' + dev['if1_ip'] + '/' + dev['if1_mask'] + ' dev ' + dev['if1']
    op = sendCmd(ch, cmd, '#')
    cmd = 'ifconfig ' + dev['if1'] + ' up'
    op = sendCmd(ch, cmd, '#')
    cmd = 'sudo ifconfig -s ' + dev['if1'] + ' mtu 8192'
    op = sendCmd(ch, cmd, '#')
    cmd = 'arp -s ' + dev['if1_peer_ip'] + ' ' + dev['if1_peer_mac']
    op = sendCmd(ch, cmd, '#')
    cmd = 'ping -c 5 ' + dev['if1_peer_ip']
    op = sendCmd(ch, cmd, '#', timeout=20)
    if "5 packets transmitted, 5 received" not in op:
        logging.info("===========================================")
        logging.info("ERROR: Packet loss with ping. Please check")
        logging.info("===========================================")
    else:
        logging.info("Ping passed")

    op = sendCmd(ch, 'modprobe ib_uverbs', '#')
    logging.info("Loading RDMA driver\n")
    op = sendCmd(ch, 'insmod /root/drivers-linux/drivers/rdma/drv/ionic/ionic_rdma.ko xxx_kdbid=1', '#')

    return ch.before

if __name__ == "__main__":
    initLogger()

    parser = argparse.ArgumentParser()

    parser.add_argument('--host', help='hostname of server where NIC is present. e.g. --host cap-rdma')
    parser.add_argument('--update', help='Update the image before reset if 1. e.g. --update 1')

    args=parser.parse_args()

    if (len(sys.argv) == 1):
        parser.print_help()
        exit()
    else:
        logging.info("Reloading naples on {0}. Please wait...\n".format(args.host))

        pwd = getPwd()
        cfg = pwd + '/../conf/dev.json'
        with open(cfg, 'r') as f:
            dev_dict = json.load(f)
        for dev in dev_dict:
            if (dev["host"] == args.host):
                logging.info(dev["host"])
                break

    if (args.update == '1'):
        # verify that the image exists
        image = pwd + '/../naples_fw.tar'
        logging.info("Looking for naples_fw.tar in nic directory\n")
        cmd = 'ls -lrt ' + image
        ret = os.system(cmd)
        if (ret != 0):
            logging.info("ERROR: naples_fw.tar not found in nic directory")
            exit()

        # copy the image to the host    
        logging.info("Copying naples_fw.tar to {0}\n".format(dev['host']))
        cmd = 'sshpass -p docker scp -o StrictHostKeyChecking=no ' + image + ' root@' + dev['host'] + ':/tmp/'
        ret = os.system(cmd)
        if (ret != 0):
            logging.info("ERROR: Failed to copy naples_fw.tar to {0}".format(dev['host']))
            exit()

        # check if memtun is already present on the host
        logging.info("Checking if memtun is already present on {0}\n".format(args.host))
        cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host + ' -t ls -lrt /tmp/memtun'
        ret = os.system(cmd)
        if (ret != 0):
            # copy memtun to the host
            logging.info("Copying memtun to {0}\n".format(args.host))
            cmd = 'sshpass -p docker scp -o StrictHostKeyChecking=no /home/haps/memtun/memtun root@' + args.host + ':/tmp/'
            ret = os.system(cmd)
            if (ret != 0):
                logging.info("ERROR: Failed to copy memtun to {0}".format(args.host))
                exit()
            else:
                logging.info("Copied memtun successfully to {0}".format(args.host))
        else:
            logging.info("memtun is already present on {0}\n".format(args.host))

    ch = sshToHost(args.host)

    if (args.update == '1'):
        update(ch)
    reset(ch, dev)
