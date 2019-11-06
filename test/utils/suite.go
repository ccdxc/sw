package utils

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"os/exec"
	"path"
	"strconv"
	"strings"
	"time"

	"github.com/onsi/ginkgo"
	"github.com/onsi/gomega"
	"golang.org/x/crypto/ssh"

	objstore "github.com/pensando/sw/venice/utils/objstore/client"

	"github.com/pensando/sw/api"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/generated/search"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/keymgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
)

const clientName = "test-utils"

// TestBedConfig is Config that can be changed with a json file and environment variables
type TestBedConfig struct {
	NumVeniceNodes int                    `json:",omitempty"`
	NumQuorumNodes int                    `json:",omitempty"`
	NumNaplesHosts int                    `json:",omitempty"`
	ClusterVIP     string                 `json:",omitempty"`
	SSHUser        string                 `json:",omitempty"`
	SSHPasswd      string                 `json:",omitempty"`
	SSHAuthMethod  string                 `json:",omitempty"` // Only password is implemented now. Cert will come later.
	FirstVeniceIP  string                 `json:",omitempty"`
	FirstNaplesIP  string                 `json:",omitempty"`
	SSHPrivKeyFile string                 `json:",omitempty"`
	User           string                 `json:",omitempty"`
	Password       string                 `json:",omitempty"`
	AuthMethod     string                 `json:",omitempty"`
	Radius         testutils.RadiusConfig `json:",omitempty"`
}

var defaultTestBedConfig = TestBedConfig{
	NumQuorumNodes: 3,
	NumVeniceNodes: 5,
	NumNaplesHosts: 2,
	ClusterVIP:     "192.168.30.10",
	SSHUser:        "vagrant",
	SSHPasswd:      "vagrant",
	SSHAuthMethod:  "password",
	FirstVeniceIP:  "192.168.30.11",
	SSHPrivKeyFile: "/root/.ssh/id_rsa",
	User:           "test",
	Password:       "Pensando0$",
	AuthMethod:     auth.Authenticators_LOCAL.String(),
	Radius: testutils.RadiusConfig{
		URL:        "10.11.100.102:1812",
		NasID:      "Venice",
		NasSecret:  "testing123",
		User:       "radiususer",
		Password:   "password",
		Tenant:     "default",
		UserGroups: []string{"Admin"},
	},
}

// TestUtils holds test config, state and any helper caches .
//   some derived fields are exported too to help callers in their tests
type TestUtils struct {
	TestBedConfig

	QuorumNodes   []string // names of Quorum Nodes in cluster
	VeniceNodeIPs []string // IP addresses of venice cluster members
	NaplesNodes   []string // names of NAPLES Nodes in cluster
	NaplesNodeIPs []string // IP addresses of NAPLES nodes
	NameToIPMap   map[string]string
	IPToNameMap   map[string]string

	vIPClient       *ssh.Client
	sshConfig       *ssh.ClientConfig
	client          map[string]*ssh.Client
	resolver        resolver.Interface
	APIGwAddr       string
	CA              *certmgr.CertificateAuthority
	TLSProvider     rpckit.TLSProvider
	APIClient       apiclient.Services
	VOSClient       objstore.Client
	Logger          log.Logger
	VeniceConf      string                  // whole file in string format
	DisabledModules []string                // list of disabled venice modules
	VeniceModules   map[string]*ServiceInfo // list of all the venice modules - disabled modules
}

// ServiceInfo metadata about each service
type ServiceInfo struct {
	DaemonSet bool
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

	ip := net.ParseIP(ts.FirstNaplesIP).To4()
	if ip == nil {
		ginkgo.Fail(fmt.Sprintf("invalid value %s for FirstNaplesIP", ts.FirstNaplesIP))
	}
	for n := 1; n <= ts.NumNaplesHosts; n++ {
		naplesName := fmt.Sprintf("naples%d", n)
		naplesIP := ip.String()
		ts.NaplesNodes = append(ts.NaplesNodes, naplesName)
		ts.NaplesNodeIPs = append(ts.NaplesNodeIPs, naplesIP)
		ts.NameToIPMap[naplesName] = naplesIP
		ts.NameToIPMap[naplesIP] = naplesIP // cluster can also be specified using IP addresses
		ts.IPToNameMap[naplesIP] = naplesName
		ip[3]++
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
	tu.APIGwAddr = tu.ClusterVIP + ":" + globals.APIGwRESTPort
	tu.VeniceConf = tu.CommandOutput(tu.VeniceNodeIPs[0], "bash -c 'if [ -f /etc/pensando/shared/common/venice-conf.json ] ; then  cat /etc/pensando/shared/common/venice-conf.json; fi' ")

	var disabledModules struct {
		DisabledModules []string `json:",omitempty"`
	}
	json.Unmarshal([]byte(tu.VeniceConf), &disabledModules)
	tu.DisabledModules = disabledModules.DisabledModules
	ginkgo.By(fmt.Sprintf("DisabledModules: %+v ", tu.DisabledModules))

	tu.populateVeniceModules()
	ginkgo.By(fmt.Sprintf("VeniceModules: %+v ", tu.VeniceModules))
}

// Initialize a test CA with the same keys used by the CMD CAs, so that the suite can create
// certificates for direct access to cluster endpoints as needed by tests
func (tu *TestUtils) caInit() {
	caBackend, err := keymgr.NewGoCryptoBackend(path.Join(globals.CmdPKIDir, "certmgr"))
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("Error opening CA backend: %v", err))
	}
	caKeyMgr, err := keymgr.NewKeyMgr(caBackend)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("Error creating CA KeyMgr: %v", err))
	}
	ca, err := certmgr.NewCertificateAuthority(caKeyMgr, false)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("Error creating CA KeyMgr: %v", err))
	}
	tu.CA = ca
}

// Initialize TLS provider for accessing cluster endpoints using rpckit
func (tu *TestUtils) tlsProviderInit() {
	tlsProvider, err := tlsproviders.NewDefaultKeyMgrBasedProvider("")
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("Error creating TLS provider: %v", err))
	}
	tlsKeyPair, err := tlsProvider.CreateDefaultKeyPair(keymgr.ECDSA384)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("Error creating default key pair: %v", err))
	}
	csr, err := certs.CreateCSR(tlsKeyPair.Signer, nil, []string{"e2eClusterTest"}, nil)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("Error creating CSR: %v", err))
	}
	tlsCert, err := tu.CA.Sign(csr)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("Error signing CSR: %v", err))
	}
	err = tlsProvider.SetDefaultCertificate(tlsCert)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("Error setting certificate: %v", err))
	}
	tlsProvider.SetCaTrustChain(tu.CA.TrustChain())
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("Error setting trust chain: %v", err))
	}
	tlsProvider.SetTrustRoots(tu.CA.TrustRoots())
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("Error setting trust roots: %v", err))
	}
	tu.TLSProvider = tlsProvider
}

// SetupAuth bootstraps default tenant, authentication policy, local user and super admin role
func (tu *TestUtils) SetupAuth() {
	apicl, err := apiclient.NewRestAPIClient(tu.APIGwAddr)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("cannot create rest client, err: %v", err))
	}
	// create tenant. default roles (admin role) are created automatically when a tenant is created
	_, err = testutils.CreateTenant(apicl, globals.DefaultTenant)
	if err != nil {
		// 412 is returned when tenant and default roles already exist. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(412)") && !strings.HasPrefix(err.Error(), "Status:(401)") {
			ginkgo.Fail(fmt.Sprintf("CreateTenant failed with err: %v", err))
		}
	}
	// create authentication policy with local auth enabled
	_, err = testutils.CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: tu.AuthMethod == auth.Authenticators_LOCAL.String()},
		&auth.Ldap{Enabled: tu.AuthMethod == auth.Authenticators_LDAP.String()})
	if err != nil {
		// 409 is returned when authpolicy already exists. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(409)") && !strings.HasPrefix(err.Error(), "Status:(401)") {
			ginkgo.Fail(fmt.Sprintf("CreateAuthenticationPolicy failed with err: %v", err))
		}
	}
	// create user is only allowed after auth policy is created and local auth is enabled
	_, err = testutils.CreateTestUser(context.TODO(), apicl, tu.User, tu.Password, globals.DefaultTenant)
	if err != nil {
		// 409 is returned when user already exists. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(409)") && !strings.HasPrefix(err.Error(), "Status:(401)") {
			ginkgo.Fail(fmt.Sprintf("CreateTestUser failed with err: %v", err))
		}
	}
	// update admin role binding
	_, err = testutils.UpdateRoleBinding(context.TODO(), apicl, globals.AdminRoleBinding, globals.DefaultTenant, globals.AdminRole, []string{tu.User}, nil)
	if err != nil {
		// 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(401)") {
			ginkgo.Fail(fmt.Sprintf("UpdateRoleBinding failed with err: %v", err))
		}
	}
	// set bootstrap flag
	_, err = testutils.SetAuthBootstrapFlag(apicl)
	if err != nil {
		// 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(401)") {
			ginkgo.Fail(fmt.Sprintf("SetAuthBootstrapFlag failed with err: %v", err))
		}
	}
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

	tu.caInit()
	tu.tlsProviderInit()
	tu.sshInit()

	ginkgo.By(fmt.Sprintf("VeniceNodeIPs: %+v ", tu.VeniceNodeIPs))
	ginkgo.By(fmt.Sprintf("NameToIPMap: %+v", tu.NameToIPMap))
	ginkgo.By(fmt.Sprintf("IPToNameMap: %+v", tu.IPToNameMap))
	ginkgo.By(fmt.Sprintf("APIGwAddr : %+v ", tu.APIGwAddr))

	// We purposefully create Auth at the start of the test and dont delete these policies at the end.
	//  deletion is not possible because as soon as we delete the user, we lose privileges to delete authpolicy.
	//	so - during creation we ignore if the authpolicy/user already exist
	if s := os.Getenv("PENS_SKIP_AUTH"); s == "" {
		tu.SetupAuth()
	}
	ginkgo.By("auth setup complete")

	cmdClient, err := apiclient.NewRestAPIClient(tu.APIGwAddr)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("cannot create API Client err: %v", err))
	}
	clusterIf := cmdClient.ClusterV1().Cluster()
	obj := api.ObjectMeta{Name: "testCluster"}
	var cl *cluster.Cluster
	gomega.Eventually(func() bool {
		cl, err = clusterIf.Get(tu.MustGetLoggedInContext(context.Background()), &obj)
		return err == nil
	}, 45, 2).Should(gomega.BeTrue(), "cluster object should be readable via api gateway but failing with %v", err)

	tu.QuorumNodes = append(tu.QuorumNodes, cl.Spec.QuorumNodes...)
	ginkgo.By(fmt.Sprintf("QuorumNodes: %+v ", tu.QuorumNodes))

	servers := make([]string, 0)
	for _, jj := range tu.QuorumNodes {
		servers = append(servers, fmt.Sprintf("%s:%s", tu.NameToIPMap[jj], globals.CMDResolverPort))
	}
	ginkgo.By(fmt.Sprintf("Resolver servers: %+v ", servers))
	tu.resolver = resolver.New(&resolver.Config{Name: clientName, Servers: servers, Options: []rpckit.Option{rpckit.WithTLSProvider(tu.TLSProvider)}})
	if tu.resolver == nil {
		ginkgo.Fail(fmt.Sprintf("resolver is nil"))
	}

	gomega.Eventually(func() int {
		instList := tu.resolver.Lookup(globals.APIServer)
		return len(instList.Items)
	}, 10, 1).Should(gomega.BeNumerically(">=", 1), "Resolver should have APIServer entry")

	gomega.Eventually(func() int {
		instList := tu.resolver.Lookup(globals.VosMinio)
		return len(instList.Items)
	}, 10, 1).Should(gomega.BeNumerically(">=", 1), "Resolver should have VosMinIO entry")

	gomega.Eventually(func() int {
		instList := tu.resolver.Lookup(globals.Npm)
		return len(instList.Items)
	}, 10, 1).Should(gomega.BeNumerically(">=", 1), "Resolver should have Npm entry")

	gomega.Eventually(func() int {
		instList := tu.resolver.Lookup(globals.Citadel)
		return len(instList.Items)
	}, 180, 10).Should(gomega.BeNumerically(">=", tu.NumQuorumNodes), "Resolver should have Citadel entries")

	gomega.Eventually(func() int {
		instList := tu.resolver.Lookup(globals.ElasticSearch)
		return len(instList.Items)
	}, 10, 1).Should(gomega.BeNumerically(">=", tu.NumQuorumNodes), "Resolver should have ElasticSearch entries")

	ginkgo.By("important services are running")

	// create api server client
	tu.Logger = log.GetNewLogger(log.GetDefaultConfig(clientName))

	tu.APIClient, err = apiclient.NewGrpcAPIClient(clientName, globals.APIServer, tu.Logger, rpckit.WithBalancer(balancer.New(tu.resolver)), rpckit.WithTLSProvider(tu.TLSProvider))
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("cannot create client to apiServer, err: %v", err))
	}
	tu.SetupObjstoreClient()

}

// SetupObjstoreClient sets up the Objstore client
func (tu *TestUtils) SetupObjstoreClient() error {
	var err error
	// Setup the objstore client
	tlcConfig := &tls.Config{
		InsecureSkipVerify: true,
	}
	tu.VOSClient, err = objstore.NewClient("default", "images", tu.resolver, objstore.WithTLSConfig(tlcConfig))
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("cannot create client to objstore, err: %v", err))
	}
	return err
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

	if tu.APIClient != nil {
		tu.APIClient.Close()
		tu.APIClient = nil
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
func (tu *TestUtils) CommandOutput(ip, command string) string {
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

// CommandOutputIgnoreError runs a command on a node and returns output in string format
func (tu *TestUtils) CommandOutputIgnoreError(ip, command string) string {
	c := tu.client[ip]

	session, err := c.NewSession()
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("err : %s", err))
	}

	out, _ := session.CombinedOutput(command)
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

// GetNodeForService returns name of one of the node on which the service is running
func (tu *TestUtils) GetNodeForService(serviceName string) string {
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
	return instList.Items[0].Node
}

// DebugStats fills the statsStruct with the debug stats of serviceName by issuing a REST request to restPort
//	here it assumes that the service is served using a resolver
func (tu *TestUtils) DebugStats(serviceName, restPort string, statsStruct interface{}) {
	tu.DebugStatsOnNode(tu.GetNodeForService(serviceName), restPort, statsStruct)
}

// DebugStatsOnNode posts a REST request to specified port and fills the statsStruct with debug stats
func (tu *TestUtils) DebugStatsOnNode(node string, restPort string, statsStruct interface{}) {
	debugURL := "http://" + node + ":" + restPort + "/debug/vars"
	err := netutils.HTTPGet(debugURL, statsStruct)
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("err : %s", err))
	}
}

// MustGetLoggedInContext authenticates user and returns a new context derived from given context with Authorization header set to JWT. Panics on Failure/Timeout
func (tu *TestUtils) MustGetLoggedInContext(ctx context.Context) context.Context {
	nctx, err := testutils.NewLoggedInContext(ctx, tu.APIGwAddr, &auth.PasswordCredential{Username: tu.User, Password: tu.Password, Tenant: globals.DefaultTenant})
	if err != nil {
		ginkgo.Fail(fmt.Sprintf("err : %s", err))
	}
	return nctx
}

//NewLoggedInContext authenticates user and returns a new context derived from given context with Authorization header set to JWT. Returns error on Failure
func (tu *TestUtils) NewLoggedInContext(ctx context.Context) (context.Context, error) {
	return testutils.NewLoggedInContext(ctx, tu.APIGwAddr, &auth.PasswordCredential{Username: tu.User, Password: tu.Password, Tenant: globals.DefaultTenant})
}

// Search sends a search query to API Gateway
func (tu *TestUtils) Search(ctx context.Context, query *search.SearchRequest, resp interface{}) error {
	return Search(ctx, tu.APIGwAddr, query, resp)
}

// Debug performs a Debug  action to fetch debug logs, profile etc.
func (tu *TestUtils) Debug(ctx context.Context, req *diagnostics.DiagnosticsRequest, resp interface{}) (string, error) {
	return tu.DebugOnAPIGw(ctx, tu.APIGwAddr, req, resp)
}

// DebugOnAPIGw performs a Debug action to fetch debug logs, profile etc on an API Gateway on a specific node
func (tu *TestUtils) DebugOnAPIGw(ctx context.Context, apiGwAddr string, req *diagnostics.DiagnosticsRequest, resp interface{}) (string, error) {
	debugURL := fmt.Sprintf("https://%s/configs/diagnostics/v1/modules/%s/Debug", apiGwAddr, req.Name)
	restcl := netutils.NewHTTPClient()
	restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	restcl.DisableKeepAlives()
	defer restcl.CloseIdleConnections()

	// get authz header
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return "", fmt.Errorf("no authorization header in context")
	}
	restcl.SetHeader("Authorization", authzHeader)
	_, err := restcl.Req("POST", debugURL, req, &resp)
	if err != nil {
		return "", err
	}
	dataBytes, err := json.Marshal(resp)
	if err != nil {
		return "", err
	}
	return string(dataBytes), nil
}

// populates all the venice modules with it's metadata
func (tu *TestUtils) populateVeniceModules() {
	tu.VeniceModules = map[string]*ServiceInfo{
		globals.Cmd:       {true},
		globals.APIGw:     {true},
		globals.APIServer: {false},
		globals.EvtsMgr:   {true},
		globals.EvtsProxy: {true},
		globals.Citadel:   {false},
		globals.Spyglass:  {false},
		globals.Npm:       {false},
		globals.Tpm:       {false},
		globals.Tsm:       {false},
	}

	// remove disabled modules
	for _, m := range tu.DisabledModules {
		delete(tu.VeniceModules, m)
	}
}

// KillContainer finds a container with servicename, kills it, and
// returns the name of the node on which the container was killed
func (tu *TestUtils) KillContainer(serviceName string) (string, error) {
	containerID := ""
	nodeIP := ""
	for _, ip := range tu.VeniceNodeIPs {
		containerID = tu.GetContainerOnNode(ip, serviceName)
		if containerID != "" {
			nodeIP = ip
			break
		}
	}
	if containerID == "" {
		return "", fmt.Errorf("couldn't find container for service %s", serviceName)
	}
	ginkgo.By(fmt.Sprintf("killing service {%s} running on node {%s}", serviceName, nodeIP))
	tu.KillContainerOnNodeByID(nodeIP, containerID)
	return tu.IPToNameMap[nodeIP], nil
}

// KillContainerOnNodeByName finds a container with the serviceName on the given node ip and kills it
func (tu *TestUtils) KillContainerOnNodeByName(ip string, serviceName string) error {
	containerID := tu.GetContainerOnNode(ip, serviceName)
	if containerID == "" {
		return fmt.Errorf("couldn't find container for service %s", serviceName)
	}
	tu.KillContainerOnNodeByID(ip, containerID)
	return nil
}

// GetContainerOnNode returns the container ID for the given service name and node ip
func (tu *TestUtils) GetContainerOnNode(ip string, serviceName string) string {
	return tu.CommandOutput(ip, fmt.Sprintf("docker ps -q -f Name=%s", serviceName))
}

// KillContainerOnNodeByID kills the docker container running on the given node ip
func (tu *TestUtils) KillContainerOnNodeByID(ip string, containerID string) {
	cmd := fmt.Sprintf("docker kill %s > /dev/null", containerID)
	_ = tu.CommandOutputIgnoreError(ip, cmd)
}
