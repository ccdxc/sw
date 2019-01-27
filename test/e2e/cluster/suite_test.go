package cluster

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
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"testing"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi/restclient"
	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
)

var configFile string

func init() {
	cfgFile := flag.String("configFile", "./tb_config.json", "Path to JSON Config file describing testbed")
	flag.Parse()
	configFile = *cfgFile
}

func TestE2ETest(t *testing.T) {
	if os.Getenv("E2E_TEST") == "" {
		return
	}
	RegisterFailHandler(Fail)
	RunSpecs(t, "E2e cmd Suite")
}

// All the test config, state and any helper caches for running this test
type TestSuite struct {
	tu              *testutils.TestUtils
	restSvc         apiclient.Services
	netagentClients []*restclient.NetagentClient
	loggedInCtx     context.Context
}

var ts *TestSuite

var _ = BeforeSuite(func() {
	var err error

	ts = &TestSuite{
		tu: testutils.New(nil, configFile),
	}
	ts.tu.Init()

	// get auth context
	ts.loggedInCtx = ts.tu.NewLoggedInContext(context.Background())

	// create apigw workload client
	apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
	ts.restSvc, err = apiclient.NewRestAPIClient(apiGwAddr)
	Expect(err).ShouldNot(HaveOccurred())

	if s := os.Getenv("PENS_SKIP_BOOTSTRAP"); s == "" && ts.tu.FirstNaplesIP != "" { // create netagent REST clients
		agIP := net.ParseIP(ts.tu.FirstNaplesIP).To4()
		Expect(len(agIP)).ShouldNot(Equal(0))
		for idx := 0; idx < ts.tu.NumNaplesHosts; idx++ {
			agURL := agIP.String() + ":" + globals.AgentRESTPort
			By(fmt.Sprintf("ts:%s connecting to netagent [%s]", time.Now().String(), agURL))

			rclient := restclient.NewNetagentClient(agURL)
			Expect(rclient).ShouldNot(Equal(nil))
			ts.netagentClients = append(ts.netagentClients, rclient)

			var naples nmd.Naples
			nmdURL := "http://" + agIP.String() + ":" + globals.RevProxyPort + "/api/v1/naples/"
			By(fmt.Sprintf("Getting Naples object from %v", nmdURL))

			resp, err := http.Get(nmdURL)
			Expect(err).ShouldNot(HaveOccurred())
			data, err := ioutil.ReadAll(resp.Body)
			Expect(err).ShouldNot(HaveOccurred())
			fmt.Println("Got Naples Response: ", string(data))
			err = json.Unmarshal(data, &naples)
			Expect(err).ShouldNot(HaveOccurred())
			resp.Body.Close()

			By(fmt.Sprintf("Creating host obj naples%d-host", idx+1))
			// Create a Host object
			host := &cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: fmt.Sprintf("naples%d-host", idx+1),
				},
				Spec: cluster.HostSpec{
					SmartNICs: []cluster.SmartNICID{
						{
							MACAddress: naples.Spec.PrimaryMAC,
						},
					},
				},
			}
			_, err = ts.restSvc.ClusterV1().Host().Create(ts.tu.NewLoggedInContext(context.Background()), host)
			Expect(err).ShouldNot(HaveOccurred())
			// Switch to managed mode
			naples.Spec.Mode = nmd.MgmtMode_NETWORK
			naples.Spec.Controllers = []string{ts.tu.ClusterVIP}
			naples.Spec.NetworkMode = nmd.NetworkMode_INBAND
			// Ensure that a random static IP is given
			naples.Spec.IPConfig = &cluster.IPConfig{
				IPAddress: "1.2.3.4",
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
