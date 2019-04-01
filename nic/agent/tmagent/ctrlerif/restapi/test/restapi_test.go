package resttest

import (
	"context"
	"os"
	"strings"
	"testing"

	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/testutils"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	protos "github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/nic/agent/netagent/state"
	tmrest "github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi"
	tmstate "github.com/pensando/sw/nic/agent/tmagent/state"
	"github.com/pensando/sw/venice/utils/log"
)

var agentRestURL = "127.0.0.1:"

type Response tmrest.Response

type agentSetup struct {
	cancel  context.CancelFunc
	nagent  *state.Nagent
	naRest  *restapi.RestServer
	tpstate *tmstate.PolicyState
	tmRest  *tmrest.RestServer
}

func TestMain(m *testing.M) {
	ag := agentSetup{}
	err := ag.SetUpSuite()
	if err != nil {
		log.Fatalf("Test set up failed. Error: %v", err)
	}

	agentRestURL = ag.tmRest.GetListenURL()
	testCode := m.Run()
	ag.TearDownSuite()
	os.Exit(testCode)
}

func (ag *agentSetup) SetUpSuite() error {
	ctx, cancel := context.WithCancel(context.Background())
	ag.cancel = cancel

	dp, err := datapath.NewHalDatapath(datapath.Kind("mock"))
	if err != nil {
		log.Errorf("Could not create HAL mock datapath. Error %v", err)
		return err
	}

	// Set tenant creation expectation
	if dp.Kind.String() == "mock" {
		dp.Hal.MockClients.MockTnclient.EXPECT().VrfCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
	}

	ag.nagent, err = state.NewNetAgent(dp, protos.AgentMode_CLASSIC, "")
	if err != nil {
		log.Errorf("Could not create net agent")
		return err
	}

	ag.naRest, err = restapi.NewRestServer(ag.nagent, nil, nil, "127.0.0.1:")
	if err != nil {
		log.Errorf("failed to create rest server, %s", err)
		return err
	}

	log.Printf("net-agent {%+v} instantiated", ag.naRest.GetListenURL())

	port := strings.Split(ag.naRest.GetListenURL(), ":")[1]
	ag.tpstate, err = tmstate.NewTpAgent(ctx, port)
	if err != nil {
		log.Errorf("Error creating telemetry policy agent. Err: %v", err)
		return err
	}

	err = populatePreTestData(ag.nagent)

	if err != nil {
		log.Errorf("Could not meet prerequisites for testing Endpoint CRUD Methods")
		return err
	}

	ag.tmRest, err = tmrest.NewRestServer(ctx, agentRestURL, ag.tpstate)
	if err != nil {
		log.Errorf("failed to create tmagent rest server %s", err)
		return err
	}
	log.Printf("telemetry agent {%+v} instantiated", ag.tmRest.GetListenURL())

	cfg := &resolver.Config{
		Name:    globals.Tmagent,
		Servers: []string{"192.168.1.101"},
	}

	// start tsdb export
	rc := resolver.New(cfg)

	// Init the TSDB
	if err := ag.tpstate.TsdbInit("agent-uuid", rc); err != nil {
		log.Errorf("failed to init tsdb, err: %v", err)
		return err
	}

	go ag.tmRest.ReportMetrics(10)

	return nil
}

func (ag *agentSetup) TearDownSuite() {
	ag.cancel()

	if ag.tmRest != nil {
		ag.tmRest.Stop()
	}

	if ag.tpstate != nil {
		ag.tpstate.Close()
	}

	if ag.naRest != nil {
		ag.naRest.Stop()
	}
}

func populatePreTestData(nagent *state.Nagent) (err error) {
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedNetwork",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.2.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}
	err = nagent.CreateNetwork(&nt)
	if err != nil {
		log.Errorf("Failed to create network. {%v}", nt)
		return
	}

	ep := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "preCreatedNetwork",
			NodeUUID:     "dummy-node-uuid",
			IPv4Address:  "10.1.1.0/24",
			MacAddress:   "4242.4242.4242",
		},
	}
	_, err = nagent.CreateEndpoint(&ep)
	if err != nil {
		log.Errorf("Failed to create endpoint. {%v}", ep)
		return
	}

	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "preCreatedTenant",
			Name:      "preCreatedTenant",
			Namespace: "preCreatedTenant",
		},
	}

	err = nagent.CreateTenant(&tn)
	if err != nil {
		log.Errorf("Failed to create tenant. {%v}", tn)
		return
	}

	ns := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "preCreatedTenant",
			Name:   "preCreatedNamespace",
		},
	}

	err = nagent.CreateNamespace(&ns)
	if err != nil {
		log.Errorf("Failed to create Namespace. {%v}", ns)
		return
	}

	return nil
}

func TestDummy(t *testing.T) {
	err := dummy()
	testutils.AssertOk(t, err, "invalid")
}
