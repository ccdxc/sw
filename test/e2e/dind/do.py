#!/usr/bin/python
import sys
import os
import time
import json
import subprocess
import argparse
import http
import httplib
from multiprocessing.dummy import Pool as ThreadPool
from multiprocessing import cpu_count
import ipaddr
import tempfile
import shutil

# root of the source tree - used in dev mode so that source is mounted every where
src_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../../..")

if sys.version_info[0] != 2:
    print("This script requires Python version 2.x")
    sys.exit(1)

#script source dir - this needs to be mounted to node1 so that we can post a cluster create object from this script
# we go ahead and mount this to all venice node to simplify this script
script_src_dir = os.path.dirname(os.path.realpath(__file__))

registry = "registry.test.pensando.io:5000"
debug = False
exposedPortBase = 10000

def makeNaplesFRU(containerIndex):
    fruSrcFile = os.path.join(src_dir, "nic/agent/nmd/state/example-configs/fru.json")
    with open(fruSrcFile, 'r') as f:
        fru = json.load(f)
        macAddr = fru['mac-address']
        newMac = macAddr[:-2] + str(int(macAddr[-2:]) + containerIndex)
        fru['mac-address'] = newMac
        return json.dumps(fru)

def runCommand(cmd, ignore_error=False):
    # subprocess.call() is multithreaded but can mess up terminal settings
    if debug:
        print cmd
        return 0
    elif ignore_error:
        return subprocess.call(cmd, shell=True, stdin=None)
    else:
        return subprocess.check_call(cmd, shell=True, stdin=None)

class TestMgmtNode:
    def __init__(self, name, ipaddress, quorumNames, quorumIPs, nodes, ipList, clustervip, containerIndex, nettype, dev_mode):
        self.containerIndex = containerIndex
        self.debug = debug
        self.name = name
        self.ipaddress = ipaddress
        self.quorumNames = quorumNames
        self.quorumIPs = quorumIPs
        self.nodes = nodes
        self.clustervip = clustervip
        self.ipList = ipList
        self.nettype = nettype
        self.dev_mode = dev_mode
    def runCmd(self, command, ignore_error=False):
        return runCommand("""docker exec {} """.format(self.name) + command, ignore_error)
    def startNode(self):
        # expose ApiGw(443) instances on 10001, 10002, 10003 ...
        # expose Elasticsearch(9200) instances on 10201, 10202, 10203 ...
        ports_exposed = ""
        if self.nettype == 'bridge':
            ports_exposed = """ -p {}:443 -p {}:9200 """.format(exposedPortBase + self.containerIndex, exposedPortBase + 200 + self.containerIndex)
        if self.dev_mode:
            runCommand("""docker run --dns-search my.dummy -td {} -l pens --network pen-dind-net --ip {}  -v /var/run/docker.sock:/var/run/docker.sock -v sshSecrets:/root/.ssh -v {}:/import/src/github.com/pensando/sw --privileged --rm --name {} -h {} registry.test.pensando.io:5000/pens-e2e:v0.5 /bin/sh """.format(ports_exposed, self.ipaddress, src_dir, self.name, self.name))
        else:
            runCommand("""docker run --dns-search my.dummy -td {} -l pens --network pen-dind-net --ip {} -v /var/run/docker.sock:/var/run/docker.sock -v sshSecrets:/root/.ssh --privileged --rm --name {} -h {} registry.test.pensando.io:5000/pens-e2e:v0.5 /bin/sh """.format(ports_exposed, self.ipaddress, self.name, self.name))
        self.runCmd("""sh -c 'if ! test -f /root/.ssh/id_rsa ; then ssh-keygen -f /root/.ssh/id_rsa -t rsa -N "";fi ' """)
        self.runCmd("""cp /root/.ssh/id_rsa.pub /root/.ssh/authorized_keys""")
        self.runCmd("""cp /root/.ssh/id_rsa.pub /root/.ssh/authorized_keys2""")
        self.runCmd("""sh -c 'ssh-keyscan {} > /root/.ssh/known_hosts' """.format(" ".join(ipList)))
        self.runCmd("""kubectl   config set-cluster e2e --server=https://{}:6443 --certificate-authority=/root/.kube/auth/ca-bundle.pem""".format(self.clustervip))
        self.runCmd("""kubectl   config set-context e2e --cluster=e2e --user=admin""")
        self.runCmd("""kubectl   config use-context e2e """)
        self.runCmd("""kubectl   config set-credentials admin --client-certificate=/root/.kube/auth/cert.pem --client-key=/root/.kube/auth/key.pem""")
        self.runCmd("""mkdir -p /etc/bash_completion.d""")
        if self.dev_mode:
            self.runCmd("""go install ./venice/exe/venice""")
            self.runCmd("""/usr/local/go/bin/venice auto-completion""")
            self.runCmd("""sh -c 'echo export PENSERVER=http://{}:443 >> ~/.bashrc' """.format(self.clustervip))
            self.runCmd("""sh -c 'echo source /etc/bash_completion.d/venice >> ~/.bashrc' """)
class Node:
    def __init__(self, name, ipaddress, containerIndex, nettype=None, venice_image=None, venice_image_dir=None, dev_mode=None, custom_config_file=None, disableTmpfs=False):
        self.containerIndex = containerIndex
        self.debug = debug
        self.name = name
        self.ipaddress = ipaddress
        self.nettype = nettype
        self.venice_image = venice_image
        self.venice_image_dir = venice_image_dir
        self.dev_mode = dev_mode
        self.custom_config_file = custom_config_file
	self.disableTmpfs = disableTmpfs
    def runCmd(self, command, ignore_error=False):
        return runCommand("""docker exec {} """.format(self.name) + command, ignore_error)
    def startNode(self):
        while runCommand("""docker inspect {} >/dev/null 2>&1""".format(self.name), ignore_error=True) == 0 and not debug:
            time.sleep(2)
        # expose ApiGw(443) instances on 10001, 10002, 10003 ...
        # expose Elasticsearch(9200) instances on 10201, 10202, 10203 ...
        extra_config = ""
        if self.nettype == 'bridge': # all original ports are exposed for macvlan in its own ip. for bridge, we need to expose explicitly
            extra_config = """ -p {}:443 -p {}:9200 -p {}:8080 """.format(exposedPortBase + self.containerIndex, exposedPortBase + 200 + self.containerIndex, 8080 + self.containerIndex - 1)
	if self.disableTmpfs is False:
            extra_config = extra_config + "--tmpfs /var/lib/pensando "
        if self.custom_config_file is not None :
            extra_config = extra_config + """ -v {}:/etc/pensando/configs/shared/common/venice-conf.json """.format(self.custom_config_file)
        if self.dev_mode:
            runCommand("""docker run --dns-search my.dummy -v/sys/fs/cgroup:/sys/fs/cgroup:ro {} -l pens -l pens-dind --network pen-dind-net --ip {} -v {}:/dind -v sshSecrets:/root/.ssh -v {}:/import/src/github.com/pensando/sw --privileged --rm -d --name {} -h {} registry.test.pensando.io:5000/pens-dind:v0.3""".format(extra_config, self.ipaddress, script_src_dir, src_dir, self.name, self.name))
        elif self.venice_image_dir != '':
            runCommand("""docker run --dns-search my.dummy -v/sys/fs/cgroup:/sys/fs/cgroup:ro {} -l pens -l pens-dind --network pen-dind-net --ip {} -v {}:/dind -v sshSecrets:/root/.ssh -v {}:/venice:ro --privileged --rm -d --name {} -h {} registry.test.pensando.io:5000/pens-dind:v0.3""".format(extra_config, self.ipaddress, script_src_dir, self.venice_image_dir, self.name, self.name))
        else:
            runCommand("""docker run --dns-search my.dummy -v/sys/fs/cgroup:/sys/fs/cgroup:ro {} -l pens -l pens-dind --network pen-dind-net --ip {} -v {}:/dind -v sshSecrets:/root/.ssh -v {}:/venice.tgz:ro --privileged --rm -d --name {} -h {} registry.test.pensando.io:5000/pens-dind:v0.3""".format(extra_config, self.ipaddress, script_src_dir, self.venice_image, self.name, self.name))
        # hitting https://github.com/kubernetes/kubernetes/issues/50770 on docker-ce on mac but not on linux
        while self.runCmd("""docker ps >/dev/null 2>&1""", ignore_error=True) != 0 and not debug:
            time.sleep(2)
        self.runCmd(r'''bash  -c ' CGR=`grep memory /proc/1/cgroup  | cut -d: -f3` ; if [ !  -z $CGR ]; then  mkdir -p /sys/fs/cgroup/cpuset/$CGR; fi; if [ !  -z $CGR ]; then mkdir -p /sys/fs/cgroup/hugetlb/$CGR; fi' ''')

    def doCluster(self):
        self.startNode()
        self.startCluster()
    def stopCluster(self):
        penSrvs = ["pen-cmd", "pen-etcd", "pen-kube-controller-manager", "pen-kube-scheduler", "pen-kube-apiserver"]
        for srv in penSrvs:
            self.runCmd("systemctl stop " + srv)
            self.runCmd("systemctl disable " + srv)
        self.runCmd("systemctl stop pen-kubelet")
        self.runCmd(r"bash -c 'for i in $(/usr/bin/systemctl list-unit-files --no-legend --no-pager -l | grep --color=never -o kube.*\.slice );do echo $i; systemctl stop $i ; done' ")
        self.runCmd("""bash -c 'if [ "$(docker ps -qa)" != "" ] ; then docker stop -t 2 $(docker ps -qa); docker rm $(docker ps -qa); fi' """)
        self.runCmd("""bash -c 'for i in $(cat /proc/mounts | grep kubelet | cut -d " " -f 2); do umount $i; done' """)
        self.runCmd("bash -c 'rm -fr /etc/pensando/* /etc/kubernetes/* /usr/pensando/bin/* /var/lib/pensando/* /var/log/pensando/*  /var/lib/cni/ /var/lib/kubelet/* /etc/cni/' ")
        self.runCmd("bash -c 'ip addr flush label *pens' ")
        self.runCmd("""bash -c 'if [ "$(docker ps -qa)" != "" ] ; then docker stop -t 2 $(docker ps -qa); docker rm -f $(docker ps -qa); fi' """)

    def startCluster(self):
        # this is a shortcut bypassing the untar of image and running the script - just to speed up things
        if self.dev_mode:
            self.runCmd("""sh -c 'cd /import/src/github.com/pensando/sw/bin && ../tools/scripts/INSTALL.sh' """)
        elif self.venice_image_dir != '':
            self.runCmd("""sh -c 'cd /venice && ./INSTALL.sh' """)
        else:
            self.runCmd("""sh -c 'mkdir -p /tmp/dind && cd /tmp/dind && tar zxvf /venice.tgz && ./INSTALL.sh' """)
    def restartCluster(self):
        self.stopCluster()
        self.startCluster()
class NaplesNode(Node):
    def __init__(self, name, ipaddress, containerIndex, testMode, clustervip):
        Node.__init__(self, name, ipaddress, containerIndex)
        self.testMode = testMode
        self.clustervip = clustervip
    def doCluster(self):
        self.startNode()
        self.startCluster()
    def startNode(self):
        # expose Naples-REST port (9008) for naples instances on 15000+offset ...
        ports_exposed = """ -p {}:9008 -p {}:8888""".format(exposedPortBase + 5000 + self.containerIndex, exposedPortBase + 5100 + self.containerIndex)
        if self.testMode == "TELEMETRY":
            self.setupCommon()
            runCommand("""docker exec {}  bash -c "cd /go && go install github.com/pensando/sw/nic/agent/cmd/tmagent" """.format(self.name))
            runCommand("""docker exec {}  bash -c "cd /go && go install github.com/pensando/sw/nic/agent/cmd/nmd" """.format(self.name))
        elif self.testMode == "HAL":
            self.setupCommon()
            runCommand("""docker exec {}  bash -c "cd /go && go install github.com/pensando/sw/nic/agent/cmd/netagent" """.format(self.name))
            runCommand("""docker exec {}  bash -c "cd /go && go install github.com/pensando/sw/nic/agent/cmd/nmd" """.format(self.name))
            runCommand("""docker exec {}  bash -c "cd /go && go install github.com/pensando/sw/nic/agent/cmd/tmagent" """.format(self.name))
        else:
            runCommand("""docker run --cap-add=NET_ADMIN --dns-search my.dummy -td {} -P -l pens -l pens-naples --network pen-dind-net --ip {}  --rm --name {} -h {} pen-netagent /bin/sh """.format(ports_exposed, self.ipaddress, self.name, self.name, self.name))
            runCommand("""docker exec {}  bash -c "echo exit 0 > /sbin/start-stop-daemon; chmod 700 /sbin/start-stop-daemon" """.format(self.name))
            runCommand("""docker exec {}  bash -c "ip link set eth0 down && sleep 1 && ip link set eth0 name oob_mnic0 && ip link set oob_mnic0 up" """.format(self.name))
            runCommand("""docker exec {}  bash -c "mkdir -p /sysconfig/config0 /sysconfig/config1" """.format(self.name))
            runCommand("""docker exec {}  bash -c "echo '{}' > /tmp/fru.json" """.format(self.name, makeNaplesFRU(self.containerIndex).replace('"', '\\"')))
        runCommand("""docker exec {}  mkdir -p /var/log/pensando """.format(self.name))
        runCommand("""docker exec {}  mkdir -p /var/log/pensando/elastic """.format(self.name))
        runCommand("""docker exec {}  mkdir -p /var/lib/pensando """.format(self.name))
        runCommand("""docker network connect pen-dind-hnet {}""".format(self.name))
        runCommand("""docker network connect pen-dind-nnet {}""".format(self.name))
        runCommand("""docker exec {}  bash -c "echo {} pen-master | tee -a /etc/hosts " """.format(self.name, self.clustervip))
    def setupCommon(self):
            runCommand("""docker run --dns-search my.dummy -td -P -l pens --network pen-dind-net  --sysctl net.ipv6.conf.all.disable_ipv6=0 --privileged --ip {} -v {}:/sw -v {}/bazel-cache:/root/.cache -w /sw/nic --rm --name {} -h {} {}/pensando/nic:1.18 /bin/sh """.format(self.ipaddress, src_dir, src_dir, self.name, self.name, registry))
            runCommand("""docker exec {}  bash -c "mkdir -p /go/src/github.com/pensando/sw" """.format(self.name))
            runCommand("""docker exec {}  bash -c "mount -o bind /sw /go/src/github.com/pensando/sw" """.format(self.name))

    def stopCluster(self):
        self.runCmd("""killall -q netagent""", ignore_error=True)
        self.runCmd("""killall -q tmagent""", ignore_error=True)
        self.runCmd("""killall -q nmd""", ignore_error=True)
        self.runCmd("""killall -q nevtsproxy""", ignore_error=True)
        self.runCmd("""killall -q ntsa""", ignore_error=True)
        self.runCmd("""rm /tmp/nmd.db""", ignore_error=True)
        self.runCmd("""rm /tmp/naples-netagent.db""", ignore_error=True)
        pass
    def restartCluster(self):
        self.stopCluster()
        self.startCluster()
    def startCluster(self):
        # start nmd as a native process on NaplesNode
        if self.testMode == "TELEMETRY":
            runCommand("""docker exec -d {} /fakedelphihub  & """.format(self.name))
            time.sleep(3)
            runCommand("""docker exec -d {} nmd & """.format(self.name, self.name))
            runCommand("""docker exec -d {} bash -c "tools/start_fte_sim.sh {}:9009 " """.format(self.name, self.clustervip))
        elif self.testMode == "HAL":
            runCommand("""docker exec -d {} /fakedelphihub  & """.format(self.name))
            time.sleep(3)
            runCommand("""docker exec -d {} nmd & """)
            runCommand("""docker exec -d {} make e2e-sanity-hal-bringup""".format(self.name))
            runCommand("""docker exec -d {} bash -c "agent/netagent/scripts/wait-for-hal.sh && netagent -npm pen-npm -resolver-urls {}:9009 -hostif eth1 -datapath hal -mode managed &" """.format(self.name, self.clustervip))
            runCommand("""docker exec -d {} tmagent &""".format(self.name))
        else:
            runCommand("""docker exec -d {} /fakedelphihub  & """.format(self.name))
            time.sleep(3)
            runCommand("""docker exec -d {} /ntsa -config /naples-tsa.json &""".format(self.name))
            runCommand("""docker exec -d {} /nmd -updinterval 2 & """.format(self.name, self.name))
            runCommand("""docker exec -d {} /netagent -datapath mock &""".format(self.name))
            runCommand("""docker exec -d {} /tmagent """.format(self.name))
            runCommand("""docker exec -d {} /nevtsproxy &""".format(self.name))
            runCommand("""docker exec -d {} /fwgen  & """.format(self.name))

def initCluster(nodeAddr, quorumNodes, clustervip):
    postUrl = 'http://' + nodeAddr + ':9001/api/v1/cluster'

    # cluster json parameters
    jdata = json.dumps({
        "kind": "Cluster",
        "api-version": "v1",
        "meta": {
            "name": "testCluster",
        },
        "spec": {
            "auto-admit-dscs": True,
            "quorum-nodes": quorumNodes,
            "virtual-ip":  clustervip,
            "ntp-servers": ["1.pool.ntp.org", "2.pool.ntp.org"]
        }
    })

    # Post the data. try upto 3 times since the server may not be ready..
    for i in range(1, 4):
        try:
            response = http.httpPost(postUrl, jdata)
        except httplib.BadStatusLine:
            response = 'Error'
        print "Init cluster with " + jdata
        print "cluster response is: " + response
        if response != "Error":
            break
        time.sleep(3)

# Copy credentials to access Kubernetes ApiServer from node1 to node0,
# so that Kubectl and other tools can access the Kubernetes cluster directly.
def copyK8sAccessCredentials():
    tmpDir = tempfile.mkdtemp()
    try:
      runCommand("""docker exec node1 tar Ccf /var/lib/pensando/pki/kubernetes/apiserver-client  - . | tar Cxf {} -""".format(tmpDir))
      runCommand("""docker cp {}/. node0:/root/.kube/auth""".format(tmpDir))
    finally:
      shutil.rmtree(tmpDir)

# Copy credentials to access Elastic cluster from node1 to node0
def copyElasticAccessCredentials():
    tmpDir = tempfile.mkdtemp()
    try:
      runCommand("""docker exec node1 tar Ccf /var/lib/pensando/pki/shared/elastic-client-auth - . | tar Cxf {} -""".format(tmpDir))
      runCommand("""docker exec node0 mkdir -p /var/lib/pensando/pki/shared/elastic-client-auth""")
      runCommand("""docker cp {}/. node0:/var/lib/pensando/pki/shared/elastic-client-auth""".format(tmpDir))
    finally:
      shutil.rmtree(tmpDir)

# Copy credentials to access etcd cluster from node0
def copyEtcdAccessCredentials():
    tmpDir = tempfile.mkdtemp()
    try:
        runCommand("""docker exec node1 tar Ccf /var/lib/pensando/pki/shared/etcd-client-auth - . | tar Cxf {} -""".format(tmpDir))
        runCommand("""docker exec node0 mkdir -p /var/lib/pensando/pki/shared/etcd-client-auth""")
        runCommand("""docker cp {}/. node0:/var/lib/pensando/pki/shared/etcd-client-auth""".format(tmpDir))
    finally:
        shutil.rmtree(tmpDir)

# Copy CMD CA keys from node1 to node0
# E2E tests can use them to mint certificates that give them direct access to cluster endpoints
def copyCMDCAKeys():
    tmpDir = tempfile.mkdtemp()
    try:
        runCommand("""docker exec node1 tar Ccf /var/lib/pensando/pki/pen-cmd/certmgr - . | tar Cxf {} -""".format(tmpDir))
        runCommand("""docker exec node0 mkdir -p /var/lib/pensando/pki/pen-cmd/certmgr""")
        runCommand("""docker cp {}/. node0:/var/lib/pensando/pki/pen-cmd/certmgr""".format(tmpDir))
    finally:
        shutil.rmtree(tmpDir)

def deleteCluster():
    runCommand("""docker stop -t 3 node0 >/dev/null 2>&1""", ignore_error=True)
    runCommand(""" for i in $(docker ps -f label=pens-dind --format '{{.ID}}'); do docker exec $i init 0; done """, ignore_error=True)
    runCommand("""docker stop -t 3 $(docker ps -f label=pens --format '{{.ID}}') >/dev/null 2>&1""", ignore_error=True)
    runCommand("""docker network remove pen-dind-net 2>/dev/null""", ignore_error=True)
    runCommand("""docker network remove pen-dind-hnet 2>/dev/null""", ignore_error=True)
    runCommand("""docker network remove pen-dind-nnet 2>/dev/null""", ignore_error=True)
    runCommand("""docker volume  prune -f 2>/dev/null""", ignore_error=True)
    runCommand("""docker system  prune -f 2>/dev/null""", ignore_error=True)

def stopCluster(nodeList, nodes, quorum, clustervip):
    pool = ThreadPool(len(nodeList))
    pool.map(lambda x: x.stopCluster(), nodes)

def createCluster(nodeList, nodes, init_cluster_nodeIP, quorum, clustervip, nettype):
    dind_net = ipaddr.IPv4Network(ipaddr.IPv4Address(clustervip).__str__() + '/24').masked()
    dind_hnet = ipaddr.IPv4Network((ipaddr.IPv4Address(clustervip) - 256 ).__str__() + '/24').masked()
    dind_nnet = ipaddr.IPv4Network((ipaddr.IPv4Address(clustervip) - 512 ).__str__() + '/24').masked()

    pool = ThreadPool(len(nodes))
    runCommand("""if ! docker network inspect pen-dind-hnet >/dev/null 2>&1; then
        docker network create --internal --ip-range {} --subnet {} pen-dind-hnet
    fi""".format(dind_nnet, dind_nnet))
    runCommand("""if ! docker network inspect pen-dind-nnet >/dev/null 2>&1; then
        docker network create --internal --ip-range {} --subnet {} pen-dind-nnet
    fi""".format(dind_hnet, dind_hnet))
    runCommand("""if ! docker network inspect pen-dind-net >/dev/null 2>&1; then
        docker network create -d {} -o parent=eth1 --ip-range {} --subnet {} pen-dind-net
    fi""".format(nettype, dind_net, dind_net))

    pool.map(lambda x: x.doCluster(), nodes)

    for i in range(1, 3):
        if runCommand("""docker exec node1 /dind/do.py  -configFile '' -init_cluster_only -init_cluster_node {} -quorum {} -clustervip {}""".format(init_cluster_nodeIP, ",".join(quorum), clustervip)) == 0:
            return
        time.sleep(2)
    raise RuntimeException("Unable to create cluster")

def restartCluster(nodeList, nodes, init_cluster_nodeIP, quorum, clustervip):
    pool = ThreadPool(len(nodes))
    pool.map(lambda x: x.restartCluster(), nodes)
    for i in range(1, 3):
        if runCommand("""docker exec node1 /dind/do.py -configFile ''  -init_cluster_only  -init_cluster_node {} -quorum {} -clustervip {}""".format(init_cluster_nodeIP, ",".join(quorum), clustervip)) == 0:
            break
        time.sleep(2)
    else:
        raise RuntimeException("Unable to create cluster")

    if not args.skipnode0:
      copyK8sAccessCredentials()
      copyElasticAccessCredentials()
      copyEtcdAccessCredentials()
      copyCMDCAKeys()

parser = argparse.ArgumentParser()
# these 4 below are used internally not to be directly executed by the caller
parser.add_argument("-clustervip", help="VIP of the cluster")
parser.add_argument("-quorum", type=str, default="", help="quorum nodes joined by ,")
parser.add_argument("-init_cluster_only", action='store_true', default=False, help="Init the cluster by posting Cluster object to CMD and exit")
parser.add_argument("-init_cluster_node", type=str, default="", help="node in which the cluster object is created.")
# args below are to be called by user

# venice_image_dir is when the images are already extracted on the host before calling the script. Hence we use only 1 copy of the files for all the containers
parser.add_argument("-venice_image_dir", type=str, default='', help="path to the extracted files of venice.tgz file. when source is not available. ")

# incase of venice_image, each docker container extracts the venice.tgz and loads all the contents - heavy on disk space and time
parser.add_argument("-venice_image", type=str, default='', help="path to the venice.tgz file. when source is not available. ")

# when the environment does not need to spin up a test node.
parser.add_argument("-skipnode0", action='store_true', default=False, help="skip creation of node0")

# by default docker bridge is created. But can also use macvlan so that ip address and ports are directly exposed off to a physical device
parser.add_argument("-nettype", type=str, default="bridge",help="network type: valid values are bridge, macvlan")
parser.add_argument("-first_venice_ip", type=str, default="192.168.30.11",help="First Venice IP")
parser.add_argument("-first_naples_ip", type=str, default="192.168.30.21",help="First Naples IP")
parser.add_argument("-vc_ip", type=str, default="192.168.30.100",help="vCenter IP")
parser.add_argument("-num_naples", type=int, default=1, help="number of naples nodes")
parser.add_argument("-num_quorum", type=int, default=1, help="number of quorum nodes")
parser.add_argument("-num_nodes", type=int, default=1, help="number of venice nodes")
parser.add_argument("-test_mode", type=str, default="MOCK", help="Specify Agent datapath mode.")
parser.add_argument("-configFile", default="tb_config.json", help="Configuration of the cluster")
parser.add_argument("-custom_config_file", default="bin/venice-conf.json", help="custom Configuration of the cluster")
parser.add_argument("-restart", action='store_true', default=False, help="restart venice components in existing Cluster by loading new Pensando code")
parser.add_argument("-delete", action='store_true', default=False, help="delete cluster by deleting containers")
parser.add_argument("-stop", action='store_true', default=False, help="stop venice cluster but keep containers")
parser.add_argument("-disableTmpfs", action='store_true', default=False, help="dont use tmpfs partition for data")
parser.add_argument("-deployvc", action='store_true', default=False, help="deploy a vc sim container")
args = parser.parse_args()

dev_mode = True
if args.venice_image != '' or args.venice_image_dir != '':
    dev_mode = False

if args.venice_image != '' and args.venice_image_dir != '':
    print "Only one of venice_image or venice_image_dir need to be specified"
    sys.exit(1)

if args.init_cluster_only:
    initCluster(args.init_cluster_node, args.quorum.split(","), args.clustervip)
    sys.exit(0)

if args.nettype != 'bridge' and args.nettype != 'macvlan':
    print "nettype " + args.nettype + " is not support. Only bridge,macvlan are supported"
    sys.exit(0)

if args.delete:
    deleteCluster()
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
num_naples =  int(datastore.get("NumNaplesHosts", args.num_naples))
test_mode = datastore.get("E2EMode", args.test_mode)
first_venice_ipstr = datastore.get("FirstVeniceIP",args.first_venice_ip)
first_venice_ip = ipaddr.IPv4Address(first_venice_ipstr)
first_naples_ipstr = datastore.get("FirstNaplesIP",args.first_naples_ip)
first_naples_ip = ipaddr.IPv4Address(first_naples_ipstr)

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
for i in range(0, num_nodes):
    ipList.append("{}".format(first_venice_ip + i ))

quorumIPs = []
for i in range(0, num_quorum):
    quorumIPs.append("{}".format(first_venice_ip + i ))

naplesIPs = []
for i in range(0, num_naples):
    naplesIPs.append("{}".format(first_naples_ip + i))

# this is a global index of all the containers running in the e2e system
# node0 has containerIndex=0.
# a containerIndex is given to every type of node (venice, naples, e2e)
containerIndex = 1

custom_config_file = None
abspath = os.path.abspath(args.custom_config_file)
if os.path.exists(abspath) and os.path.isfile(abspath):
    custom_config_file = abspath

nodes = []
for addr in xrange(len(nodeList)):
    node = Node(nodeList[addr], ipList[addr], containerIndex, args.nettype, args.venice_image, args.venice_image_dir, dev_mode, custom_config_file, args.disableTmpfs)
    containerIndex = containerIndex + 1
    nodes.append(node)

naplesNodes = []
for addr in xrange(len(naplesNames)):
    node = NaplesNode(naplesNames[addr], naplesIPs[addr], containerIndex, test_mode, clustervip)
    containerIndex = containerIndex + 1
    naplesNodes.append(node)


if args.stop:
    stopCluster(nodeList, nodes + naplesNodes, quorumNames, clustervip)
    sys.exit(0)
if args.restart:
    restartCluster(nodeList, nodes + naplesNodes, ipList[0], quorumNames, clustervip)
    sys.exit(0)

deleteCluster()
testMgmtNode = TestMgmtNode("node0","{}".format(first_venice_ip + num_nodes + num_naples), quorumNames, quorumIPs, nodes, ipList, clustervip, 0, args.nettype, dev_mode)
createCluster(nodeList, nodes + naplesNodes, ipList[0], quorumNames, clustervip, args.nettype)
if not args.skipnode0:
    testMgmtNode.startNode()
    if args.deployvc:
        runCommand("""docker run --dns-search my.dummy -td -p 8989:8989 -l pens --network pen-dind-net --ip {} -v /var/run/docker.sock:/var/run/docker.sock -v sshSecrets:/root/.ssh --privileged --rm --name {} -h {} pen-vcsim:latest /bin/sh """.format(args.vc_ip, "vc", "vc"))
    copyK8sAccessCredentials()
    copyElasticAccessCredentials()
    copyEtcdAccessCredentials()
    copyCMDCAKeys()

sys.exit(0)
