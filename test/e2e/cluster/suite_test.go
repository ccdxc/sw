package cluster

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"math/rand"
	"os"
	"strconv"
	"strings"
	"sync"

	"golang.org/x/crypto/ssh"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"testing"
)

func TestE2ETest(t *testing.T) {
	if os.Getenv("E2E_TEST") == "" {
		return
	}
	RegisterFailHandler(Fail)
	RunSpecs(t, "E2e cmd Suite")
}

// the Config that can be changed with a json file and environment variables
type e2eTestBedConfig struct {
	NumVeniceNodes int
	NumQuorumNodes int
	ClusterVIP     string
	SSHUser        string
	SSHPasswd      string
	SSHAuthMethod  string // Only password is implemented now. Cert will come later.
}

var defaultTestBedConfig = e2eTestBedConfig{
	NumQuorumNodes: 3,
	NumVeniceNodes: 5,
	ClusterVIP:     "192.168.30.10",
	SSHUser:        "vagrant",
	SSHPasswd:      "vagrant",
	SSHAuthMethod:  "password",
}

// All the test config, state and any helper caches for running this test
type e2eTestSuite struct {
	e2eTestBedConfig

	sshconfig     *ssh.ClientConfig
	client        map[string]*ssh.Client
	quorumnodes   []string
	veniceNodeIPs []string
	nameToIPMap   map[string]string
	ipToNameMap   map[string]string
	apiGwAddr     string
	vipClient     *ssh.Client

	onceVipClient sync.Once
}

func makeTestSuite() *e2eTestSuite {
	rand.Seed(GinkgoRandomSeed())

	ts := e2eTestSuite{
		client:      make(map[string]*ssh.Client),
		nameToIPMap: make(map[string]string),
		ipToNameMap: make(map[string]string),
	}
	ts.e2eTestBedConfig = defaultTestBedConfig

	// default can be overwritten with a json config
	file, e := ioutil.ReadFile("./e2e_config.json")
	if e == nil {
		json.Unmarshal(file, &ts)
	} else {
		fmt.Printf("cant read e2e_config.json %v", e)
	}

	// Environment variables override defaults, config file
	if s := os.Getenv("PENS_NODES"); s != "" {
		var err error
		ts.NumVeniceNodes, err = strconv.Atoi(s)
		Expect(err).ShouldNot(HaveOccurred())
	}
	return &ts
}

var e2eTest *e2eTestSuite

var _ = BeforeSuite(func() {

	e2eTest = makeTestSuite()

	e2eTest.sshconfig = &ssh.ClientConfig{
		User:            e2eTest.SSHUser,
		Auth:            []ssh.AuthMethod{ssh.Password(e2eTest.SSHPasswd)},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	for i := 0; i < e2eTest.NumQuorumNodes; i++ {
		e2eTest.quorumnodes = append(e2eTest.quorumnodes, "node"+strconv.Itoa(1+i))
	}
	for i := 0; i < e2eTest.NumVeniceNodes; i++ {
		e2eTest.veniceNodeIPs = append(e2eTest.veniceNodeIPs, "192.168.30."+strconv.Itoa(11+i))
	}
	By(fmt.Sprintf("quorumnodes: %+v ", e2eTest.quorumnodes))
	By(fmt.Sprintf("veniceNodeIPs: %+v ", e2eTest.veniceNodeIPs))

	for _, ip := range e2eTest.veniceNodeIPs {
		var err error
		e2eTest.client[ip], err = ssh.Dial("tcp", ip+":22", e2eTest.sshconfig)
		Expect(err).ShouldNot(HaveOccurred())
	}

	for _, ip := range e2eTest.veniceNodeIPs {
		name := commandOutput(ip, "/usr/bin/hostname")

		Expect(e2eTest.nameToIPMap).ShouldNot(HaveKey(name))
		Expect(e2eTest.ipToNameMap).ShouldNot(HaveKey(ip))

		e2eTest.nameToIPMap[name] = ip
		e2eTest.ipToNameMap[ip] = name
	}

	By(fmt.Sprintf("nameToIpMap: %+v", e2eTest.nameToIPMap))
	By(fmt.Sprintf("ipToNameMap: %+v", e2eTest.ipToNameMap))

})
var _ = AfterSuite(func() {
	for _, c := range e2eTest.client {
		c.Close()
	}
	if e2eTest.vipClient != nil {
		e2eTest.vipClient.Close()
		e2eTest.vipClient = nil
	}
})

// IP addresses of non-Quorum nodes. Given in map form for easy lookup
func nonClusterNodes() map[string]interface{} {
	ips := make(map[string]interface{})
	// make a map of ALL IPs
	for _, ip := range e2eTest.veniceNodeIPs {
		ips[ip] = nil
	}
	// remove quorum nodes from the map
	for _, qnode := range e2eTest.quorumnodes {
		ip := e2eTest.nameToIPMap[qnode]
		delete(ips, ip)
	}
	return ips
}
func commandOutput(ip string, command string) string {
	c := e2eTest.client[ip]

	session, err := c.NewSession()
	Expect(err).ShouldNot(HaveOccurred())

	out, err := session.CombinedOutput(command)
	Expect(err).ShouldNot(HaveOccurred())
	return strings.TrimSpace(string(out))
}

func vipCommandOutput(command string) string {
	e2eTest.onceVipClient.Do(func() {
		var err error
		e2eTest.vipClient, err = ssh.Dial("tcp", e2eTest.ClusterVIP+":22", e2eTest.sshconfig)
		Expect(err).ShouldNot(HaveOccurred())
	})
	session, err := e2eTest.vipClient.NewSession()
	Expect(err).ShouldNot(HaveOccurred())
	out, err := session.CombinedOutput(command)
	Expect(err).ShouldNot(HaveOccurred())
	return strings.TrimSpace(string(out))
}

func makeSigner(keyname string) (signer ssh.Signer, err error) {
	fp, err := os.Open(keyname)
	if err != nil {
		return signer, err
	}
	defer fp.Close()

	buf, _ := ioutil.ReadAll(fp)
	signer, _ = ssh.ParsePrivateKey(buf)
	return signer, err
}
