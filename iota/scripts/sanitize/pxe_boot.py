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
import defines
from server import Server

import harness.infra.utils.parser as par

from threading import Thread

parser = argparse.ArgumentParser(description='Sanitize Host in test bed file')
# Mandatory parameters
parser.add_argument('--testbed', dest='file_name', required = True, default=None, help='Testbed JSON filename')
parser.add_argument('--os', dest='os', required = True, default=None, help='which OS to boot to')
parser.add_argument('--os-ver', dest='os_ver', required = False, default=None, help='which Version to boot')

GlobalOptions = parser.parse_args()

def __get_date_string(osname):
    x = datetime.now()
    x = x + timedelta(minutes=2)
    year  =  str(x.year)[2:]
    month = "{0:02d}".format(x.month)
    day   = "{0:02d}".format(x.day)
    hour   = "{0:02d}".format(x.hour)
    minute = "{0:02d}".format(x.minute)
    if osname == 'freebsd':
        ret = "%s%s%s%s%s" % (str(year),str(month),str(day),str(hour),str(minute))
    if osname == 'linux':
        ret = ("%s%s%s" % (str(year),str(month),str(day)), "%s%s" % (str(hour),str(minute)))
    return ret

def __init_linux(host):
    cmds = []
    #host.SetUserPassword('vm', 'vm')
    cmds.append("cp /usr/share/zoneinfo/America/Los_Angeles /etc/localtime")
    d = __get_date_string('linux')
    cmds.append("date %s -s %s && date %s -s %s" % ('%y%m%d', d[0], '%H%M', d[1] ))
    cmds.append("hostnamectl  set-hostname %s" % host.ipaddr)
    cmds.append("rm -f /root/.ssh/known_hosts")
    cmds.append("mkdir -p /pensando")
    cmds.append("chown vm:vm /pensando")
    #cmds.append("cat /root/.bashrc | sed s/\'$ \'/\'%s \'/ > /tmp/prompt.txt && cp /tmp/prompt.txt /root/.bashrc && cat /root/.bashrc" % "#")
    cmds.append("echo PS1=\\'\\\\\u@\\\\\H:\\\\\w#\\'  >> /root/.bashrc && cat /root/.bashrc")
    for cmd in cmds:
        host.RunSshCmd(cmd, True)
    pass


def __init_freebsd(host):
    cmds = []
    cmds.append("cp /usr/share/zoneinfo/America/Los_Angeles /etc/localtime")
    cmds.append("date %s" % __get_date_string('freebsd'))
    cmds.append("cat /etc/rc.conf | sed s/FreeBSD/%s/ > /tmp/hostname.txt && cp /tmp/hostname.txt /etc/rc.conf && cat /etc/rc.conf" % host.ipaddr)
    cmds.append("rm -f /root/.ssh/known_hosts")
    cmds.append("mkdir -p /pensando")
    cmds.append("chown vm:vm /pensando")
    for cmd in cmds:
        host.RunSshCmd(cmd, True)
    pass

def __init_node(host, osname):
    if osname == 'freebsd':
        __init_freebsd(host)
    if osname == 'linux':
        __init_linux(host)
    if osname == 'ubuntu':
        __init_linux(host)

def __print_msg(msg):
    print (msg)

PXE_OP_CREATE = 'create'
PXE_OP_DELETE = 'delete'

def __setup_pxe_server(server, operation, macs, osname):
    for mac in macs:
        mac = '01-' + re.sub(':', '-', mac)
        if operation == PXE_OP_CREATE:
            ssh_cmd = 'cd /export/tftpboot/pxelinux.cfg && ln -sf %s %s' % (defines.GetOsPath(osname), mac.lower())
        if operation == PXE_OP_DELETE:
            ssh_cmd = 'cd /export/tftpboot/pxelinux.cfg && rm -f %s' % (mac.lower())

        server.RunSshCmd(ssh_cmd)
    
def __get_defines_osname(osname):
    if osname == 'linux':
        osname = osname + '_' + GlobalOptions.os_ver
    if not defines.CheckPxeSupport(osname):
        raise Exception("Requested OS[%s] Not supported for pxe yet" % osname)
    return osname 

def __pxe_boot(host_spec):
    osname = __get_defines_osname(host_spec.NodeOs)
    
    server = Server(defines.NODE_TYPE_PXE, defines.GetPxeServerIp())
    host   = Server(defines.NODE_TYPE_HOST, host_spec.NodeMgmtIP, host_spec.NodeCimcIP)
    
    macs = host.GetOobMacFromCimc()
    __setup_pxe_server(server, PXE_OP_CREATE, macs, osname)
    
    __print_msg('[%s]: Setting boot device to pxe' % host.ipaddr)
    host.RunIpmi( "chassis bootdev pxe optsions=persistant" )
    __print_msg('[%s]: restarting the host to pxe boot' % host.ipaddr)
    host.RunIpmi( "chassis power reset")
    time.sleep(10)
    host.WaitForReachability("Waiting for host to PXE boot")
    #Change boot dev to local disck
    __print_msg('[%s]: Setting boot device to disk' % host.ipaddr)
    host.RunIpmi( "chassis bootdev disk optsions=persistant" )
    host.WaitForReboot("Waiting for host to Install")
    host.WaitForReachability("Waiting for host to boot from disk after installation")
    __print_msg('[%s]: Setting boot device to disk' % host.ipaddr)
    host.RunIpmi( "chassis bootdev disk optsions=persistant" )
    
    __setup_pxe_server(server, PXE_OP_DELETE, macs, osname)

    host.SetBootOrder("hdd")
    host.WaitForReboot("bios setting changed to hdd: waiting to reboot")
    host.WaitForReachability("bios setting changed to hdd: rebooting")
    
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
