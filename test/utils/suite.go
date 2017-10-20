package utils

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/onsi/ginkgo"
	"golang.org/x/crypto/ssh"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
)

// TestBedConfig is Config that can be changed with a json file and environment variables
type TestBedConfig struct {
	NumVeniceNodes int    `json:",omitempty"`
	NumQuorumNodes int    `json:",omitempty"`
	ClusterVIP     string `json:",omitempty"`
	SSHUser        string `json:",omitempty"`
	SSHPasswd      string `json:",omitempty"`
	SSHAuthMethod  string `json:",omitempty"` // Only password is implemented now. Cert will come later.
}

var defaultTestBedConfig = TestBedConfig{
	NumQuorumNodes: 3,
	NumVeniceNodes: 5,
	ClusterVIP:     "192.168.30.10",
	SSHUser:        "vagrant",
	SSHPasswd:      "vagrant",
	SSHAuthMethod:  "password",
}

// TestUtils holds test config, state and any helper caches .
//   some derived fields are exported too to help callers in their tests
type TestUtils struct {
	TestBedConfig

	QuorumNodes   []string // names of Quorum Nodes in cluster
	VeniceNodeIPs []string // IP addresses of venice cluster members
	NameToIPMap   map[string]string
	IPToNameMap   map[string]string

	vIPClient *ssh.Client
	sshConfig *ssh.ClientConfig
	client    map[string]*ssh.Client
	resolver  resolver.Interface
}

// New creates a new instane of TestUtils. It can be passed a different config (only specifying the fields to be overwritten)
func New(config *TestBedConfig) *TestUtils {
	ts := TestUtils{
		client:      make(map[string]*ssh.Client),
		NameToIPMap: make(map[string]string),
		IPToNameMap: make(map[string]string),
	}
	ts.TestBedConfig = defaultTestBedConfig
	ginkgo.By(fmt.Sprintf("TestBedConfig: %+v", ts.TestBedConfig))

	// Any non-nil stuff from per-testsuite config overrides the global default testbed config
	if config != nil {
		bytes, err := json.Marshal(config)
		if err != nil {
			ginkgo.Fail(fmt.Sprintf("err : %s", err))
		}
		json.Unmarshal(bytes, &ts)
	}
	ginkgo.By(fmt.Sprintf("TestBedConfig: %+v", ts.TestBedConfig))

	// which  can be overwritten with a json config
	file, e := ioutil.ReadFile("./tb_config.json")
	if e == nil {
		json.Unmarshal(file, &ts)
	} else {
		fmt.Printf("cant read tb_config.json %v", e)
	}
	ginkgo.By(fmt.Sprintf("TestBedConfig: %+v", ts.TestBedConfig))

	// Environment variables take highest precedence and overwrite previous config
	if s := os.Getenv("PENS_NODES"); s != "" {
		s2 := strings.Split(s, ",")
		ts.NumVeniceNodes = len(s2)
	}

	if s := os.Getenv("PENS_QUORUM_NODES"); s != "" {
		var err error
		ts.NumQuorumNodes, err = strconv.Atoi(s)
		if err != nil {
			ginkgo.Fail(fmt.Sprintf("err : %s", err))
		}
	}
	ginkgo.By(fmt.Sprintf("TestBedConfig: %+v", ts.TestBedConfig))

	return &ts
}

// Init starts connecting to the nodes and builds initial data about cluster
func (tu *TestUtils) Init() {
	tu.sshConfig = &ssh.ClientConfig{
		User:            tu.SSHUser,
		Auth:            []ssh.AuthMethod{ssh.Password(tu.SSHPasswd)},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	// TODO: Get this from Cluster Object instead of creating it locally
	for i := 0; i < tu.NumQuorumNodes; i++ {
		tu.QuorumNodes = append(tu.QuorumNodes, "node"+strconv.Itoa(1+i))
	}
	clusterIPPrefix := tu.ClusterVIP[0 : strings.LastIndex(tu.ClusterVIP, ".")+1]
	for i := 0; i < tu.NumVeniceNodes; i++ {
		tu.VeniceNodeIPs = append(tu.VeniceNodeIPs, clusterIPPrefix+strconv.Itoa(11+i))
	}
	ginkgo.By(fmt.Sprintf("QuorumNodes: %+v ", tu.QuorumNodes))
	ginkgo.By(fmt.Sprintf("VeniceNodeIPs: %+v ", tu.VeniceNodeIPs))

	for _, ip := range tu.VeniceNodeIPs {
		var err error
		tu.client[ip], err = ssh.Dial("tcp", ip+":22", tu.sshConfig)
		if err != nil {
			ginkgo.Fail(fmt.Sprintf("err : %s", err))
		}
	}

	for _, ip := range tu.VeniceNodeIPs {
		name := tu.CommandOutput(ip, "/usr/bin/hostname")

		_, prs := tu.NameToIPMap[name]
		if prs {
			ginkgo.Fail(fmt.Sprintf("name:%s present in NameToIPMap when it should not", name))
		}
		_, prs = tu.IPToNameMap[ip]
		if prs {
			ginkgo.Fail(fmt.Sprintf("ip:%s present in IPToNameMap when it should not", ip))
		}

		tu.NameToIPMap[name] = ip
		tu.IPToNameMap[ip] = name
	}

	ginkgo.By(fmt.Sprintf("NameToIpMap: %+v", tu.NameToIPMap))
	ginkgo.By(fmt.Sprintf("IPToNameMap: %+v", tu.IPToNameMap))

	var err error
	tu.vIPClient, err = ssh.Dial("tcp", tu.ClusterVIP+":22", tu.sshConfig)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("err : %s", err))
	}

	servers := make([]string, 0)
	for _, jj := range tu.VeniceNodeIPs {
		servers = append(servers, fmt.Sprintf("%s:%s", jj, globals.CMDGRPCPort))
	}
	tu.resolver = resolver.New(&resolver.Config{Servers: servers})
	if tu.resolver == nil {
		ginkgo.Fail(fmt.Sprintf("resolver is nil"))
	}

}

// Close any open connections to nodes
func (tu *TestUtils) Close() {
	if tu.resolver != nil {
		tu.resolver.Stop()
		tu.resolver = nil
	}

	for _, c := range tu.client {
		c.Close()
	}
	if tu.vIPClient != nil {
		tu.vIPClient.Close()
		tu.vIPClient = nil
	}
}

// NonQuorumNodes returns IP addresses of non-Quorum nodes in map form for easy lookup
func (tu *TestUtils) NonQuorumNodes() map[string]interface{} {
	ips := make(map[string]interface{})
	// make a map of ALL IPs
	for _, ip := range tu.VeniceNodeIPs {
		ips[ip] = nil
	}
	// remove quorum nodes from the map
	for _, qnode := range tu.QuorumNodes {
		ip := tu.NameToIPMap[qnode]
		delete(ips, ip)
	}
	return ips
}

// CommandOutput runs a command on a node and returns output in string format
func (tu *TestUtils) CommandOutput(ip string, command string) string {
	c := tu.client[ip]

	session, err := c.NewSession()
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("err : %s", err))
	}

	out, err := session.CombinedOutput(command)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("err : %s", err))
	}
	return strings.TrimSpace(string(out))
}

// VIPCommandOutput runs a command on the VIP node of cluster and returns output in string format
func (tu *TestUtils) VIPCommandOutput(command string) string {
	session, err := tu.vIPClient.NewSession()
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("err : %s", err))
	}
	out, err := session.CombinedOutput(command)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("err : %s", err))
	}
	return strings.TrimSpace(string(out))
}

// DebugStats fills the statsStruct with the debug stats of serviceName by issuing a REST request to restPort
//	here it assumes that the service is served using a resolver
func (tu *TestUtils) DebugStats(serviceName string, restPort string, statsStruct interface{}) {
	instList := tu.resolver.Lookup(serviceName)
	try := 0
	for len(instList.Items) == 0 && try < 20 {
		ginkgo.By(fmt.Sprintf("Sleeping 100ms waiting for resolver"))
		time.Sleep(100 * time.Millisecond)
		instList = tu.resolver.Lookup(serviceName)
		try++
	}
	if try >= 20 {
		ginkgo.Fail(fmt.Sprintf("resolver lookup for service:%s is empty after %d tries", serviceName, try))
	}
	node := instList.Items[0].Node

	tu.DebugStatsOnNode(node, restPort, statsStruct)
}

// DebugStatsOnNode posts a REST request to specified port and fills the statsStruct with debug stats
func (tu *TestUtils) DebugStatsOnNode(node string, restPort string, statsStruct interface{}) {
	debugURL := "http://" + node + ":" + restPort + "/debug/vars"
	err := netutils.HTTPGet(debugURL, statsStruct)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("err : %s", err))
	}
}
