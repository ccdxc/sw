#! /usr/bin/python3
import argparse
import sys
import os
import re
import time
from datetime import datetime
from datetime import timedelta

top_dir = os.path.dirname(sys.argv[0])
sys.path.insert(0,'%s' % top_dir)
sys.path.insert(0,'../../')
import helpers
from helpers import *
from server import Server

import harness.infra.utils.parser as par

from threading import Thread

parser = argparse.ArgumentParser(description='Sanitize Host in test bed file')
# Mandatory parameters
parser.add_argument('--testbed', dest='file_name', required = True, default=None, help='Testbed JSON filename')
parser.add_argument('--os', dest='os', required = True, default=None, help='which OS to boot to')

GlobalOptions = parser.parse_args()

INITFILE = "./init.config"

def __get_date_command(osname):
    x = datetime.now()
    x = x + timedelta(minutes=2)
    year  =  str(x.year)[2:]
    month = "{0:02d}".format(x.month)
    day   = "{0:02d}".format(x.day)
    hour   = "{0:02d}".format(x.hour)
    minute = "{0:02d}".format(x.minute)
    if 'freebsd' in osname.lower():
        d = "%s%s%s%s%s" % (str(year),str(month),str(day),str(hour),str(minute))
        cmd  = "date %s" % (d)
    if 'linux' in osname.lower():
        d = ("%s%s%s" % (str(year),str(month),str(day)), "%s%s" % (str(hour),str(minute)))
        cmd  = "date %s -s %s && date %s -s %s" % ('%y%m%d', d[0], '%H%M', d[1] ) 
    return cmd

def __get_prompt_settings_command(prompt):
    return "echo PS1=\\'\\\\\u@\\\\\H:\\\\\w#\\'  >> /root/.bashrc && cat /root/.bashrc"

def __get_hostname_command(osname, hostname):
    if 'freebsd' in osname.lower():
        return ("cat /etc/rc.conf | sed s/FreeBSD/%s/ > /tmp/hostname.txt && cp /tmp/hostname.txt /etc/rc.conf && cat /etc/rc.conf" % hostname)
    if 'linux' in osname.lower():
        return ("hostnamectl  set-hostname %s" % hostname)

def __init_node(host, osname):
    #read from file and operate per os params later 
    config  = helpers.JsonToDict(INITFILE)
    cmds = config['cmds']
    for key, value in config.items():
        if key == 'cmds':
            continue
        if key == 'hostname':
            cmds.append(__get_hostname_command(osname, host.ipaddr))
        if key == "prompt":
            cmds.append(__get_prompt_settings_command(osname))
        if key == "date":
            cmds.append(__get_date_command(osname))
    for cmd in cmds:
        host.RunSshCmd(cmd)

def __print_msg(msg):
    print (msg)

def __pxe_boot(host_spec):
    osname = GlobalOptions.os
    
    server = Server(helpers.NODE_TYPE_PXE, PxeServerHelper.GetServerIp())
    host   = Server(helpers.NODE_TYPE_HOST, host_spec.NodeMgmtIP, host_spec.NodeCimcIP)
    macs = CimcHelper.GetOobMacFromCimc(host)
    PxeServerHelper.SetupPxeServer(server, PxeServerHelper.PXE_OP_CREATE, macs, osname)
   
    __print_msg('[%s]: Setting boot device to pxe' % host.ipaddr)
    HostHelper.RunIpmi(host, "chassis bootdev pxe optsions=persistant" )
    __print_msg('[%s]: restarting the host to pxe boot' % host.ipaddr)
    HostHelper.RunIpmi(host, "chassis power reset")
    time.sleep(10)
    HostHelper.WaitForReachability(host, "Waiting for host to PXE boot")
    #Change boot dev to local disck
    __print_msg('[%s]: Setting boot device to disk' % host.ipaddr)
    HostHelper.RunIpmi(host, "chassis bootdev disk optsions=persistant" )
    HostHelper.WaitForReboot(host, "Waiting for host to Install")
    HostHelper.WaitForReachability(host, "Waiting for host to boot from disk after installation")
    __print_msg('[%s]: Setting boot device to disk' % host.ipaddr)
    HostHelper.RunIpmi(host, "chassis bootdev disk optsions=persistant" )
    PxeServerHelper.SetupPxeServer(server, PxeServerHelper.PXE_OP_DELETE, macs, osname)
    CimcHelper.SetBootOrder(host, "hdd")
    HostHelper.WaitForReboot(host, "bios setting changed to hdd: waiting to reboot")
    HostHelper.WaitForReachability(host, "bios setting changed to hdd: rebooting")
    time.sleep(5)
    # Initialize few things on node for iota run
    __init_node(host, host_spec.NodeOs)

    # Validation to check os version is correct
    #host.ShowVersion()

def __get_hosts(filename):
    return par.JsonParse(filename).Instances

from multiprocessing.dummy import Pool as ThreadPool 
def Main():
    '''
    Assumptions:
    Bringup all the hosts with os in argument via PXE boot.
    Run iota on this TB for basic connectivity check and conclude its good to go.
    '''
    PxeServerHelper.InitServerInfo()
    hosts = __get_hosts(GlobalOptions.file_name)
    #PXE Boot the servers and make sure this are up
    run_parallel = True
    if not run_parallel: 
        for host in hosts:
            results = __pxe_boot(host)
    else:
        pool = ThreadPool(len(hosts))
        results = pool.map(__pxe_boot, hosts)
    print ("System Ready: Please start iota")

if __name__ == '__main__':
    try: 
        Main()
    except Exception as ex:
        sys.stderr.write(str(ex))
        sys.exit(1)
    sys.exit(os.EX_OK)
