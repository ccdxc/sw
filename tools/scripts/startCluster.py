#!/usr/bin/python

# Start pensando agent on all nodes

import time
import sys
import os
import argparse
import paramiko
import threading
import json
import http
import exceptions
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
    def __init__(self, addr, username='vagrant', password='vagrant', gopath='/import/'):
        self.debug = False
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
            if self.debug:
                print "run: " + cmd
            # We we disconnected for any reason, reconnect
            if not self.isConnected():
                self.ssh = self.sshConnect(self.username, self.password)

            # Execute the command
            stdin, stdout, stderr = self.ssh.exec_command(cmd, timeout=timeout)
            out = stdout.readlines()
            err = stderr.readlines()
            exitCode = stdout.channel.recv_exit_status()
            if self.debug:
                if out != [] or exitCode != 0:
                    print "stdout(" + str(exitCode) + "):" + ''.join(out)
                if err != []:
                    print "stderr: " + ''.join(err)
            return out, err, exitCode
        except exceptions.EOFError:
            print "Ignoring EOF errors executing command"
            return [], [], 0

    # Stop pensando cluster services on each node
    def startCluster(self, args=""):
        print "#### Starting pen-base container on " + self.addr
        self.runCmd("docker load -i /import/bin/pen.tar")
        self.runCmd("docker run --privileged --net=host --name pen-base -v /usr/pensando/bin:/host/usr/pensando/bin -v /usr/lib/systemd/system:/host/usr/lib/systemd/system -v /var/run/dbus:/var/run/dbus -v /run/systemd:/run/systemd  -v /etc/systemd/system:/etc/systemd/system  -v /etc/pensando:/etc/pensando -v /etc/kubernetes:/etc/kubernetes -v /sys/fs/cgroup:/sys/fs/cgroup:ro -d pen-base")

    # Start pen base container on each node
    def stopCluster(self, args=""):
        print "#### Stopping pensando services on " + self.addr

        # stop all services
        penSrvs = [ "pen-base", "pen-apiserver", "pen-apigw", "pen-etcd", "pen-kube-controller-manager", "pen-kube-scheduler", "pen-kube-apiserver", "pen-elasticsearch" ]
        for srv in penSrvs:
            self.runCmd("sudo systemctl stop " + srv)
            self.runCmd("docker stop " + srv)
            self.runCmd("docker rm -f " + srv)

        #stop kubelet
        self.runCmd("sudo systemctl stop pen-kubelet")
        self.runCmd("bash -c 'for i in $(/usr/bin/systemctl list-unit-files --no-legend --no-pager -l | grep --color=never -o kube.*\.slice );do echo $i; systemctl stop $i ; done' ")
        self.runCmd("""bash -c 'if [ "$(docker ps -qa)" != "" ] ; then docker stop $(docker ps -qa); docker rm $(docker ps -qa); fi' """)

        self.runCmd("sudo rm -fr /etc/pensando/* /etc/kubernetes/* /usr/pensando/bin/* /var/lib/pensando/* /var/log/pensando/*  /var/lib/cni/ /var/lib/kubelet/* /etc/cni/ ")
        self.runCmd("sudo ip addr flush dev eth1 label *pens")

# Inits cluster by posting a message to CMD
def initCluster(nodeAddr):
    postUrl = 'http://' + nodeAddr + ':9001/api/v1/cluster'

    # cluster json parameters
    jdata = json.dumps({
        "kind": "Cluster",
        "metadata": {
            "name": "testCluster"
        },
        "spec": {
            "quorumNodes": [ "node1", "node2", "node3" ],
            "virtualIP": "192.168.30.10"
        }
    })

    # Post the data
    response = http.httpPost(postUrl, jdata)
    print "Init cluster response is: " + response

# Parse command line args
# Create the parser and sub parser
parser = argparse.ArgumentParser()
parser.add_argument('--version', action='version', version='1.0.0')
parser.add_argument("-nodes", required=True, help="list of nodes(comma separated)")
parser.add_argument("-user", default='vagrant', help="User id for ssh")
parser.add_argument("-password", default='vagrant', help="password for ssh")
parser.add_argument("-gopath", default='/import', help="GOPATH directory path")
parser.add_argument("-stop", dest='stop', action='store_true')

# Parse the args
args = parser.parse_args()
addrList = args.nodes.split(",")

# Basic error checking
if len(addrList) < 1:
    print "Empty address list"
    sys.exit(1)

# list of nodes
nodes = []


print "################### Stopping all cluster services ###################"

# Connect to nodes
for addr in addrList:
    node = Node(addr, args.user, args.password, args.gopath)
    nodes.append(node)

pool = ThreadPool(len(addrList))
pool.map(lambda x: x.stopCluster(),nodes)

print "################### Waiting for cluster cleanup to complete ###################"
time.sleep(2)

# When -stop was passed, we are done
if args.stop:
    os._exit(0)

print "################### Starting all cluster services ###################"

pool = ThreadPool(len(addrList))
pool.map(lambda x: x.startCluster(),nodes)


print "################### Waiting for pen-base container to come up ###################"
time.sleep(5)

# init the cluster
initCluster(addrList[0])

print "################### Created Pensando Cluster #####################"
os._exit(0)
