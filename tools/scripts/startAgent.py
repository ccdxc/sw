#!/usr/bin/python

# Start pensando agent on all nodes

import time
import sys
import os
import argparse
import paramiko
import threading

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
    def __init__(self, addr, username='vagrant', password='vagrant', gopath='/import/'):
        self.addr = addr
        self.username = username
        self.password = password
        self.gopath = gopath
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

    # Start agent process on vagrant node
    def startAgent(self, args=""):
        ssh_object = self.sshConnect(self.username, self.password)
        command = "sudo " + self.gopath + "/bin/agent " + args + "> /tmp/pensando-agent.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()


# Parse command line args
# Create the parser and sub parser
parser = argparse.ArgumentParser()
parser.add_argument('--version', action='version', version='1.0.0')
parser.add_argument("-nodes", required=True, help="list of nodes(comma separated)")
parser.add_argument("-user", default='vagrant', help="User id for ssh")
parser.add_argument("-password", default='vagrant', help="password for ssh")
parser.add_argument("-gopath", default='/import', help="GOPATH directory path")

# Parse the args
args = parser.parse_args()
addrList = args.nodes.split(",")

# Basic error checking
if len(addrList) < 1:
    print "Empty address list"
    sys.exit(1)

#Start the agent
nodes = []



# Connect to nodes
for addr in addrList:
    node = Node(addr, args.user, args.password, args.gopath)
    nodes.append(node)

    # cleanup any old agent instances still running
    node.runCmd("sudo pkill agent")
    node.runCmd("/usr/sbin/ifconfig -a | grep -e vport | awk '{print $1}' | xargs -r -n1 -I{} sudo ip link delete {} type veth")

    # Copy conf and binary files for CNI plugin
    node.runCmd("sudo cp " + node.gopath + "/src/github.com/pensando/sw/agent/plugins/k8s/cni/pensando-net/01-pensando.conf /etc/cni/net.d/")
    node.runCmd("sudo cp " + node.gopath + "/bin/pensando-net /opt/cni/bin/")

    # create directory for .sock files and remove any stale .sock files
    node.runCmd("sudo mkdir -p /run/pensando/")
    node.runCmd("sudo rm /run/pensando/pensando-cni.sock")

    # Start pensando agent
    node.startAgent()

print "################### Waiting for agent to come up ###################"
time.sleep(15)

print "################### Started Pensando Agent #####################"
os._exit(0)
