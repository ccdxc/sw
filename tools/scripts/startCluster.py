#!/usr/bin/python

import time
import sys
import os
import argparse
import json
import http
import exceptions
from multiprocessing.dummy import Pool as ThreadPool

import paramiko

dryRun = False

# Utility function to run ssh
def ssh_exec_thread(ssh_object, command):
    print "run: " + command
    _, stdout, _ = ssh_object.exec_command(command)
    out = stdout.readlines()
    print out
    print "Program exited: " + command
    exitCode = stdout.channel.recv_exit_status()
    if exitCode != 0:
        print "Exit code: " + str(exitCode)

# This class represents a vagrant node
class Node:
    def __init__(self, ipaddr, username='vagrant', password='vagrant', gopath='/import/'):
        self.debug = False
        self.addr = ipaddr
        self.username = username
        self.password = password
        self.gopath = gopath
        self.ssh = self.sshConnect(username, password)
        if not dryRun:
            out, _, _ = self.runCmd("hostname")
            self.hostname = out[0].split('\n')[0]
        else:
            self.hostname = "dryrun " + self.addr
        print "Connected to " + self.hostname

    # Connect to vagrant node
    def sshConnect(self, username, password):
        ssh_object = paramiko.SSHClient()
        ssh_object.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        print "Connecting to " + self.addr + " with userid: " + username + " password: " + password
        if dryRun:
            return
        try:
            ssh_object.connect(self.addr, username=username, password=password)
            return ssh_object
        except paramiko.ssh_exception.AuthenticationException:
            sys.exit("Authentication failed")

    def isConnected(self):
        if dryRun:
            return True
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
            if dryRun:
                return

            # Execute the command
            _, stdout, stderr = self.ssh.exec_command(cmd, timeout=timeout)
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

    # Start pensando cluster services on each node
    def startCluster(self):
        print "#### Loading container images on " + self.addr
        self.runCmd("""sync; sudo bash -c "echo 3 > /proc/sys/vm/drop_caches" """)
        self.runCmd("""bash -c 'for i in /import/bin/tars/* ; do  docker load -i $i; sync; sudo bash -c "echo 3 > /proc/sys/vm/drop_caches";  done;' """)
        self.runCmd("docker system prune -f")
        self.runCmd("docker run --rm --name pen-cmd -v /usr/pensando/bin:/host/usr/pensando/bin -v /usr/lib/systemd/system:/host/usr/lib/systemd/system -v /etc/pensando:/host/etc/pensando pen-cmd -c /initscript")
        self.runCmd("sudo systemctl daemon-reload")
        self.runCmd("sudo systemctl enable pensando.target")
        self.runCmd("sudo systemctl start pensando.target")
        self.runCmd("sudo systemctl enable pen-cmd")
        print "#### Starting pen-cmd container on " + self.addr
        self.runCmd("sudo systemctl start pen-cmd")

    # Stop Cluster (stop various services that we started and cleanup all config and data)
    def stopCluster(self):
        print "#### Stopping pensando services on " + self.addr

        # stop all services
        self.runCmd("sudo systemctl stop pensando.target")
        self.runCmd("sudo systemctl disable pensando.target")
        penSrvs = ["pen-cmd", "pen-apiserver", "pen-apigw", "pen-etcd", "pen-kube-controller-manager", "pen-kube-scheduler", "pen-kube-apiserver", "pen-elasticsearch", "pen-vchub", "pen-npm" ]
        for srv in penSrvs:
            self.runCmd("sudo systemctl stop " + srv)
            self.runCmd("sudo systemctl disable " + srv)
            self.runCmd("docker stop " + srv)
            self.runCmd("docker rm -f " + srv)

        #stop kubelet
        self.runCmd("sudo systemctl stop pen-kubelet")
        self.runCmd(r"bash -c 'for i in $(/usr/bin/systemctl list-unit-files --no-legend --no-pager -l | grep --color=never -o kube.*\.slice );do echo $i; systemctl stop $i ; done' ")
        self.runCmd("""bash -c 'if [ "$(docker ps -qa)" != "" ] ; then docker stop $(docker ps -qa); docker rm $(docker ps -qa); fi' """)

        self.runCmd("sudo rm -fr /etc/pensando/* /etc/kubernetes/* /usr/pensando/bin/* /var/lib/pensando/* /var/log/pensando/*  /var/lib/cni/ /var/lib/kubelet/* /etc/cni/ ")
        self.runCmd("sudo ip addr flush dev eth1 label *pens")
        self.runCmd("docker rm -f `docker ps -aq`")


# Inits cluster by posting a message to CMD
def initCluster(nodeAddr):
    postUrl = 'http://' + nodeAddr + ':9001/api/v1/cluster'

    # cluster json parameters
    jdata = json.dumps({
        "kind": "Cluster",
        "APIVersion": "v1",
        "meta": {
            "name": "testCluster"
        },
        "spec": {
            "autoAdmitNics": True,
            "quorumNodes": quorumNames,
            "virtualIP":  clustervip,
            "ntpServers": ["1.pool.ntp.org","2.pool.ntp.org"]
        }
    })

    # Post the data
    response = http.httpPost(postUrl, jdata)
    print "Init cluster response is: " + response

# Parse command line args
# Create the parser and sub parser
parser = argparse.ArgumentParser()
parser.add_argument('--version', action='version', version='1.0.0')
parser.add_argument("-clustervip", default="", help="VIP of the cluster")
parser.add_argument("-nodes", default="", help="list of nodes(comma separated)")
parser.add_argument("-quorum", default="", help="list of quorum nodenames(comma separated)")
parser.add_argument("-user", default='vagrant', help="User id for ssh")
parser.add_argument("-password", default='vagrant', help="password for ssh")
parser.add_argument("-gopath", default='/import', help="GOPATH directory path")
parser.add_argument("-stop", dest='stop', action='store_true')

# Parse the args
args = parser.parse_args()
addrList = args.nodes.split(",")
quorum = args.quorum.split(",")

# Pick up nodes list from environment
if args.nodes == '':
    addrList = os.environ["PENS_NODES"].split(",")

if args.quorum == '':
    quorum = os.environ["PENS_QUORUM_NODENAMES"].split(",")

if args.clustervip == '':
    clustervip = os.environ["PENS_CLUSTER_VIP"]
else:
    clustervip = args.clustervip

if clustervip == '':
    print "Invalid ClusterVIP"
    sys.exit(1)

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

quorumNames = []
if quorum == "":
    for i in range(1, len(nodes)):
        quorumNames.append("node{}".format(i))
else:
    quorumNames = quorum

pool = ThreadPool(len(addrList))
pool.map(lambda x: x.stopCluster(), nodes)

print "################### Waiting for cluster cleanup to complete ###################"
time.sleep(2)

# When -stop was passed, we are done
if args.stop:
    sys.exit(0)

print "################### Starting all cluster services ###################"

pool = ThreadPool(len(addrList))
pool.map(lambda x: x.startCluster(), nodes)


print "################### Waiting for pen-cmd container to come up ###################"
time.sleep(5)

# init the cluster
initCluster(addrList[0])

print "################### Created Pensando Cluster #####################"
sys.exit(0)
