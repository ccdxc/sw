#!/usr/bin/python

import pexpect
import logging
import sys
import os
import argparse
import time
import getpass

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
    ch.expect('Password: ')
    sendCmd(ch, 'docker', '#')
    return ch

def collect(ch, args):
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

    sendCmd(ch, 'ssh -o StrictHostKeyChecking=no root@1.0.0.2', 'root@1.0.0.2\'s password: ')
    sendCmd(ch, 'pen123', '#')

    op = sendCmd(ch, 'ls -lrt /captrace.cfg', '#')
    if "No such file or directory" not in op:
        op = sendCmd(ch, 'rm /captrace.cfg', '#')
    op = sendCmd(ch, 'touch /captrace.cfg', '#')

    cmdlist = []
    if (args.rxdma == '1'):
        cmdlist.append('rxdma,*,*,table_key_enable=1,instr_enable=1,trace_size=8192,wrap=1')

    if (args.txdma == '1'):
        cmdlist.append('txdma,*,*,table_key_enable=1,instr_enable=1,trace_size=8192,wrap=1')

    if (args.p4ig == '1'):
        cmdlist.append('p4ig,*,*,table_key_enable=1,instr_enable=1,trace_size=8192,wrap=1')

    if (args.p4eg == '1'):
        cmdlist.append('p4eg,*,*,table_key_enable=1,instr_enable=1,trace_size=8192,wrap=1')

    for cmd in cmdlist:
        cmd = 'echo ' + cmd + ' >> /captrace.cfg'
        op = sendCmd(ch, cmd, '#')

    op = sendCmd(ch, 'cat /captrace.cfg', '#')

    #disable and reset
    op = sendCmd(ch, '/platform/bin/captrace disable', '#')
    op = sendCmd(ch, '/platform/bin/captrace reset', '#')

    # configure
    op = sendCmd(ch, '/platform/bin/captrace conf /captrace.cfg', '#')

    # enable
    op = sendCmd(ch, '/platform/bin/captrace enable', '#')

    ret = raw_input('\nDo you want to dump captrace now? (y/n)?')
    if (ret != 'y' and ret != 'Y'):
        logging.info('Exiting')
        ch.close()
        exit()

    # dump
    op = sendCmd(ch, '/platform/bin/captrace dump captrace.bin', '#')
        
    logging.info("Copying captrace.bin to {0}".format(args.host))
    sendCmd(ch, 'scp -o StrictHostKeyChecking=no captrace.bin root@1.0.0.1:/tmp/', 'Password: ')
    sendCmd(ch, 'docker', '#')

    logging.info("Copying capri_loader.conf to {0}".format(args.host))
    sendCmd(ch, 'scp -o StrictHostKeyChecking=no /capri_loader.conf root@1.0.0.1:/tmp/', 'Password: ')
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

    args=parser.parse_args()

    if (len(sys.argv) == 1):
        parser.print_help()
        exit()
    else:
        logging.info("Collecting captrace on {0}. Please wait...\n".format(args.host))

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

    cmd = pwd + '/../../platform/src/app/captrace/captrace.py gen_syms'
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("ERROR: failed to generate captrace.syms")
        exit()

    cmd = pwd + '/../../platform/src/app/captrace/captrace.py decode captrace.bin > captrace.decode'
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
            cmd = pwd + '/../../platform/src/app/captrace/captrace.py decode captrace.bin --fltr ' + fltr + ' > captrace.decode.filter'
            logging.info(cmd)
            ret = os.system(cmd)
            if (ret != 0):
                logging.info("ERROR: failed to decode captrace.bin using the provided filter")
                exit()
            else:
                logging.info("Decoded captrace.bin successfully using the filter in captrace.decode.filter")
