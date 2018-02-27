package utils

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"

	"github.com/onsi/ginkgo"
	"golang.org/x/crypto/ssh"

	"github.com/pensando/sw/api"

	cmdclient "github.com/pensando/sw/api/generated/cmd/grpc/client"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
)

// TestBedConfig is Config that can be changed with a json file and environment variables
type TestBedConfig struct {
	NumVeniceNodes int    `json:",omitempty"`
	NumQuorumNodes int    `json:",omitempty"`
	NumNaplesNodes int    `json:",omitempty"`
	ClusterVIP     string `json:",omitempty"`
	SSHUser        string `json:",omitempty"`
	SSHPasswd      string `json:",omitempty"`
	SSHAuthMethod  string `json:",omitempty"` // Only password is implemented now. Cert will come later.
	FirstVeniceIP  string `json:",omitempty"`
	SSHPrivKeyFile string `json:",omitempty"`
}

var defaultTestBedConfig = TestBedConfig{
	NumQuorumNodes: 3,
	NumVeniceNodes: 5,
	NumNaplesNodes: 2,
	ClusterVIP:     "192.168.30.10",
	SSHUser:        "vagrant",
	SSHPasswd:      "vagrant",
	SSHAuthMethod:  "password",
	FirstVeniceIP:  "192.168.30.11",
	SSHPrivKeyFile: "/root/.ssh/id_rsa",
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
	apiGwAddr string
}

// New creates a new instane of TestUtils. It can be passed a different config (only specifying the fields to be overwritten)
func New(config *TestBedConfig, configFile string) *TestUtils {
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
	if configFile != "" {
		file, e := ioutil.ReadFile(configFile)
		if e == nil {
			json.Unmarshal(file, &ts)
		} else {
			fmt.Printf("cant read configFile %s %v", configFile, e)
		}
		ginkgo.By(fmt.Sprintf("After config file parsing TestBedConfig: %+v", ts.TestBedConfig))
	}

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

func (tu *TestUtils) sshInit() {
	tu.sshConfig = &ssh.ClientConfig{
		User:            tu.SSHUser,
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	var signer ssh.Signer
	key, err := ioutil.ReadFile(tu.SSHPrivKeyFile)
	if err == nil {
		signer, err = ssh.ParsePrivateKey(key)
		if err == nil {
			tu.sshConfig.Auth = append(tu.sshConfig.Auth, ssh.PublicKeys(signer))
		}
	}

	tu.sshConfig.Auth = append(tu.sshConfig.Auth, ssh.Password(tu.SSHPasswd))

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
		tu.NameToIPMap[ip] = ip // cluster can also be specified using IP addresses
		tu.IPToNameMap[ip] = name
	}
	tu.vIPClient, err = ssh.Dial("tcp", tu.ClusterVIP+":22", tu.sshConfig)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("err : %s", err))
	}
	servers := make([]string, 0)
	for _, jj := range tu.VeniceNodeIPs {
		servers = append(servers, fmt.Sprintf("%s:%s", tu.IPToNameMap[jj], globals.CMDClusterMgmtPort))
	}
	ginkgo.By(fmt.Sprintf("resolver servers: %+v ", servers))
	tu.resolver = resolver.New(&resolver.Config{Servers: servers})

	tu.apiGwAddr = tu.ClusterVIP + ":" + globals.APIGwRESTPort

}

// Init starts connecting to the nodes and builds initial data about cluster
func (tu *TestUtils) Init() {
	ip := net.ParseIP(tu.FirstVeniceIP).To4()
	if ip == nil {
		ginkgo.Fail(fmt.Sprintf("invalid value %s for FirstVeniceIP", tu.FirstVeniceIP))
	}
	for i := 0; i < tu.NumVeniceNodes; i++ {
		tu.VeniceNodeIPs = append(tu.VeniceNodeIPs, ip.String())
		ip[3]++
	}

	tu.sshInit()
	ginkgo.By(fmt.Sprintf("VeniceNodeIPs: %+v ", tu.VeniceNodeIPs))
	ginkgo.By(fmt.Sprintf("NameToIPMap: %+v", tu.NameToIPMap))
	ginkgo.By(fmt.Sprintf("IPToNameMap: %+v", tu.IPToNameMap))

	var err error
	if tu.resolver == nil {
		ginkgo.Fail(fmt.Sprintf("resolver is nil"))
	}

	ginkgo.By(fmt.Sprintf("apiGwAddr : %+v ", tu.apiGwAddr))
	cmdClient := cmdclient.NewRestCrudClientCmdV1(tu.apiGwAddr)
	clusterIf := cmdClient.Cluster()
	obj := api.ObjectMeta{Name: "testCluster"}
	cl, err := clusterIf.Get(context.Background(), &obj)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("cluster Get err : %s", err))
	}
	for _, qn := range cl.Spec.QuorumNodes {
		tu.QuorumNodes = append(tu.QuorumNodes, qn)
	}
	ginkgo.By(fmt.Sprintf("QuorumNodes: %+v ", tu.QuorumNodes))
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
		var ip string
		// if QuorumNodes are names then lookup
		ipAddr := net.ParseIP(qnode)
		if ipAddr == nil {
			ip = tu.NameToIPMap[qnode]
		} else {
			ip = ipAddr.String()
		}
		delete(ips, ip)
	}
	return ips
}

// LocalCommandOutput runs a command on a node and returns output in string format
func (tu *TestUtils) LocalCommandOutput(command string) string {
	out, _ := exec.Command("bash", "-c", command).CombinedOutput()
	return strings.TrimSpace(string(out))
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
		ginkgo.Fail(fmt.Sprintf("command %s on %s failed with err : %s after output: %s", command, ip, err.Error(), string(out)))
	}
	return strings.TrimSpace(string(out))
}

// VIPCommandOutput runs a command on the VIP node of cluster and returns output in string format
func (tu *TestUtils) VIPCommandOutput(command string) string {
	session, err := tu.vIPClient.NewSession()
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("err : %s", err))
	}
	ginkgo.By("Executing " + command)
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
