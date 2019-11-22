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
    
    # expect either of two prompts
    res = tn.expect(["Username: ", "Password: "], timeout=5)
    if (res[0] == 0):
        sendTelnetCmd(tn, 'admin\n', ['Password: '])

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

def copyDrivers(dev, hostOS):
    # check if the drivers tar file is already present
    pwd = getPwd()
    if hostOS == 'Linux':
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

    elif hostOS == 'FreeBSD':
        drivers = pwd + '/../../platform/gen/drivers-freebsd.tar.xz'
        cmd = 'ls -lrt ' + drivers
        ret = os.system(cmd)
        if (ret != 0):
            logging.info("drivers tar file is not present in the workspace. Generating it")
            cmd = pwd + '/../platform/tools/package-freebsd.sh'
            ret = os.system(cmd)
            if (ret != 0):
                logging.info("ERROR: Failed to get the drivers tar file")
                exit()

    time.sleep(3)
    # copy the drivers to the host
    logging.info("Copying drivers tar file to {0}\n".format(dev['host']))
    cmd = 'sshpass -p docker scp -o StrictHostKeyChecking=no ' + drivers + ' root@' + dev['host'] + ':/tmp/'
    logging.info(cmd)
    ret = os.system(cmd)
    logging.info("ret is {0}".format(ret))
    if (ret != 0):
        logging.info("ERROR: Failed to copy drivers tar file to {0}".format(dev['host']))
        exit()

def update(ch):
    logging.info("Removing known_hosts")
    op = sendCmd(ch, 'rm /root/.ssh/known_hosts', '#')

    logging.info("Looking for naples_fw.tar in /tmp\n")
    op = sendCmd(ch, 'ls -lrt /tmp/naples_fw.tar', '#')
    if "No such file or directory" in op:
        logging.info("ERROR: /tmp/naples_fw.tar not found on the host")
        exit()

    logging.info("Copying naples_fw.tar to naples\n")
    sendCmd(ch, 'scp -o StrictHostKeyChecking=no /tmp/naples_fw.tar root@169.254.0.1:/update/', '[Pp]assword.*')
    sendCmd(ch, 'pen123', '#')

    sendCmd(ch, 'ssh -o StrictHostKeyChecking=no root@169.254.0.1', '[Pp]assword.*')
    sendCmd(ch, 'pen123', '#')

    logging.info("Updating naples_fw.tar on naples\n")
    op = sendCmd(ch, '/nic/tools/sysupdate.sh -p /update/naples_fw.tar', '#', timeout=120)
    sendCmd(ch, 'exit', '#')

def reset(ch, dev, hostOS):
    logging.info("Rebooting the host {0}\n".format(dev['host']))
    ch.sendline('reboot')

    logging.info("Waiting for the host to be back up...\n")
    time.sleep(30)
    cmd = 'ping -c 5 ' + dev['host']
    retry_cnt = 1
    while (retry_cnt <= 50):
        ret = os.system(cmd)
        if (ret != 0):
            logging.info("The host {0} is still not up. Retry cnt {1}".format(dev['host'], retry_cnt))
            retry_cnt += 1
        else:
            logging.info("The host {0} is back up!".format(dev['host']))
            break

    ch.close()

    if (retry_cnt > 50):
        logging.info("ERROR: Timeout. Host is not reachable after 50 retries")
        exit()

    copyDrivers(dev, hostOS)

    ch = sshToHost(dev['host'])
    loadDrivers(ch, dev, hostOS)
    ch.logfile = None
    ch.close()

    mnic_intf = dev['if2']
    # try to setup MNIC interface
    if hostOS == 'Linux':
        cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + dev['host'] + ' -t ip addr add 169.254.0.2/24 dev ' + mnic_intf
        ret = os.system(cmd)
        if (ret != 0):
            logging.info("Failed to assign IP to MNIC interface")

        cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + dev['host'] + ' -t ifconfig ' + mnic_intf + ' up'
        ret = os.system(cmd)
        if (ret != 0):
            logging.info("Failed to bring up MNIC interface")

    elif hostOS == 'FreeBSD':
        cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + dev['host'] + ' -t ifconfig ' + mnic_intf + ' 169.254.0.2/24 up'
        ret = os.system(cmd)
        if (ret != 0):
            logging.info("Failed to assign IP to and/or bring up MNIC interface")

    logging.info("\nReloaded naples successfully\n")
    exit()

def sshToHost(host):
    logging.info("ssh to {0}".format(host))
    # SSH to the host
    cmd = 'ssh root@' + host
    ch = pexpect.spawn (cmd)
    ch.logfile = sys.stdout
    ch.expect('[Pp]assword.*')
    sendCmd(ch, 'docker', '#')
    return ch

def loadDrivers(ch, dev, hostOS):
    logging.info("Installing drivers")

    # verify that the drivers tar file is present on the host
    sendCmd(ch, 'cd /tmp', '#')

    if hostOS == 'Linux':
        op = sendCmd(ch, 'ls -lrt drivers-linux.tar.xz', '#')
        if "No such file or directory" in op:
            logging.info("ERROR: drivers tar file not found on the host")
            exit()

        sendCmd(ch, 'tar xfm drivers-linux.tar.xz', '#')
        sendCmd(ch, 'cd drivers-linux', '#')
        sendCmd(ch, './build.sh', '#', timeout=60)
        time.sleep(30)
        sendCmd(ch, 'insmod drivers/eth/ionic/ionic.ko', '#')
        time.sleep(5)
        sendCmd(ch, 'modprobe ib_uverbs', '#')
        time.sleep(1)
        sendCmd(ch, 'insmod drivers/rdma/drv/ionic/ionic_rdma.ko', '#')
        time.sleep(5)

    elif hostOS == 'FreeBSD':
        op = sendCmd(ch, 'ls -lrt drivers-freebsd.tar.xz', '#')
        if "No such file or directory" in op:
            logging.info("ERROR: drivers tar file not found on the host")
            exit()

        sendCmd(ch, 'tar xfm drivers-freebsd.tar.xz', '#')
        sendCmd(ch, 'cd drivers-freebsd', '#')
        sendCmd(ch, './build.sh', '#', timeout=60)
        time.sleep(30)
        sendCmd(ch, 'kldload sys/modules/ionic/ionic.ko', '#')
        time.sleep(10)
        sendCmd(ch, 'kldload sys/modules/ionic_rdma/ionic_rdma.ko', '#')
        time.sleep(1)

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
                    logging.info("Failed to assign IP to MNIC interface. Copying drivers")
                    # load eth driver, so that MNIC interface can be configured
                    copyDrivers(dev, hostOS)
                    ch = sshToHost(dev['host'])
                    loadDrivers(ch, dev, hostOS)
                    ch.close()

                    # try to setup MNIC interface
                    ret = os.system(cmd)
                    if (ret != 0):
                        logging.info("ERROR: Failed to assign IP to MNIC interface. Copying drivers")
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
                    logging.info("ERROR: Failed to assign IP to and/or bring up MNIC interface. Copying drivers")
                    # load eth driver, so that MNIC interface can be configured
                    copyDrivers(dev, hostOS)
                    ch = sshToHost(dev['host'])
                    loadDrivers(ch, dev, hostOS)
                    ch.close()

                    # try to setup MNIC interface
                    ret = os.system(cmd)
                    if (ret != 0):
                        logging.info("ERROR: Failed to assign IP to MNIC interface. Copying drivers")
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

    ch = sshToHost(args.host)

    if (args.update == '1'):
        update(ch)
    reset(ch, dev, hostOS)
