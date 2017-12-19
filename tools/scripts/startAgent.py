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

    # Start K8s agent process on vagrant node
    def startK8sAgent(self, args=""):
        ssh_object = self.sshConnect(self.username, self.password)
        command = "sudo " + self.gopath + "/bin/k8sagent " + args + "> /tmp/pensando-k8sagent.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()

    # Start Naples netagent process on vagrant node
    def startN4sAgent(self, hostif, uplink):
        ssh_object = self.sshConnect(self.username, self.password)
        command = "sudo " + self.gopath + "/bin/n4sagent -hostif " + hostif + " -uplink " + uplink + " > /tmp/pensando-n4sagent.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()

    # Start NMD process on vagrant node
    def startNMD(self, hostif):
        ssh_object = self.sshConnect(self.username, self.password)
        command = "sudo " + self.gopath + "/bin/nmd -hostif " + hostif + " > /tmp/pensando-nmd.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()

    # Start netctrler process on vagrant node
    def startNetctrler(self, args=""):
        ssh_object = self.sshConnect(self.username, self.password)
        command = self.gopath + "/bin/npm " + args + "> /tmp/pensando-npm.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()

    # Start api server process on vagrant node
    def startApiserver(self, args=""):
        ssh_object = self.sshConnect(self.username, self.password)
        command = self.gopath + "/bin/apiserver " + args + "> /tmp/pensando-apiserver.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()

    # Start api gw process on vagrant node
    def startApigw(self, args=""):
        ssh_object = self.sshConnect(self.username, self.password)
        command = self.gopath + "/bin/apigw -port :9090" + args + "> /tmp/pensando-apigw.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()

    # Start hostsim process on vagrant node
    def startHostsim(self, simif):
        ssh_object = self.sshConnect(self.username, self.password)
        command = "sudo -E " + self.gopath + "/bin/hostsim -uplink " + simif + " > /tmp/pensando-hostsim.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()

    # Start etcd
    def startEtcd(self):
        self.runCmd("docker run -d -v /usr/share/ca-certificates/:/etc/ssl/certs --net=host --name etcd quay.io/coreos/etcd:v3.2.5")

# Parse command line args
# Create the parser and sub parser
parser = argparse.ArgumentParser()
parser.add_argument('--version', action='version', version='1.0.0')
parser.add_argument("-nodes", default='', help="list of nodes(comma separated)")
parser.add_argument("-user", default='vagrant', help="User id for ssh")
parser.add_argument("-password", default='vagrant', help="password for ssh")
parser.add_argument("-gopath", default='/import', help="GOPATH directory path")
parser.add_argument("-k8s", dest='k8s', action='store_true')
parser.add_argument("-stop", dest='stop', action='store_true')
parser.add_argument("-hostif", default='ntrunk0', help="Host facing interface")
parser.add_argument("-uplink", default='eth2', help="Naples uplink")
parser.add_argument("-simif", default='strunk0', help="Hostsim uplink")

# Parse the args
args = parser.parse_args()
addrList = args.nodes.split(",")

if args.nodes == '':
    addrList = os.environ["PENS_NODES"].split(",")

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
    node.runCmd("sudo pkill n4sagent")
    node.runCmd("sudo pkill k8sagent")
    node.runCmd("sudo pkill nmd")
    node.runCmd("sudo pkill npm")
    node.runCmd("sudo pkill apigw")
    node.runCmd("sudo pkill apiserver")
    node.runCmd("sudo pkill hostsim")
    node.runCmd("/usr/sbin/ifconfig -a | grep -e vport | awk '{print $1}' | xargs -r -n1 -I{} sudo ip link delete {} type veth")
    node.runCmd("docker rm -f `docker ps -aq`")
    node.runCmd("sudo ip link delete strunk0 type veth peer name ntrunk0")
    node.runCmd("sudo ovs-vsctl del-br SimBridge")

    # Copy conf and binary files for CNI plugin
    node.runCmd("sudo cp " + node.gopath + "/src/github.com/pensando/sw/agent/plugins/k8s/cni/pensandonet/01-pensando.conf /etc/cni/net.d/")
    node.runCmd("sudo cp " + node.gopath + "/bin/pensandonet /opt/cni/bin/")

    # create directory for .sock files and remove any stale .sock files
    node.runCmd("sudo mkdir -p /run/pensando/")
    node.runCmd("sudo rm /run/pensando/pensando-cni.sock")
    node.runCmd("sudo ifconfig eth2 promisc up")

# When -stop was passed, we are done
if args.stop:
    os._exit(0)

# start apiserver and api gw on master node
nodes[0].startEtcd()
time.sleep(2)
nodes[0].startApiserver()
time.sleep(2)
nodes[0].startApigw()

# start netctrler on master node
nodes[0].startNetctrler()

print "################### Waiting for netctrler to come up ###################"
time.sleep(5)

# Start pensando agent
for node in nodes:
    # creat the veth pair for ovs
    node.runCmd("sudo ip link add strunk0 type veth peer name ntrunk0")
    node.runCmd("sudo ifconfig ntrunk0 promisc up")
    node.runCmd("sudo ifconfig strunk0 promisc up")

    # start the agent
    if args.k8s:
        node.startK8sAgent()
    else:
        node.startN4sAgent(args.hostif, args.uplink)
        node.startNMD(args.hostif)

    # start hostsim
    node.startHostsim(args.simif)


print "################### Waiting for agent to come up ###################"
time.sleep(5)

print "################### Started Pensando Agent #####################"
os._exit(0)
