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

def sshToHost(ip, host):
    logging.info("ssh to {0}".format(ip))
    # SSH to the host/naples
    cmd = 'ssh -o "StrictHostKeyChecking no" root@' + ip
    ch = pexpect.spawn (cmd)
    ch.logfile = sys.stdout
    ch.expect('[Pp]assword.*')
    if host is True:
        sendCmd(ch, 'docker', '#')
    else:
        sendCmd(ch, 'pen123', '#')
    return ch

def collectAndDump(ch, args):
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
    op = sendCmd(ch, '/platform/bin/captrace dump /tmp/captrace.bin', '#')

    if args.host is not None:
        ip = args.host
        logging.info("Copying captrace.bin to {0}".format(args.host))
        sendCmd(ch, 'scp -o StrictHostKeyChecking=no /tmp/captrace.bin root@169.254.0.2:/tmp/', '[Pp]assword.*')
        sendCmd(ch, 'docker', '#')
        logging.info("Copying mpu_prog_info.json to {0}".format(args.host))
        sendCmd(ch, 'scp -o StrictHostKeyChecking=no /nic/conf/gen/mpu_prog_info.json root@169.254.0.2:/tmp/', '[Pp]assword.*')
        sendCmd(ch, 'docker', '#')
    if args.mgmt is not None:
        logging.info("Copying mpu_prog_info.json to /tmp/")
        sendCmd(ch, 'rm -f /tmp/mpu_prog_info.json', '#')
        sendCmd(ch, 'cp /nic/conf/gen/mpu_prog_info.json /tmp/', '#')

def collectOnDevice(ch, args):
    sendCmd(ch, 'ssh -o StrictHostKeyChecking=no root@169.254.0.1', '[Pp]assword.*')
    sendCmd(ch, 'pen123', '#')

def processHost(args):
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

    logging.info("Copying captrace.cfg to {0}".format(args.host))
    conf = pwd + '/../conf/captrace/*'
    cmd = 'sshpass -p docker scp -o StrictHostKeyChecking=no ' + conf + ' root@' + args.host + ':/tmp/'
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("ERROR: failed to copy captrace.cfg to {0}".format(args.host))
        exit()

def collectHost(ch, args):
    sendCmd(ch, 'ssh -o StrictHostKeyChecking=no root@169.254.0.1', '[Pp]assword.*')
    sendCmd(ch, 'pen123', '#')

    op = sendCmd(ch, 'ls -lrt /captrace.cfg', '#')
    if "No such file or directory" not in op:
        op = sendCmd(ch, 'rm /captrace.cfg', '#')
    op = sendCmd(ch, 'touch /captrace.cfg', '#')

    if args.rxdma is True:
        sendCmd(ch, 'scp -o StrictHostKeyChecking=no root@169.254.0.2:/tmp/rxdma-all.json /captrace.cfg', '[Pp]assword.*')
        sendCmd(ch, 'docker', '#')

    if args.txdma is True:
        sendCmd(ch, 'scp -o StrictHostKeyChecking=no root@169.254.0.2:/tmp/txdma-all.json /captrace.cfg', '[Pp]assword.*')
        sendCmd(ch, 'docker', '#')

    if args.p4ig is True:
        sendCmd(ch, 'scp -o StrictHostKeyChecking=no root@169.254.0.2:/tmp/p4ig-all.json /captrace.cfg', '[Pp]assword.*')
        sendCmd(ch, 'docker', '#')

    if args.p4eg is True:
        sendCmd(ch, 'scp -o StrictHostKeyChecking=no root@169.254.0.2:/tmp/p4eg-all.json /captrace.cfg', '[Pp]assword.*')
        sendCmd(ch, 'docker', '#')

    if args.dma_all is True:
        sendCmd(ch, 'scp -o StrictHostKeyChecking=no root@169.254.0.2:/tmp/dma-all.json /captrace.cfg', '[Pp]assword.*')
        sendCmd(ch, 'docker', '#')

    if args.p4_all is True:
        sendCmd(ch, 'scp -o StrictHostKeyChecking=no root@169.254.0.2:/tmp/p4-all.json /captrace.cfg', '[Pp]assword.*')
        sendCmd(ch, 'docker', '#')

    if args.all is True:
        sendCmd(ch, 'scp -o StrictHostKeyChecking=no root@169.254.0.2:/tmp/pipeline-all.json /captrace.cfg', '[Pp]assword.*')
        sendCmd(ch, 'docker', '#')

def collectNaples(ch, args):
    op = sendCmd(ch, 'ls -lrt /captrace.cfg', '#')
    if "No such file or directory" not in op:
        op = sendCmd(ch, 'rm /captrace.cfg', '#')
    op = sendCmd(ch, 'touch /captrace.cfg', '#')

    if args.rxdma is True:
        sendCmd(ch, 'cp /nic/conf/captrace/rxdma-all.json /captrace.cfg', '#')

    if args.txdma is True:
        sendCmd(ch, 'cp /nic/conf/captrace/txdma-all.json /captrace.cfg', '#')

    if args.p4ig is True:
        sendCmd(ch, 'cp /nic/conf/captrace/p4ig-all.json /captrace.cfg', '#')

    if args.p4eg is True:
        sendCmd(ch, 'cp /nic/conf/captrace/p4eg-all.json /captrace.cfg', '#')

    if args.dma_all is True:
        sendCmd(ch, 'cp /nic/conf/captrace/dma-all.json /captrace.cfg', '#')

    if args.p4_all is True:
        sendCmd(ch, 'cp /nic/conf/captrace/p4-all.json /captrace.cfg', '#')

    if args.all is True:
        sendCmd(ch, 'cp /nic/conf/captrace/pipeline-all.json /captrace.cfg', '#')

def collect(ch, args):
    if args.host is not None:
        collectHost(ch, args)
    elif args.mgmt is not None:
        collectNaples(ch, args)
    op = sendCmd(ch, 'cat /captrace.cfg', '#')

    collectAndDump(ch, args)

    ch.close()
    return

if __name__ == "__main__":
    initLogger()

    parser = argparse.ArgumentParser()

    parser.add_argument('--host', help='hostname of server where NIC is present. e.g. --host cap-rdma')
    parser.add_argument('--mgmt', help='management ip for NAPLES device')
    parser.add_argument('--pipeline', help='specify iris|apollo|artemis|apulu pipeline for captrace.syms. pipeline is iris by default if nothing is provided')
    parser.add_argument('--rxdma', action='store_true', help='enable captrace for RxDMA')
    parser.add_argument('--txdma', action='store_true',  help='enable captrace for TxDMA')
    parser.add_argument('--p4ig', action='store_true',  help='enable captrace for P4 ingress')
    parser.add_argument('--p4eg', action='store_true',  help='enable captrace for P4 egress')
    parser.add_argument('--dma_all', action='store_true',  help='enable captrace for RxDMA and TxDMA')
    parser.add_argument('--p4_all', action='store_true',  help='enable captrace for P4 ingress and egress')
    parser.add_argument('--all', action='store_true',  help='enable captrace for all pipelines')

    args=parser.parse_args()

    if (len(sys.argv) == 1):
        parser.print_help()
        exit()

    if args.host is not None and args.mgmt is not None:
        print("Error: Please provide either --host or --mgmt option.")
        parser.print_help()
        print("\neg: captrace_collect.py --mgmt 192.168.70.157 --rxdma")
        exit()

    if args.host is not None:
        ip = args.host
    elif args.mgmt is not None:
        ip = args.mgmt
    else:
        parser.print_help()
        exit()

    if args.rxdma is False and args.txdma is False and args.p4ig is False and \
       args.p4eg is False and args.dma_all is False and args.p4_all is False \
       and args.all is False:
        print("Error: Please provide any one of --rxdma, --txdma, --p4ig, --p4eg, --dma_all, --p4_all, --all options.")
        parser.print_help()
        print("\neg: captrace_collect.py --mgmt 192.168.70.157 --rxdma")
        exit()

    global g_pipeline
    if args.pipeline is None:
        g_pipeline = 'iris'
    else:
        if args.pipeline != 'iris' and args.pipeline != 'apollo' and args.pipeline != 'artemis' and args.pipeline != 'apulu':
            print("Error: Invalid argument for --pipeline. Please provide one of - iris|apollo|artemis|apulu")
            parser.print_help()
            print("\neg: captrace_collect.py --mgmt 192.168.70.157 --pipeline apollo --rxdma")
            exit()
        g_pipeline = args.pipeline

    logging.info("Collecting captrace on {0}. Please wait...\n".format(ip))

    if args.host is not None:
        processHost(args)

    if args.host is not None:
        ip = args.host
        host = True
    elif args.mgmt is not None:
        ip = args.mgmt
        host = False
    ch = sshToHost(ip, host)

    collect(ch, args)

    pwd = getPwd()
    ws = pwd + '/../'
    # copy captrace.bin
    if args.host is not None:
        ip = args.host
        password = 'docker'
    elif args.mgmt is not None:
        ip = args.mgmt
        password = 'pen123'
    logging.info("Copying captrace.bin from {0} to the workspace\n".format(ip))
    cmd = 'sshpass -p ' + password + ' scp -o StrictHostKeyChecking=no root@' + ip + ':/tmp/captrace.bin ' + ws
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("ERROR: failed to copy captrace.bin from {0}".format(ip))
        exit()

    # copy mpu_prog_info.json
    logging.info("Copying mpu_prog_info.json from {0} to the workspace\n".format(ip))
    cmd = 'sshpass -p ' + password + ' scp -o StrictHostKeyChecking=no root@' + ip + ':/tmp/mpu_prog_info.json ' + ws
    ret = os.system(cmd)
    if (ret != 0):
        logging.info("ERROR: failed to copy mpu_prog_info.json from {0}".format(ip))
        exit()
    cmd = pwd + '/../../../sdk/platform/mputrace/captrace.py gen_syms --pipeline=%s' % g_pipeline
    os.chdir(pwd + '/../../../')
    ret = os.system(cmd)
    os.chdir(pwd)
    if (ret != 0):
        logging.info("ERROR: failed to generate captrace.syms")
        exit()

    cmd = pwd + '/../../../sdk/platform/mputrace/captrace.py decode captrace.bin > captrace.decode'
    os.chdir(pwd + '/../../../')
    ret = os.system(cmd)
    os.chdir(pwd)
    if (ret != 0):
        logging.info("ERROR: failed to decode captrace.bin")
        exit()

    logging.info("Decoded captrace.bin successfully in ../../../captrace.decode")

    while (True):
        ret = raw_input('\nDo you want to filter the captrace decode? (y/n)')
        if (ret != 'y' and ret != 'Y'):
            logging.info('Exiting')
            exit()
        else:
            fltr = raw_input('\nProvide the filter to be applied. e.g. mpu_processing_pkt_id_next=0xaf\n')
            os.chdir(pwd + '/../../../')
            cmd = pwd + '/../../../sdk/platform/mputrace/captrace.py decode captrace.bin --fltr ' + fltr + ' > captrace.decode.filter'
            logging.info(cmd)
            ret = os.system(cmd)
            os.chdir(pwd)
            if (ret != 0):
                logging.info("ERROR: failed to decode captrace.bin using the provided filter")
                exit()
            else:
                logging.info("Decoded captrace.bin successfully using the filter in ../../../captrace.decode.filter")
