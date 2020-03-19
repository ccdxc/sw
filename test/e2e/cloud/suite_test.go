package cloud

import (
	"bytes"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"testing"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/protos/generated/restclient"
	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
)

var configFile = flag.String("configFile", "./tb_config_cp.json", "Path to JSON Config file describing testbed")

func TestE2ETest(t *testing.T) {
	if os.Getenv("E2E_TEST") == "" {
		return
	}
	RegisterFailHandler(Fail)
	s := NewReporter()
	RunSpecsWithDefaultAndCustomReporters(t, "Cloud E2E  Suite", []Reporter{s})
}

// All the test config, state and any helper caches for running this test
type TestSuite struct {
	tu              *testutils.TestUtils
	restSvc         apiclient.Services
	netagentClients []*restclient.AgentClient
	loggedInCtx     context.Context
}

var ts *TestSuite

var _ = BeforeSuite(func() {
	var err error

	flag.Parse()
	ts = &TestSuite{
		tu: testutils.New(nil, *configFile),
	}
	ts.tu.Init()

	// get auth context
	ts.loggedInCtx = ts.tu.MustGetLoggedInContext(context.Background())

	// create apigw workload client
	apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
	ts.restSvc, err = apiclient.NewRestAPIClient(apiGwAddr)
	Expect(err).ShouldNot(HaveOccurred())

	if s := os.Getenv("PENS_SKIP_BOOTSTRAP"); s == "" && ts.tu.FirstNaplesIP != "" { // create netagent REST clients
		agIP := net.ParseIP(ts.tu.FirstNaplesIP).To4()
		Expect(len(agIP)).ShouldNot(Equal(0))
		for idx := 0; idx < ts.tu.NumNaplesHosts; idx++ {
			// nmd shuts down all interfaces in host mode.
			// bring up oob_mnic0 to send mode switch
			gw := net.ParseIP(ts.tu.NaplesNodeIPs[idx])
			gw[len(gw)-1] = 1 // 1st ip is used as gateway
			By(fmt.Sprintf("setting gateway:%v", gw))
			st := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s ip link set oob_mnic0 up", ts.tu.NaplesNodes[idx]))
			Expect(st).Should(Equal(""))
			time.Sleep(time.Second)
			st = ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s route add default gw %v oob_mnic0", ts.tu.NaplesNodes[idx], gw.String()))
			Expect(st).Should(Equal(""))

			agURL := agIP.String() + ":" + globals.AgentProxyPort
			By(fmt.Sprintf("ts:%s connecting to netagent [%s]", time.Now().String(), agURL))

			rclient := restclient.NewAgentClient(agURL)
			Expect(rclient).ShouldNot(Equal(nil))
			ts.netagentClients = append(ts.netagentClients, rclient)

			var naples nmd.DistributedServiceCard
			nmdURL := "http://" + agIP.String() + ":" + globals.AgentProxyPort + "/api/v1/naples/"
			By(fmt.Sprintf("Getting Naples object from %v", nmdURL))

			resp, err := http.Get(nmdURL)
			Expect(err).ShouldNot(HaveOccurred())
			data, err := ioutil.ReadAll(resp.Body)
			Expect(err).ShouldNot(HaveOccurred())
			fmt.Println("Got Naples Response: ", string(data))
			err = json.Unmarshal(data, &naples)
			Expect(err).ShouldNot(HaveOccurred())
			resp.Body.Close()
			By(fmt.Sprintf("Naples [naples%d] MAC address [%v]-host", idx+1, naples.Spec.PrimaryMAC))
			By(fmt.Sprintf("Naples [naples%d] IP address [%v]-host", idx+1, agIP.String()))

			By(fmt.Sprintf("Creating host obj naples%d-host", idx+1))
			// Create a Host object
			host := &cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: fmt.Sprintf("naples%d-host", idx+1),
				},
				Spec: cluster.HostSpec{
					DSCs: []cluster.DistributedServiceCardID{
						{
							MACAddress: naples.Status.Fru.MacStr,
						},
					},
				},
			}
			_, err = ts.restSvc.ClusterV1().Host().Create(ts.tu.MustGetLoggedInContext(context.Background()), host)
			Expect(err).ShouldNot(HaveOccurred())
			// Switch to managed mode
			naples.Spec.Mode = nmd.MgmtMode_NETWORK.String()
			naples.Spec.Controllers = ts.tu.VeniceNodeIPs
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
			_, err = http.Post(nmdURL, "application/json", bytes.NewReader(out))
			Expect(err).ShouldNot(HaveOccurred())

			agIP[3]++
		}
	}
})

var _ = AfterSuite(func() {
	ts.tu.Close()
})
