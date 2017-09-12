#!/usr/bin/python

# run VMs using vsimctl command line tool

import time
import sys
import os
import argparse
import paramiko
import threading
import json
import exceptions
import traceback
from multiprocessing.dummy import Pool as ThreadPool

# Utility function to run ssh
def ssh_exec_thread(ssh_object, command):
    print "run: " + command
    stdin, stdout, stderr = ssh_object.exec_command(command)
    out = stdout.readlines()
    print out
    print "Program exited: " + command
    exitCode = stdout.channel.recv_exit_status()
    if exitCode != 0:
        print "Exit code: " + str(exitCode)

# This class represents a vagrant node
class Node:
    def __init__(self, addr, nidx, username='vagrant', password='vagrant', gobin='/import/bin'):
        self.addr = addr
        self.username = username
        self.password = password
        self.gobin = gobin
        self.nidx = nidx
        self.ssh = self.sshConnect(username, password)
        out, err, ec = self.runCmd("hostname")
        self.hostname = out[0].split('\n')[0]
        print "Connected to " + self.hostname

    # Connect to vagrant node
    def sshConnect(self, username, password):
        ssh_object = paramiko.SSHClient()
        ssh_object.set_missing_host_key_policy( paramiko.AutoAddPolicy() )
        print "Connecting to " + self.addr + " with userid: " + username + " password: " + password
        try:
            ssh_object.connect(self.addr, username=username, password=password)
            return ssh_object
        except paramiko.ssh_exception.AuthenticationException:
            tutils.exit("Authentication failed")

    def isConnected(self):
        transport = self.ssh.get_transport() if self.ssh else None
        return transport and transport.is_active()

    # Run a command on vagrant node
    def runCmd(self, cmd, timeout=None):
        try:
            print "run: " + cmd
            # We we disconnected for any reason, reconnect
            if not self.isConnected():
                self.ssh = self.sshConnect(self.username, self.password)

            # Execute the command
            stdin, stdout, stderr = self.ssh.exec_command(cmd, timeout=timeout)
            out = stdout.readlines()
            err = stderr.readlines()
            exitCode = stdout.channel.recv_exit_status()
            if out != [] or exitCode != 0:
                print "stdout(" + str(exitCode) + "):" + ''.join(out)
            if err != []:
                print "stderr: " + ''.join(err)
            return out, err, exitCode
        except exceptions.EOFError:
            print "Ignoring EOF errors executing command"
            return [], [], 0


# Parse command line args
# Create the parser and sub parser
parser = argparse.ArgumentParser()
parser.add_argument('--version', action='version', version='1.0.0')
parser.add_argument("-hosts", default=3, help="number of hosts")
parser.add_argument("-vcsim", default='http://pen-master:18086', help="vcsim URL")
parser.add_argument("-vms", default=3, help="number of VMs to run")
parser.add_argument("-user", default='vagrant', help="User id for ssh")
parser.add_argument("-password", default='vagrant', help="password for ssh")
parser.add_argument("-gobin", default='/import/bin', help="$GOPATH/bin directory path")

# Parse the args
args = parser.parse_args()

# warn if number of VMs is not a multiple of number of hosts
if (int(args.vms) % int(args.hosts)) != 0:
    numVms = (int(args.vms) / int(args.hosts)) * int(args.hosts)
    print "### Warning: number of VMs is not a multiple of number of hosts. Starting " + str(numVms) + " VMs"

try:
    # connect to master and create VMs
#    vcsim = Node("pen-master", 0, args.user, args.password, args.gobin)
#    for vmidx in range(int(args.vms)):
#        hostidx = vmidx % int(args.hosts)
#        ipidx = vmidx / int(args.hosts)
#        vcsim.runCmd("vsimctl -create  -hostmac 02:02:02:02:02:" + '{:02x}'.format(hostidx+1) + " -ip 10.1." + str(ipidx+1) + "." + str(hostidx+1) + "/16 -vlan " + str(ipidx+1) + " -net default")

    nodes = []
    for nidx in range(int(args.hosts)):
        node = Node("pen-master", nidx, args.user, args.password, args.gobin)
        nodes.append(node)

    vmPerHost = int(args.vms) / int(args.hosts)
    for cidx in range(vmPerHost):
        pool = ThreadPool(int(args.hosts))
        pool.map(lambda node: node.runCmd("vsimctl -create  -hostmac 02:02:02:02:02:" + '{:02x}'.format(node.nidx+1) + " -ip 10.1." + str(cidx+1) + "." + str(node.nidx+1) + "/16 -vlan " + str(cidx+1) + " -net default"), nodes)


    print "################### Started VMs #####################"

except Exception, err:
    print "ERROR while running the script: "
    print err
    traceback.print_exc()
    os._exit(1)

time.sleep(1)
os._exit(0)
