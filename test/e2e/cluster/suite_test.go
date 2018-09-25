package cluster

import (
	"context"
	"flag"
	"fmt"
	"net"
	"os"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"testing"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi/restclient"
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

	// create netagent REST clients
	if ts.tu.FirstNaplesIP != "" {
		agIP := net.ParseIP(ts.tu.FirstNaplesIP).To4()
		Expect(len(agIP)).ShouldNot(Equal(0))
		for idx := 0; idx < ts.tu.NumNaplesHosts; idx++ {
			agURL := agIP.String() + ":" + globals.AgentRESTPort
			By(fmt.Sprintf("ts:%s connecting to netagent [%s]", time.Now().String(), agURL))

			rclient := restclient.NewNetagentClient(agURL)
			Expect(rclient).ShouldNot(Equal(nil))
			ts.netagentClients = append(ts.netagentClients, rclient)
			agIP[3]++
		}
	}
})

var _ = AfterSuite(func() {
	ts.tu.Close()
})
