#!/usr/bin/python
import sys
import os
import time
import json
import subprocess
import argparse
import http
from multiprocessing.dummy import Pool as ThreadPool
from multiprocessing import cpu_count

def runCommand(cmd):
    # subprocess.call() is multithreaded but can mess up terminal settings.
    # hence we call 'stty sane' at the exit of the program
    return subprocess.call(cmd, shell=True,stdin=None, stdout=None, stderr=None)
    # if we use os.system() the it serializes across all threads - a bit slower.
    # return os.system(cmd)

class TestMgmtNode:
    def __init__(self, name, ipaddress, quorumNames, quorumIPs, nodes, ipList, clustervip):
        self.debug = False
        self.name = name
        self.ipaddress = ipaddress
        self.quorumNames = quorumNames
        self.quorumIPs = quorumIPs
        self.nodes = nodes
        self.clustervip = clustervip
        self.ipList = ipList
    def runCmd(self,command):
        return runCommand("""docker exec -it {} """.format(self.name) + command)
    def startNode(self):
        runCommand("""docker run -td -P -l pens --network pen-dind-net --ip {}  -v sshSecrets:/root/.ssh -v $GOPATH/src:/import/src --privileged --rm --name {} -h {} registry.test.pensando.io:5000/pens-e2e:v0.1 /bin/sh """.format(self.ipaddress, self.name, self.name))
        self.runCmd("""apk add openssh""")
        self.runCmd("""sh -c 'if ! test -f /root/.ssh/id_rsa ; then ssh-keygen -f /root/.ssh/id_rsa -t rsa -N "";fi ' """)
        self.runCmd("""cp /root/.ssh/id_rsa.pub /root/.ssh/authorized_keys""")
        self.runCmd("""cp /root/.ssh/id_rsa.pub /root/.ssh/authorized_keys2""")
        self.runCmd("""sh -c 'ssh-keyscan {} > /root/.ssh/known_hosts' """.format(" ".join(ipList)))
        self.runCmd("""kubectl   config set-cluster e2e --server=http://{}:8080 """.format(self.clustervip))
        self.runCmd("""kubectl   config set-context e2e --cluster=e2e """)
        self.runCmd("""kubectl   config use-context e2e """)
class Node:
    def __init__(self, name, ipaddress):
        self.debug = False
        self.name = name
        self.ipaddress = ipaddress
    def runCmd(self,command):
        return runCommand("""docker exec -it {} """.format(self.name) + command)
    def startNode(self):
        while runCommand("""docker inspect {} >/dev/null 2>&1""".format(self.name)) == 0:
            time.sleep(2)
        runCommand("""docker run -v/sys/fs/cgroup:/sys/fs/cgroup:ro -P -l pens --network pen-dind-net --ip {} -v sshSecrets:/root/.ssh -v $GOPATH/src:/import/src --privileged --rm -d --name {} -h {} registry.test.pensando.io:5000/pens-dind:v0.1""".format(self.ipaddress, self.name, self.name))
        while self.runCmd("""docker ps >/dev/null 2>&1""".format(self.name)) != 0:
            time.sleep(2)
    def doCluster(self):
        self.startNode()
        self.runCmd("""sh -c 'for i in /import/src/github.com/pensando/sw/bin/tars/* ; do docker load -i  $i; done'""")
        self.startCluster()
    def stopCluster(self):
        penSrvs = ["pen-cmd", "pen-apiserver", "pen-apigw", "pen-etcd", "pen-kube-controller-manager", "pen-kube-scheduler", "pen-kube-apiserver", "pen-elasticsearch", "pen-vchub", "pen-npm" ]
        for srv in penSrvs:
            self.runCmd("systemctl stop " + srv)
            self.runCmd("systemctl disable " + srv)
        self.runCmd("systemctl stop pen-kubelet")
        self.runCmd(r"bash -c 'for i in $(/usr/bin/systemctl list-unit-files --no-legend --no-pager -l | grep --color=never -o kube.*\.slice );do echo $i; systemctl stop $i ; done' ")
        self.runCmd("""bash -c 'if [ "$(docker ps -qa)" != "" ] ; then docker stop -t 2 $(docker ps -qa); docker rm $(docker ps -qa); fi' """)
        self.runCmd("bash -c 'rm -fr /etc/pensando/* /etc/kubernetes/* /usr/pensando/bin/* /var/lib/pensando/* /var/log/pensando/*  /var/lib/cni/ /var/lib/kubelet/* /etc/cni/' ")
        self.runCmd("bash -c 'ip addr flush label *pens' ")
        self.runCmd("""bash -c 'if [ "$(docker ps -qa)" != "" ] ; then docker stop -t 2 $(docker ps -qa); docker rm -f $(docker ps -qa); fi' """)
    def startCluster(self):
        self.runCmd("""docker run --rm --name pen-cmd -v /usr/pensando/bin:/host/usr/pensando/bin -v /usr/lib/systemd/system:/host/usr/lib/systemd/system -v /etc/pensando:/host/etc/pensando pen-cmd -c /initscript""")
        self.runCmd("""systemctl daemon-reload""")
        self.runCmd("""systemctl enable pensando.target""")
        self.runCmd("""systemctl start pensando.target""")
        self.runCmd("""systemctl enable pen-cmd""")
        self.runCmd("""systemctl start pen-cmd""")
    def restartCluster(self):
        self.stopCluster()
        self.runCmd("""sh -c 'for i in /import/src/github.com/pensando/sw/bin/tars/pen* ; do docker load -i  $i; done'""")
        self.startCluster()

def initCluster(nodeAddr, quorumNodes, clustervip):
    postUrl = 'http://' + nodeAddr + ':9001/api/v1/cluster'

    # cluster json parameters
    jdata = json.dumps({
        "kind": "Cluster",
        "APIVersion": "v1",
        "meta": {
            "name": "testCluster"
        },
        "spec": {
            "auto-admit-nics": True,
            "quorum-nodes": quorumNodes,
            "virtual-ip":  clustervip,
            "ntp-servers": ["1.pool.ntp.org","2.pool.ntp.org"]
        }
    })

    # Post the data. try upto 3 times since the server may not be ready..
    for i in range (1,4):
        response = http.httpPost(postUrl, jdata)
        print "Init cluster with " + jdata
        print "cluster response is: " + response
        if response != "Error" :
            break
        time.sleep(3)

def deleteCluster():
    runCommand("""docker stop -t 3 $(docker ps -f label=pens --format '{{.ID}}') >/dev/null 2>&1""")
def stopCluster(nodeList, nodes, quorum, clustervip):
    pool = ThreadPool(len(nodeList))
    pool.map(lambda x: x.stopCluster(), nodes)
def createCluster(nodeList, nodes, quorum, clustervip):
    pool = ThreadPool(len(nodeList))
    runCommand("""if ! docker network inspect pen-dind-net >/dev/null 2>&1; then
        docker network create --ip-range 192.168.30.1/24 --subnet 192.168.30.0/24 pen-dind-net
    fi""")
    pool.map(lambda x: x.doCluster(), nodes)

    for i in range (1,3):
        if runCommand("""docker exec -it node1 /import/src/github.com/pensando/sw/test/e2e/dind/do.py  -configFile '' -init_cluster_only -num_nodes {} -num_quorum {} -clustervip {}""".format(len(nodes), len(quorum), clustervip)) == 0:
            break
        time.sleep(2)

def restartCluster(nodeList, nodes, quorum, clustervip):
    pool = ThreadPool(len(nodeList))
    pool.map(lambda x: x.restartCluster(), nodes)
    for i in range (1,3):
        if runCommand("""docker exec -it node1 /import/src/github.com/pensando/sw/test/e2e/dind/do.py -configFile ''  -init_cluster_only -num_nodes {} -num_quorum {} -clustervip {}""".format(len(nodes), len(quorum), clustervip)) == 0:
            break
        time.sleep(2)

parser = argparse.ArgumentParser()
# these 4 below are used internally not to be directly executed by the caller
parser.add_argument("-clustervip", help="VIP of the cluster")
parser.add_argument("-num_nodes", type=int,help="number of nodes")
parser.add_argument("-num_quorum", type=int,help="number of quorum nodenames")
parser.add_argument("-init_cluster_only", action='store_true',default=False, help="Init the cluster by posting Cluster object to CMD and exit")
# these 4 below are to be called by user
parser.add_argument("-configFile", default="tb_config.json", help="Configuration of the cluster")
parser.add_argument("-restart", action='store_true',default=False, help="restart venice components in existing Cluster by loading new Pensando code")
parser.add_argument("-delete", action='store_true',default=False, help="delete cluster by deleting containers")
parser.add_argument("-stop", action='store_true',default=False, help="stop venice cluster but keep containers")
args = parser.parse_args()

datastore={}
if args.configFile:
    try:
        with open(args.configFile, 'r') as f:
            datastore = json.load(f)
    except IOError:
        pass

clustervip = datastore.get("ClusterVIP", args.clustervip)
num_nodes = int(datastore.get("NumVeniceNodes", args.num_nodes))
num_quorum =  int(datastore.get("NumQuorumNodes", args.num_quorum))

quorumNames = []
for i in range(1, num_quorum + 1):
    quorumNames.append("node{}".format(i))

nodeList = []
for i in range(1, num_nodes + 1):
    nodeList.append("node{}".format(i))

ipList = []
for i in range(1, num_nodes + 1):
    ipList.append("192.168.30.{}".format(10+i))

quorumIPs = []
for i in range(1, num_quorum + 1):
    quorumIPs.append("192.168.30.{}".format(10+i))


nodes = []
for addr in xrange(len(nodeList)):
    node = Node(nodeList[addr], ipList[addr])
    nodes.append(node)

if args.init_cluster_only:
    initCluster(ipList[0], quorumIPs, clustervip)
    os.system("stty sane")
    sys.exit(0)

if args.stop:
    stopCluster(nodeList, nodes, quorumNames, clustervip)
    os.system("stty sane")
    sys.exit(0)
if args.restart:
    restartCluster(nodeList, nodes, quorumNames, clustervip)
    os.system("stty sane")
    sys.exit(0)
if args.delete:
    deleteCluster()
    os.system("stty sane")
    sys.exit(0)

deleteCluster()
testMgmtNode=TestMgmtNode("node0","192.168.30.9", quorumNames, quorumIPs, nodes, ipList, clustervip)
createCluster(nodeList, nodes, quorumNames, clustervip)
testMgmtNode.startNode()

os.system("stty sane")
sys.exit(0)
