package cluster

import (
	"bytes"
	"context"
	"crypto/tls"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"strings"
	"time"

	"github.com/pensando/sw/venice/utils/netutils"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"testing"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/protos/generated/restclient"
	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
)

var configFile = flag.String("configFile", "./tb_config.json", "Path to JSON Config file describing testbed")

const (
	insertionFWProfileName = "InsertionFWProfile"
	wildCardAudience       = "*"
)

func TestE2ETest(t *testing.T) {
	if os.Getenv("E2E_TEST") == "" {
		return
	}
	RegisterFailHandler(Fail)
	s := NewReporter()
	RunSpecsWithDefaultAndCustomReporters(t, "E2e cmd Suite", []Reporter{s})
}

// All the test config, state and any helper caches for running this test
type TestSuite struct {
	tu                 *testutils.TestUtils
	restSvc            apiclient.Services
	netagentClients    []*restclient.AgentClient
	loggedInCtx        context.Context
	naplesNameToMACMap map[string]string
}

var ts *TestSuite

var _ = BeforeSuite(func() {
	var err error

	flag.Parse()
	ts = &TestSuite{
		tu:                 testutils.New(nil, *configFile),
		naplesNameToMACMap: make(map[string]string),
	}
	ts.tu.Init()

	// get auth context
	ts.loggedInCtx = ts.tu.MustGetLoggedInContext(context.Background())

	// create apigw workload client
	apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
	ts.restSvc, err = apiclient.NewRestAPIClient(apiGwAddr)
	Expect(err).ShouldNot(HaveOccurred())

	if s := os.Getenv("PENS_SKIP_BOOTSTRAP"); s == "" && ts.tu.FirstNaplesIP != "" { // create netagent REST clients
		// Create a DSC profile for firewall
		insertionFWProfile := &cluster.DSCProfile{}
		insertionFWProfile.Defaults("all")
		insertionFWProfile.Name = insertionFWProfileName
		insertionFWProfile.Spec.FwdMode = cluster.DSCProfileSpec_INSERTION.String()
		insertionFWProfile.Spec.FlowPolicyMode = cluster.DSCProfileSpec_ENFORCED.String()
		Eventually(func() bool {
			ctx, cancel := context.WithTimeout(ts.loggedInCtx, 10*time.Second)
			_, err := ts.restSvc.ClusterV1().DSCProfile().Create(ctx, insertionFWProfile)
			cancel()
			if err != nil {
				By(fmt.Sprintf("failed to create firewall profile, err: %v", err))
				return false
			}
			return true
		}, 90, 10).Should(BeTrue(), "Failed to create firewall profile")

		agIP := net.ParseIP(ts.tu.FirstNaplesIP).To4()
		Expect(len(agIP)).ShouldNot(Equal(0))
		for idx := 0; idx < ts.tu.NumNaplesHosts; idx++ {
			// nmd shuts down all interfaces in host mode.
			// bring up oob_mnic0 to send mode switch
			gw := net.ParseIP(ts.tu.NaplesNodeIPs[idx])
			gw[len(gw)-1] = 1 // 1st ip is used as gateway
			By(fmt.Sprintf("setting gateway:%v", gw))
			nodeName := ts.tu.NaplesNodes[idx]
			st := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s ip link set oob_mnic0 up", nodeName))
			Expect(st).Should(Equal(""))
			time.Sleep(time.Second)
			st = ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s route add default gw %v oob_mnic0", nodeName, gw.String()))
			Expect(st).Should(Equal(""))
			// netagent expects bond0 to be available and with an assigned IP
			st = ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s ip link add link oob_mnic0 name bond0 type dummy", nodeName))
			Expect(st).Should(Equal(""))
			time.Sleep(time.Second)
			st = ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s ifconfig bond0 %s netmask 255.255.255.0", nodeName, agIP.String()))
			Expect(st).Should(Equal(""))
			// remove route to avoid interference with oob_mnic0 // FIXME remove hardcoded network
			st = ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s route del -net 192.168.30.0 netmask 255.255.255.0 dev bond0", nodeName))
			Expect(st).Should(Equal(""))

			agURL := agIP.String() + ":" + globals.AgentProxyPort
			By(fmt.Sprintf("ts:%s connecting to netagent [%s]", time.Now().String(), agURL))

			rclient := restclient.NewAgentClient(agURL)
			Expect(rclient).ShouldNot(Equal(nil))
			ts.netagentClients = append(ts.netagentClients, rclient)

			var naples nmd.DistributedServiceCard
			nmdURL := "https://" + agIP.String() + ":" + globals.AgentProxyPort + "/api/v1/naples/"
			By(fmt.Sprintf("Getting Naples object from %v", nmdURL))

			transport := &http.Transport{TLSClientConfig: &tls.Config{InsecureSkipVerify: true}}
			client := &http.Client{Transport: transport}
			resp, err := client.Get(nmdURL)
			Expect(err).ShouldNot(HaveOccurred())
			data, err := ioutil.ReadAll(resp.Body)
			Expect(err).ShouldNot(HaveOccurred())
			fmt.Println("Got Naples Response: ", string(data))
			err = json.Unmarshal(data, &naples)
			Expect(err).ShouldNot(HaveOccurred())
			resp.Body.Close()

			// cache name:mac mapping
			nodeMAC := naples.Status.Fru.MacStr
			ts.naplesNameToMACMap[nodeName] = nodeMAC

			By(fmt.Sprintf("Creating host obj naples%d-host", idx+1))
			// Create a Host object
			host := &cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: fmt.Sprintf("naples%d-host", idx+1),
				},
				Spec: cluster.HostSpec{
					DSCs: []cluster.DistributedServiceCardID{
						{
							MACAddress: nodeMAC,
						},
					},
				},
			}
			_, err = ts.restSvc.ClusterV1().Host().Create(ts.tu.MustGetLoggedInContext(context.Background()), host)
			Expect(err).ShouldNot(HaveOccurred())

			// verify authorization in Host mode prior to switching to Network mode
			verifyProtectedCommandRequests(nodeName, nodeMAC)

			// Switch to managed mode
			naples.Spec.Mode = nmd.MgmtMode_NETWORK.String()
			naples.Spec.Controllers = []string{ts.tu.ClusterVIP}
			naples.Spec.ID = fmt.Sprintf("naples-%d", idx)
			naples.Spec.NetworkMode = nmd.NetworkMode_OOB.String()
			// Ensure that a random static IP is given
			naples.Spec.IPConfig = &cluster.IPConfig{
				IPAddress: agIP.String() + "/24",
				DefaultGW: gw.String(),
			}
			By(fmt.Sprintf("Switching Naples %+v to managed mode", naples))
			out, err := json.Marshal(&naples)
			Expect(err).ShouldNot(HaveOccurred())
			_, err = client.Post(nmdURL, "application/json", bytes.NewReader(out))
			Expect(err).ShouldNot(HaveOccurred())

			// Point the DSC to firewall profile
			// The DSC object must have been created, but we don't need to wait until the card is admitted.
			Eventually(func() bool {
				ctx, cancel := context.WithTimeout(ts.loggedInCtx, 30*time.Second)
				err := ts.tu.SetDSCProfile(ctx, &api.ObjectMeta{Name: naples.Status.Fru.MacStr}, insertionFWProfileName)
				cancel()
				if err != nil {
					By(fmt.Sprintf("Error setting profile %s for DSC object %s: %v", insertionFWProfile, naples.Spec.ID, err))
					return false
				}
				return true
			}, 90, 10).Should(BeTrue(), fmt.Sprintf("Error setting profile for DSC %s", naples.Spec.ID))

			agIP[3]++
		}
	}
})

func verifyProtectedCommandRequests(nodeName string, nodeMAC string) {
	verifyCommandRequestsWithoutCert(nodeName, nodeMAC, "getdate", false)
	verifyCommandRequestsWithoutCert(nodeName, nodeMAC, "mksshdir", true)
	audienceToTokenMap := map[string]string{
		wildCardAudience: "/import/src/github.com/pensando/sw/test/e2e/cluster/tokens/penctl_hmm_wildcard.token",
		"00ae.cd00.112a": "/import/src/github.com/pensando/sw/test/e2e/cluster/tokens/penctl_hmm_mac.token",
		"00ae.cd00.0000": "/import/src/github.com/pensando/sw/test/e2e/cluster/tokens/penctl_hmm_random_mac.token",
	}
	for audience, tokenFilePath := range audienceToTokenMap {
		verifyCommandRequestsToDSCInHostMode(nodeName, nodeMAC, "getdate", false, tokenFilePath, audience)
		verifyCommandRequestsToDSCInHostMode(nodeName, nodeMAC, "mksshdir", true, tokenFilePath, audience)
	}
}

func verifyCommandRequestsWithoutCert(nodeName string, nodeMAC string, command string, isProtected bool) {
	verifyCommandRequestsToDSCInHostMode(nodeName, nodeMAC, command, isProtected, "", "")
}

func verifyCommandRequestsToDSCInHostMode(nodeName string, nodeMAC string, command string, isProtected bool, tokenFilePath string, audience string) {
	var clientCerts []tls.Certificate
	isCertProvided := tokenFilePath != ""
	if isCertProvided {
		pemBlocks, err := ioutil.ReadFile(tokenFilePath)
		Expect(err).ShouldNot(HaveOccurred())
		cert, err := tls.X509KeyPair(pemBlocks, pemBlocks)
		Expect(err).ShouldNot(HaveOccurred())
		clientCerts = []tls.Certificate{cert}
	}
	dscCommand := &nmd.DistributedServiceCardCmdExecute{
		Executable: command,
		Opts:       strings.Join([]string{""}, ""),
	}
	tlsConfig := &tls.Config{
		InsecureSkipVerify: true, // do not check agent's certificate
		Certificates:       clientCerts,
	}

	client := netutils.NewHTTPClient()
	client.WithTLSConfig(tlsConfig)
	client.DisableKeepAlives()
	defer client.CloseIdleConnections()

	responseRaw, responseStatus, err := client.ReqRaw("GET", fmt.Sprintf("https://%s:%s/cmd/v1/naples/", ts.tu.NameToIPMap[nodeName], globals.AgentProxyPort), dscCommand)
	By(fmt.Sprintf("ts:%s Host mode verification on naples node:%s for request command:%s with audience:%s, response message: %s error: %+v", time.Now().String(), nodeName, command, audience, string(responseRaw), err))

	if isProtected {
		if isCertProvided {
			if audience != wildCardAudience && nodeMAC != audience {
				// request should not be allowed
				Expect(responseStatus).Should(Equal(http.StatusUnauthorized))
			} else {
				// request should be allowed
				Expect(responseStatus).ShouldNot(Equal(http.StatusUnauthorized))
			}
		} else {
			// request should not be allowed
			Expect(responseStatus).Should(Equal(http.StatusUnauthorized))
		}
	} else {
		// request should be allowed
		Expect(responseStatus).ShouldNot(Equal(http.StatusUnauthorized))
	}
}

var _ = AfterSuite(func() {
	ts.tu.Close()
})
