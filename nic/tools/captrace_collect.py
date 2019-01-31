#!/usr/bin/python

import pexpect
import logging
import sys
import os
import argparse
import time
import getpass
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

def sshToHost(host):
    logging.info("ssh to {0}".format(host))
    # SSH to the host
    cmd = 'ssh root@' + host
    ch = pexpect.spawn (cmd)
    ch.logfile = sys.stdout
    ch.expect('[Pp]assword.*')
    sendCmd(ch, 'docker', '#')
    return ch

def collect(ch, args):
    sendCmd(ch, 'ssh -o StrictHostKeyChecking=no root@169.254.0.1', '[Pp]assword.*')
    sendCmd(ch, 'pen123', '#')

    op = sendCmd(ch, 'ls -lrt /captrace.cfg', '#')
    if "No such file or directory" not in op:
        op = sendCmd(ch, 'rm /captrace.cfg', '#')
    op = sendCmd(ch, 'touch /captrace.cfg', '#')

    cmdlist = []
    if (args.rxdma == '1'):
        cmdlist.append('rxdma,*,*,table_key_enable=1,instr_enable=1,trace_size=4096,wrap=1')

    if (args.txdma == '1'):
        cmdlist.append('txdma,*,*,table_key_enable=1,instr_enable=1,trace_size=4096,wrap=1')

    if (args.p4ig == '1'):
        cmdlist.append('p4ig,*,*,table_key_enable=1,instr_enable=1,trace_size=4096,wrap=1')

    if (args.p4eg == '1'):
        cmdlist.append('p4eg,*,*,table_key_enable=1,instr_enable=1,trace_size=4096,wrap=1')

    if (args.all == '1'):
        cmdlist.append('*,*,*,table_key_enable=1,instr_enable=1,trace_size=4096,wrap=1')

    for cmd in cmdlist:
        cmd = 'echo ' + cmd + ' >> /captrace.cfg'
        op = sendCmd(ch, cmd, '#')

    op = sendCmd(ch, 'cat /captrace.cfg', '#')

    # reset
    op = sendCmd(ch, '/platform/bin/captrace reset', '#')

    # configure
    op = sendCmd(ch, '/platform/bin/captrace conf /captrace.cfg', '#')

    ret = raw_input('\nDo you want to dump captrace now? (y/n)?')
    if (ret != 'y' and ret != 'Y'):
        logging.info('Exiting')
        ch.close()
        exit()

    # dump
    op = sendCmd(ch, '/platform/bin/captrace dump captrace.bin', '#')
        
    logging.info("Copying captrace.bin to {0}".format(args.host))
    sendCmd(ch, 'scp -o StrictHostKeyChecking=no captrace.bin root@169.254.0.2:/tmp/', '[Pp]assword.*')
    sendCmd(ch, 'docker', '#')

    logging.info("Copying capri_loader.conf to {0}".format(args.host))
    sendCmd(ch, 'scp -o StrictHostKeyChecking=no /capri_loader.conf root@169.254.0.2:/tmp/', '[Pp]assword.*')
    sendCmd(ch, 'docker', '#')

    ch.close()
    return

if __name__ == "__main__":
    initLogger()

    parser = argparse.ArgumentParser()

    parser.add_argument('--host', help='hostname of server where NIC is present. e.g. --host cap-rdma')
    parser.add_argument('--rxdma', help='enable captrace for RxDMA')
    parser.add_argument('--txdma', help='enable captrace for TxDMA')
    parser.add_argument('--p4ig', help='enable captrace for P4 ingress')
    parser.add_argument('--p4eg', help='enable captrace for P4 egress')
    parser.add_argument('--all', help='enable captrace for all pipelines')

    args=parser.parse_args()

    if (len(sys.argv) == 1):
        parser.print_help()
        exit()
    else:
        logging.info("Collecting captrace on {0}. Please wait...\n".format(args.host))

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

    # verify that ping works between arm and host thru mnic intf
    mnic_intf = dev['if2']
    cmd = 'sshpass -p docker ssh -o StrictHostKeyChecking=no root@' + args.host + ' -t ping -c 5 169.254.0.1'
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("Ping failed between arm and host")

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

    ch = sshToHost(args.host)

    collect(ch, args)

    pwd = getPwd()
    ws = pwd + '/../'
    # copy captrace.bin
    logging.info("Copying captrace.bin from {0} to the workspace\n".format(args.host))
    cmd = 'sshpass -p docker scp -o StrictHostKeyChecking=no root@' + args.host + ':/tmp/captrace.bin ' + ws
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("ERROR: failed to copy captrace.bin from {0}".format(args.host))
        exit()

    # copy capri_loader.conf
    logging.info("Copying capri_loader.conf from {0} to the workspace\n".format(args.host))
    cmd = 'sshpass -p docker scp -o StrictHostKeyChecking=no root@' + args.host + ':/tmp/capri_loader.conf ' + ws
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("ERROR: failed to copy capri_loader.conf from {0}".format(args.host))
        exit()

    cmd = pwd + '/../sdk/platform/mputrace/captrace.py gen_syms'
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("ERROR: failed to generate captrace.syms")
        exit()

    cmd = pwd + '/../sdk/platform/mputrace/captrace.py decode captrace.bin > captrace.decode'
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("ERROR: failed to decode captrace.bin")
        exit()

    logging.info("Decoded captrace.bin successfully in captrace.decode")

    while (True):
        ret = raw_input('\nDo you want to filter the captrace decode? (y/n)')
        if (ret != 'y' and ret != 'Y'):
            logging.info('Exiting')
            exit()
        else:
            fltr = raw_input('\nProvide the filter to be applied. e.g. mpu_processing_pkt_id_next=0xaf\n')
            cmd = pwd + '/../sdk/platform/mputrace/captrace.py decode captrace.bin --fltr ' + fltr + ' > captrace.decode.filter'
            logging.info(cmd)
            ret = os.system(cmd)
            if (ret != 0):
                logging.info("ERROR: failed to decode captrace.bin using the provided filter")
                exit()
            else:
                logging.info("Decoded captrace.bin successfully using the filter in captrace.decode.filter")
