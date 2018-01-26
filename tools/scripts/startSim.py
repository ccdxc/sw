#!/usr/bin/python

# Start pensando agent and hostsim on all nodes

import time
import sys
import os
import argparse
import paramiko
import threading
import json
import http
import exceptions
import traceback

# Port numbers - keep in sync with venice/globals/constants.go
APIGwRESTPort = "9000"
CMDClusterMgmtPort = "9002"
CMDResolverPort = "9009"
CMDUpdatesPort = "9009"

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
    def __init__(self, addr, username='vagrant', password='vagrant', gobin='/import/bin'):
        self.addr = addr
        self.username = username
        self.password = password
        self.gobin = gobin
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
        command = "sudo " + self.gobin + "/k8sagent " + args + "> /tmp/pensando-k8sagent.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()

    # Start Naples netagent process on the node
    def startN4sAgent(self, npm, resolvers, hostif, uplink):
        ssh_object = self.sshConnect(self.username, self.password)
        command = "sudo " + self.gobin + "/n4sagent -npm " + npm + " -resolver-urls " + resolvers + " -hostif " + hostif + " -uplink " + uplink + " > /tmp/pensando-n4sagent.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()

    # Start NMD process on the node
    def startNMD(self, cmdreg, cmdupd, resolvers, hostif, uplink):
        ssh_object = self.sshConnect(self.username, self.password)
        command = "sudo " + self.gobin + "/nmd -cmdregistration " + cmdreg + " -cmdupdates " + cmdupd + \
                  " -mode managed -hostif " + hostif + " > /tmp/pensando-nmd.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()

    # Start hostsim process on the node
    def startHostsim(self, simif, simbin):
        ssh_object = self.sshConnect(self.username, self.password)
        command = "sudo -E " + simbin + "/hostsim -uplink " + simif + " > /tmp/pensando-hostsim.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()

    # Start vcsim
    def startVcsim(self, hostsims, snics):
        ssh_object = self.sshConnect(self.username, self.password)
        command = self.gobin + "/vcsim -hostsim-urls " + hostsims + " -snic-list " + snics + " > /tmp/pensando-vcsim.log 2>&1"
        self.npThread = threading.Thread(target=ssh_exec_thread, args=(ssh_object, command))
        # npThread.setDaemon(True)
        self.npThread.start()
        # self.runCmd("docker run -d --net=host --name vcSim pen-vcsim -hostsim-urls " + hostsims + " -snic-list " + snics)


# Create the network by posting a message to apigw
def createNetwork(nodeAddr, name, subnet, gw, vlanId):
    postUrl = 'http://' + nodeAddr + ':' + APIGwRESTPort + '/v1/networks/default/networks'

    # network json parameters
    jdata = json.dumps({
        "kind": "Network",
        "meta": {
            "tenant": "default",
            "name": name
        },
        "spec": {
            "ipv4-subnet": subnet,
            "ipv4-gateway": gw,
            "vlan-id": vlanId
        }
    })

    # Post the data
    response = http.httpPost(postUrl, jdata)
    print "Network create response is: " + response

# Parse command line args
# Create the parser and sub parser
parser = argparse.ArgumentParser()
parser.add_argument('--version', action='version', version='1.0.0')
parser.add_argument("-nodes", default='', help="list of nodes(comma separated)")
parser.add_argument("-npm", default='pen-npm', help="NPM URL")
parser.add_argument("-cmdregistration", default='pen-master:' + CMDClusterMgmtPort, help="CMD Cluster Mgmt URL")
parser.add_argument("-cmdupdates", default='pen-master:' + CMDUpdatesPort, help="CMD NIC Updated URL")
parser.add_argument("-resolvers", default='pen-master:' + CMDResolverPort, help="Resolver URLs")
parser.add_argument("-simnodes", default='', help="list of nodes(comma separated)")
parser.add_argument("-user", default='vagrant', help="User id for ssh")
parser.add_argument("-password", default='vagrant', help="password for ssh")
parser.add_argument("-gobin", default='/import/bin', help="$GOPATH/bin directory path")
parser.add_argument("-simbin", default='/import/bin', help="host sim & vcsim binary path")
parser.add_argument("-k8s", dest='k8s', action='store_true')
parser.add_argument("-stop", dest='stop', action='store_true')
parser.add_argument("-hostif", default='ntrunk0', help="Host facing interface")
parser.add_argument("-uplink", default='eth2', help="Naples uplink")
parser.add_argument("-simif", default='strunk0', help="Hostsim uplink")

# Parse the args
args = parser.parse_args()
addrList = args.nodes.split(",")
simAddrList = args.simnodes.split(",")

if args.nodes == '':
    addrList = os.environ["PENS_NODES"].split(",")

if args.simnodes == '':
    simAddrList = addrList


# Basic error checking
if len(addrList) < 1:
    print "Empty address list"
    sys.exit(1)

nodes = []
simNodes = []
try:
    # Connect to nodes
    for addr in addrList:
        node = Node(addr, args.user, args.password, args.gobin)
        nodes.append(node)

        # cleanup any old agent instances still running
        node.runCmd("sudo pkill n4sagent")
        node.runCmd("sudo pkill k8sagent")
        node.runCmd("sudo pkill nmd")
        node.runCmd("sudo pkill hostsim")
        node.runCmd("sudo pkill vcsim")
        node.runCmd("/usr/sbin/ifconfig -a | grep -e vport | awk '{print $1}' | xargs -r -n1 -I{} sudo ip link delete {} type veth")
        node.runCmd("sudo ip link delete strunk0 type veth peer name ntrunk0")
        node.runCmd("sudo ovs-vsctl del-br SimBridge")

        # Copy conf and binary files for CNI plugin
        node.runCmd("sudo cp " + node.gobin + "../src/github.com/pensando/sw/agent/plugins/k8s/cni/pensandonet/01-pensando.conf /etc/cni/net.d/")
        node.runCmd("sudo cp " + node.gobin + "/pensandonet /opt/cni/bin/")

        # create directory for .sock files and remove any stale .sock files
        node.runCmd("sudo mkdir -p /run/pensando/")
        node.runCmd("sudo rm /run/pensando/pensando-cni.sock")

    # Connect to sim nodes
    for addr in simAddrList:
        snode = Node(addr, args.user, args.password, args.gobin)
        simNodes.append(snode)

        # cleanup any old agent instances still running
        snode.runCmd("sudo pkill n4sagent")
        snode.runCmd("sudo pkill k8sagent")
        snode.runCmd("sudo pkill nmd")
        snode.runCmd("sudo pkill hostsim")
        snode.runCmd("sudo docker ps -a | grep alpine | awk '{print $1}' | xargs -r -n1 -I{} echo sudo docker rm -f {}")
        snode.runCmd("sudo ip link delete strunk0 type veth peer name ntrunk0")
        snode.runCmd("sudo ovs-vsctl del-br SimBridge")

    # Stop vcsim
    try:
        vcsim = Node("pen-master", args.user, args.password, args.simbin)
        vcsim.runCmd("sudo pkill vcsim")
    except:
        pass

    # When -stop was passed, we are done
    if args.stop:
        os._exit(0)

    # Start pensando agent
    for idx, node in enumerate(nodes):
        # creat the veth pair for ovs
        node.runCmd("sudo ip link add strunk0 type veth peer name ntrunk0")
        node.runCmd("sudo ifconfig ntrunk0 hw ether 02:02:02:02:02:" + '{:02x}'.format(idx+1) + " promisc up")
        node.runCmd("sudo ifconfig strunk0 hw ether 02:02:02:02:02:" + '{:02x}'.format(idx+1) + " promisc up")
        node.runCmd("sudo ifconfig " + args.hostif + " promisc up")
        node.runCmd("sudo ifconfig " + args.uplink + " promisc up")

        # start the agent
        if args.k8s:
            node.startK8sAgent()
        else:
            node.startN4sAgent(args.npm, args.resolvers, args.hostif, args.uplink)
            node.startNMD(args.cmdregistration, args.cmdupdates, args.resolvers, args.hostif, args.uplink)

    print "################### Started Pensando Agents #####################"

    for idx, snode in enumerate(simNodes):
        snode.runCmd("sudo ifconfig " + args.simif + " promisc up")
        snode.runCmd("sudo docker pull alpine")
        # start hostsim
        snode.startHostsim(args.simif, args.simbin)

    # gather ism addresses
    hsims = []
    snics = []
    for idx, addr in enumerate(simAddrList):
        hsims.append("http://" + addr + ":5050")
        snics.append("02:02:02:02:02:" + '{:02x}'.format(idx+1))

    # start vcsim
    vcsim = Node("pen-master", args.user, args.password, args.simbin)
    vcsim.startVcsim(",".join(hsims), ",".join(snics))
    time.sleep(2)

    print "################### Started Simulation agents #####################"

    # Create a network
    createNetwork("pen-master", "default", "10.1.0.0/16", "10.1.254.254", 2)
    print "################### Created default network #####################"
except Exception, err:
    print "ERROR while running the script: "
    print err
    traceback.print_exc()
    os._exit(1)

time.sleep(1)
os._exit(0)
