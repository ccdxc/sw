// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package e2e_test

import (
	"os"
	"testing"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/orch"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/rpckit"
	. "github.com/pensando/sw/utils/testutils"
)

const (
	vmsPerHost         = 2
	vchContainerName   = "vcHub"
	vcSimContainerName = "vcSim"
	vchApiAddr         = "pen-master:" + globals.VCHubAPIPort
	defVCSimURL        = "http://pen-master:18086"
	defSoapURL         = "http://user:pass@pen-master:8989/sdk"
	hostSimURL         = "http://192.168.30.121:5050"
	hostMac            = "02:02:02:02:02:01"
	apigwURL           = "pen-master:9000"
)

type e2eSuite struct {
	name           string
	vchubRpcClient *rpckit.RPCClient
	vcHubClient    orch.OrchApiClient
	restClient     apiclient.Services
}

// TestMain is the entry point for e2e tests
func TestMain(m *testing.M) {
	if os.Getenv("E2E_TEST") == "" {
		os.Exit(0)
	}
	os.Exit(m.Run())
}

func (s *e2eSuite) SetUpSuite(c *C) {
	// Open a vch api client
	rpcClient, err := rpckit.NewRPCClient("e2e-test-vchubclient", vchApiAddr)
	c.Assert(err, IsNil)
	s.vchubRpcClient = rpcClient
	s.vcHubClient = orch.NewOrchApiClient(rpcClient.ClientConn)

	// REST Client
	restcl, err := apiclient.NewRestAPIClient(apigwURL)
	if err != nil {
		c.Fatalf("cannot create REST client. Err: %v", err)
	}
	s.restClient = restcl

	// create the default network, if it doesnt exist
	s.createNetwork("default", "default", "10.1.1.0/24", "10.1.1.254")

}

func TestE2E(t *testing.T) {
	if os.Getenv("E2E_TEST") == "" {
		os.Exit(0)
	}

	// test context
	var sts = &e2eSuite{}
	var _ = Suite(sts)

	TestingT(t)
}

// TestVCHBasic tests basic vchub functionality
func (s *e2eSuite) TestVCHBasic(c *C) {
	// make sure vchub has the smart nics
	AssertEventually(c, func() bool {
		nics, err := s.vchubGetNics()
		if err == nil && len(nics) >= 1 {
			return true
		}
		return false
	}, "Error getting smartnic list", "50ms", "10s")

	// Add a NwIF
	resp, err := s.createVM("testNwIF", "10.1.1.1/24", "21", "default", hostMac)
	AssertOk(c, err, "Error creating VM in vcsim")
	log.Infof("VM create Response: %+v", resp)

	// verify VM got created
	AssertEventually(c, func() bool {
		return s.vchubNwifExists("10.1.1.1", hostMac)
	}, "Error getting vm nwifs", "50ms", "10s")

	// delete the ep and verify
	err = s.deleteVM(resp.UUID)
	c.Assert(err, IsNil)

	// verify vm was removed
	AssertEventually(c, func() bool {
		return !s.vchubNwifExists("10.1.1.1", "")
	}, "VM nwif is not deleted", "50ms", "10s")
}

func (s *e2eSuite) TearDownSuite(c *C) {
	s.vchubRpcClient.Close()
}
