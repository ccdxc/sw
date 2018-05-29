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

src_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../../..")
registry = "registry.test.pensando.io:5000"

debug = False

exposedPortBase = 10000

def runCommand(cmd, ignore_error=False):
    # subprocess.call() is multithreaded but can mess up terminal settings.
    # hence we call 'stty sane' at the exit of the program
    if debug:
        print cmd
        return 0
    elif ignore_error:
        return subprocess.call(cmd, shell=True, stdin=None)
    else:
        return subprocess.check_call(cmd, shell=True,stdin=None)

class TestMgmtNode:
    def __init__(self, name, ipaddress, quorumNames, quorumIPs, nodes, ipList, clustervip, containerIndex):
        self.containerIndex = containerIndex
        self.debug = False
        self.name = name
        self.ipaddress = ipaddress
        self.quorumNames = quorumNames
        self.quorumIPs = quorumIPs
        self.nodes = nodes
        self.clustervip = clustervip
        self.ipList = ipList
    def runCmd(self,command, ignore_error=False):
        return runCommand("""docker exec -it {} """.format(self.name) + command, ignore_error)
    def startNode(self):
        # expose ApiGw(9000) instances on 10001, 10002, 10003 ...
        # expose Elasticsearch(9200) instances on 10201, 10202, 10203 ...
        runCommand("""docker run -td -p {}:9000 -p {}:9200 -l pens --network pen-dind-net --ip {}  -v sshSecrets:/root/.ssh -v {}:/import/src/github.com/pensando/sw --privileged --rm --name {} -h {} registry.test.pensando.io:5000/pens-e2e:v0.2 /bin/sh """.format(exposedPortBase + self.containerIndex, exposedPortBase + 200 + self.containerIndex, self.ipaddress, src_dir, self.name, self.name))
        self.runCmd("""apk add openssh""")
        self.runCmd("""sh -c 'if ! test -f /root/.ssh/id_rsa ; then ssh-keygen -f /root/.ssh/id_rsa -t rsa -N "";fi ' """)
        self.runCmd("""cp /root/.ssh/id_rsa.pub /root/.ssh/authorized_keys""")
        self.runCmd("""cp /root/.ssh/id_rsa.pub /root/.ssh/authorized_keys2""")
        self.runCmd("""sh -c 'ssh-keyscan {} > /root/.ssh/known_hosts' """.format(" ".join(ipList)))
        self.runCmd("""kubectl   config set-cluster e2e --server=http://{}:8080 """.format(self.clustervip))
        self.runCmd("""kubectl   config set-context e2e --cluster=e2e """)
        self.runCmd("""kubectl   config use-context e2e """)
        self.runCmd("""mkdir -p /etc/bash_completion.d""")
        self.runCmd("""go install ./venice/exe/venice""")
        self.runCmd("""/usr/local/go/bin/venice auto-completion""")
        # expose ApiGw(9000) instances on 10001, 10002, 10003 ...
        # expose Elasticsearch(9200) instances on 10201, 10202, 10203 ...
        self.runCmd("""sh -c 'echo export PENSERVER=http://{}:9000 >> ~/.bashrc' """.format(self.clustervip))
        self.runCmd("""sh -c 'echo source /etc/bash_completion.d/venice >> ~/.bashrc' """)
class Node:
    def __init__(self, name, ipaddress, containerIndex):
        self.containerIndex = containerIndex
        self.debug = False
        self.name = name
        self.ipaddress = ipaddress
    def runCmd(self,command, ignore_error=False):
        return runCommand("""docker exec -it {} """.format(self.name) + command, ignore_error)
    def startNode(self):
        while debug or runCommand("""docker inspect {} >/dev/null 2>&1""".format(self.name), ignore_error=True) == 0:
            time.sleep(2)
        runCommand("""docker run -v/sys/fs/cgroup:/sys/fs/cgroup:ro -p {}:9000 -p {}:9200 -l pens -l pens-dind --network pen-dind-net --ip {} -v sshSecrets:/root/.ssh -v {}:/import/src/github.com/pensando/sw --privileged --rm -d --name {} -h {} registry.test.pensando.io:5000/pens-dind:v0.1""".format(exposedPortBase + self.containerIndex, exposedPortBase + 200 + self.containerIndex, self.ipaddress, src_dir, self.name, self.name))
        # hitting https://github.com/kubernetes/kubernetes/issues/50770 on docker-ce on mac but not on linux
        while self.runCmd("""docker ps >/dev/null 2>&1""".format(self.name), ignore_error=True) != 0:
            time.sleep(2)
        self.runCmd(r'''bash  -c ' CGR=`grep memory /proc/1/cgroup  | cut -d: -f3` ; if [ !  -z $CGR ]; then  mkdir -p /sys/fs/cgroup/cpuset/$CGR; fi; if [ !  -z $CGR ]; then mkdir -p /sys/fs/cgroup/hugetlb/$CGR; fi' '''.format(self.name))

    def doCluster(self):
        self.startNode()
        self.loadImage()
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
    def loadImage(self):
        self.runCmd("""sh -c 'for i in /import/src/github.com/pensando/sw/bin/tars/pen* ; do docker load -i  $i; done'""")
        self.runCmd("""docker run --rm --name pen-install -v /usr/pensando/bin:/host/usr/pensando/bin -v /usr/lib/systemd/system:/host/usr/lib/systemd/system -v /etc/pensando:/host/etc/pensando pen-install -c /initscript""")
    def startCluster(self):
        self.runCmd("""systemctl daemon-reload""")
        self.runCmd("""systemctl enable pensando.target""")
        self.runCmd("""systemctl start pensando.target""")
        self.runCmd("""systemctl enable pen-cmd""")
        self.runCmd("""systemctl start pen-cmd""")
    def restartCluster(self):
        self.stopCluster()
        self.loadImage()
        self.startCluster()
class NaplesNode(Node):
    def __init__(self, name, ipaddress, containerIndex, testMode):
        Node.__init__(self,name,ipaddress, containerIndex)
        self.testMode = testMode
    def doCluster(self):
        self.startNode()
        self.startCluster()
    def startNode(self):
        if self.testMode == "HAL":
            runCommand("""docker run -td -P -l pens --network pen-dind-net  --sysctl net.ipv6.conf.all.disable_ipv6=0 --privileged --ip {} -v {}:/sw -v {}/bazel-cache:/root/.cache -w /sw/nic --rm --name {} -h {} {}/pensando/nic:1.18 /bin/sh """.format(self.ipaddress, src_dir, src_dir, self.name, self.name, registry))
            runCommand("""docker exec {}  bash -c "mkdir -p /go/src/github.com/pensando/sw" """.format(self.name))
            runCommand("""docker exec {}  bash -c "mount -o bind /sw /go/src/github.com/pensando/sw" """.format(self.name))
            runCommand("""docker exec {}  bash -c "cd /go && go install github.com/pensando/sw/nic/agent/cmd/netagent" """.format(self.name))
            runCommand("""docker exec {}  bash -c "cd /go && go install github.com/pensando/sw/nic/agent/cmd/nmd" """.format(self.name))
        else:
            runCommand("""docker run -td -P -l pens --network pen-dind-net --ip {}  --rm --name {} -h {} pen-netagent /bin/sh """.format(self.ipaddress, self.name, self.name, self.name))
        runCommand("""docker exec {}  mkdir -p /var/log/pensando """.format(self.name))
        runCommand("""docker network connect pen-dind-hnet {}""".format(self.name))
        runCommand("""docker network connect pen-dind-nnet {}""".format(self.name))
        runCommand("""docker exec {}  bash -c "echo 192.168.30.10 pen-master | tee -a /etc/hosts " """.format(self.name))
    def stopCluster(self):
        self.runCmd("""killall -q netagent""")
        self.runCmd("""killall -q nmd""")
        pass
    def restartCluster(self):
        self.stopCluster()
        self.startCluster()
    def startCluster(self):
        # start nmd as a native process on NaplesNode
        if self.testMode == "HAL":
            runCommand("""docker exec -d {} nmd -cmdregistration 192.168.30.10:9002 -cmdupdates 192.168.30.10:9009 -hostif eth1 -resolver 192.168.30.10:9009 -mode managed & """.format(self.name))
            runCommand("""docker exec -d {} make e2e-sanity-hal-bringup""".format(self.name))
            runCommand("""docker exec -d {} bash -c "agent/netagent/scripts/wait-for-hal.sh && netagent -npm pen-npm -resolver-urls 192.168.30.10:9009 -hostif eth1 -datapath hal -mode managed &" """.format(self.name))
        else:
            runCommand("""docker exec -d {} /nmd -cmdregistration 192.168.30.10:9002 -cmdupdates 192.168.30.10:9009 -hostif eth1 -resolver 192.168.30.10:9009 -mode managed -mode managed & """.format(self.name))
            runCommand("""docker exec -d {} /netagent -npm pen-npm -resolver-urls 192.168.30.10:9009 -hostif eth1 -datapath mock &""".format(self.name))

def initCluster(nodeAddr, quorumNodes, clustervip):
    postUrl = 'http://' + nodeAddr + ':9001/api/v1/cluster'

    # cluster json parameters
    jdata = json.dumps({
        "kind": "Cluster",
        "api-version": "v1",
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
    runCommand("""docker stop -t 3 node0 >/dev/null 2>&1""", ignore_error=True)
    runCommand(""" for i in $(docker ps -f label=pens-dind --format '{{.ID}}'); do docker exec -it $i init 0; done """, ignore_error=True)
    runCommand("""docker stop -t 3 $(docker ps -f label=pens --format '{{.ID}}') >/dev/null 2>&1""", ignore_error=True)
    runCommand("""docker network remove pen-dind-net 2>/dev/null""", ignore_error=True)
    runCommand("""docker network remove pen-dind-hnet 2>/dev/null""", ignore_error=True)
    runCommand("""docker network remove pen-dind-nnet 2>/dev/null""", ignore_error=True)
    runCommand("""docker volume  prune -f 2>/dev/null""", ignore_error=True)
    runCommand("""docker system  prune -f 2>/dev/null""", ignore_error=True)

def stopCluster(nodeList, nodes, quorum, clustervip):
    pool = ThreadPool(len(nodeList))
    pool.map(lambda x: x.stopCluster(), nodes)

def createCluster(nodeList, nodes, quorum, clustervip, test_mode):
    pool = ThreadPool(len(nodes))
    runCommand("""if ! docker network inspect pen-dind-hnet >/dev/null 2>&1; then
        docker network create --internal --ip-range 192.168.28.1/24 --subnet 192.168.28.0/24 pen-dind-hnet
    fi""")
    runCommand("""if ! docker network inspect pen-dind-nnet >/dev/null 2>&1; then
        docker network create --internal --ip-range 192.168.29.1/24 --subnet 192.168.29.0/24 pen-dind-nnet
    fi""")
    runCommand("""if ! docker network inspect pen-dind-net >/dev/null 2>&1; then
        docker network create --ip-range 192.168.30.1/24 --subnet 192.168.30.0/24 pen-dind-net
    fi""")
    pool.map(lambda x: x.doCluster(), nodes)

    for i in range (1,3):
        if runCommand("""docker exec -it node1 /import/src/github.com/pensando/sw/test/e2e/dind/do.py  -configFile '' -init_cluster_only -num_nodes {} -num_quorum {} -clustervip {} -test_mode {}""".format(len(nodes), len(quorum), clustervip, test_mode)) == 0:
                break
        time.sleep(2)


def restartCluster(nodeList, nodes, quorum, clustervip):
    pool = ThreadPool(len(nodes))
    pool.map(lambda x: x.restartCluster(), nodes)
    for i in range (1,3):
        if runCommand("""docker exec -it node1 /import/src/github.com/pensando/sw/test/e2e/dind/do.py -configFile ''  -init_cluster_only -num_nodes {} -num_quorum {} -clustervip {} -test_mode {}""".format(len(nodes), len(quorum), clustervip), test_mode) == 0:
            break
        time.sleep(2)

parser = argparse.ArgumentParser()
# these 6 below are used internally not to be directly executed by the caller
parser.add_argument("-clustervip", help="VIP of the cluster")
parser.add_argument("-num_nodes", type=int,help="number of nodes")
parser.add_argument("-num_naples", type=int,default=0,help="number of naples nodes")
parser.add_argument("-num_quorum", type=int,help="number of quorum nodenames")
parser.add_argument("-init_cluster_only", action='store_true',default=False, help="Init the cluster by posting Cluster object to CMD and exit")
parser.add_argument("-test_mode", type=str, default="MOCK", help="Specify Agent datapath mode.")
# these 5 below are to be called by user
parser.add_argument("-configFile", default="tb_config.json", help="Configuration of the cluster")
parser.add_argument("-restart", action='store_true',default=False, help="restart venice components in existing Cluster by loading new Pensando code")
parser.add_argument("-delete", action='store_true',default=False, help="delete cluster by deleting containers")
parser.add_argument("-stop", action='store_true',default=False, help="stop venice cluster but keep containers")
parser.add_argument("-load_image_only", action='store_true',default=False, help="load and install new venice image (not restart)")
args = parser.parse_args()

if args.delete:
    deleteCluster()
    os.system("stty sane")
    sys.exit(0)

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
num_naples =  int(datastore.get("NumNaplesNodes", args.num_naples))
test_mode = datastore.get("E2EMode", args.test_mode)

quorumNames = []
for i in range(1, num_quorum + 1):
    quorumNames.append("node{}".format(i))

naplesNames = []
for i in range(1, num_naples + 1):
    naplesNames.append("naples{}".format(i))

nodeList = []
for i in range(1, num_nodes + 1):
    nodeList.append("node{}".format(i))

ipList = []
for i in range(1, num_nodes + 1):
    ipList.append("192.168.30.{}".format(10+i))

quorumIPs = []
for i in range(1, num_quorum + 1):
    quorumIPs.append("192.168.30.{}".format(10+i))

naplesIPs = []
for i in range(1, num_naples + 1):
    naplesIPs.append("192.168.30.{}".format(20+i))

# this is a global index of all the containers running in the e2e system
# node0 has containerIndex=0.
# a containerIndex is given to every type of node (venice, naples, e2e)
containerIndex = 1

nodes = []
for addr in xrange(len(nodeList)):
    node = Node(nodeList[addr], ipList[addr], containerIndex)
    containerIndex = containerIndex + 1
    nodes.append(node)

naplesNodes = []
for addr in xrange(len(naplesNames)):
    node = NaplesNode(naplesNames[addr], naplesIPs[addr], containerIndex, test_mode)
    containerIndex = containerIndex + 1
    naplesNodes.append(node)


if args.init_cluster_only:
    initCluster(ipList[0], quorumNames, clustervip)
    os.system("stty sane")
    sys.exit(0)
if args.load_image_only:
    pool = ThreadPool(len(nodes))
    pool.map(lambda x: x.loadImage(), nodes)
    os.system("stty sane")
    sys.exit(0)
if args.stop:
    stopCluster(nodeList, nodes + naplesNodes, quorumNames, clustervip)
    os.system("stty sane")
    sys.exit(0)
if args.restart:
    restartCluster(nodeList, nodes + naplesNodes, quorumNames, clustervip)
    os.system("stty sane")
    sys.exit(0)

deleteCluster()
testMgmtNode=TestMgmtNode("node0","192.168.30.9", quorumNames, quorumIPs, nodes, ipList, clustervip, 0)
createCluster(nodeList, nodes + naplesNodes, quorumNames, clustervip, test_mode)
testMgmtNode.startNode()

os.system("stty sane")
sys.exit(0)
