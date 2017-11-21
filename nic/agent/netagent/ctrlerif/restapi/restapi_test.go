package restapi

import (
	"flag"
	"os"
	"testing"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

func TestMain(m *testing.M) {
	srv, err := setup()
	if err != nil {
		log.Fatalf("Test set up failed. Error: %v", err)
	}
	testCode := m.Run()
	srv.Stop()
	os.Exit(testCode)
}

const (
	agentRestURL      = "localhost:1337"
	agentDatapathKind = "mock"
)

var datapathKind = flag.String("datapath", agentDatapathKind, "Specify the datapath type. mock | hal")

func setup() (*RestServer, error) {

	dp, err := datapath.NewHalDatapath(datapath.Kind(*datapathKind))
	if err != nil {
		log.Errorf("Could not create HAL datapath. Kind: %v, Error %v", datapathKind, err)
		return nil, err
	}

	nagent, err := state.NewNetAgent(dp, "", "dummy-node-uuid")
	if err != nil {
		log.Errorf("Could not create net agent")
		return nil, err
	}

	if dp.Kind.String() == "mock" {
		dp.Hal.MockClients.MockNetclient.EXPECT().L2SegmentCreate(gomock.Any(), gomock.Any()).MaxTimes(3).Return(nil, nil)
		dp.Hal.MockClients.MockNetclient.EXPECT().L2SegmentUpdate(gomock.Any(), gomock.Any()).Return(nil, nil)
		dp.Hal.MockClients.MockNetclient.EXPECT().L2SegmentDelete(gomock.Any(), gomock.Any()).Return(nil, nil)

		dp.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupCreate(gomock.Any(), gomock.Any()).MaxTimes(3).Return(nil, nil)
		dp.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupUpdate(gomock.Any(), gomock.Any()).Return(nil, nil)
		dp.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupDelete(gomock.Any(), gomock.Any()).Return(nil, nil)

		dp.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyCreate(gomock.Any(), gomock.Any()).MaxTimes(3).Return(nil, nil)
		dp.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyUpdate(gomock.Any(), gomock.Any()).Return(nil, nil)
		dp.Hal.MockClients.MockSgclient.EXPECT().SecurityGroupPolicyDelete(gomock.Any(), gomock.Any()).Return(nil, nil)

		dp.Hal.MockClients.MockEpclient.EXPECT().EndpointCreate(gomock.Any(), gomock.Any()).MaxTimes(3).Return(nil, nil)
		dp.Hal.MockClients.MockEpclient.EXPECT().EndpointUpdate(gomock.Any(), gomock.Any()).Return(nil, nil)
		dp.Hal.MockClients.MockEpclient.EXPECT().EndpointDelete(gomock.Any(), gomock.Any()).Return(nil, nil)

		dp.Hal.MockClients.MockTnclient.EXPECT().VrfCreate(gomock.Any(), gomock.Any()).MaxTimes(3).Return(nil, nil)
		dp.Hal.MockClients.MockTnclient.EXPECT().VrfUpdate(gomock.Any(), gomock.Any()).Return(nil, nil)
		dp.Hal.MockClients.MockTnclient.EXPECT().VrfDelete(gomock.Any(), gomock.Any()).Return(nil, nil)
	}

	err = populatePreTestData(nagent)

	if err != nil {
		log.Errorf("Could not meet prerequisites for testing Endpoint CRUD Methods")
		return nil, err
	}

	return NewRestServer(nagent, agentRestURL)

}

func populatePreTestData(nagent *state.NetAgent) (err error) {
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "preCreatedNetwork",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}
	err = nagent.CreateNetwork(&nt)
	if err != nil {
		return
	}

	ep := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "preCreatedEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "preCreatedNetwork",
		},
		Status: netproto.EndpointStatus{
			NodeUUID:    "dummy-node-uuid",
			IPv4Address: "10.1.1.0/24",
		},
	}
	_, err = nagent.CreateEndpoint(&ep)
	if err != nil {
		return
	}

	sg := netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "preCreatedSecurityGroup",
		},
		Spec: netproto.SecurityGroupSpec{
			SecurityProfile: "unknown",
			Rules: []netproto.SecurityRule{
				{
					Direction: "Incoming",
					PeerGroup: "",
					Action:    "Allow",
				},
			},
		},
	}
	err = nagent.CreateSecurityGroup(&sg)
	if err != nil {
		return
	}

	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "preCreatedTenant",
			Name:   "preCreatedTenant",
		},
	}

	err = nagent.CreateTenant(&tn)
	if err != nil {
		return
	}

	return nil
}

func TestRestServerStartStop(t *testing.T) {
	t.Parallel()
	// Don't need agent
	restSrv, err := NewRestServer(nil, ":0")
	if err != nil {
		t.Errorf("Could not start REST Server. Error: %v", err)
	}

	restURL := restSrv.GetListenURL()
	if len(restURL) == 0 {
		t.Errorf("Could not get the REST URL. URL: %s", restURL)
	}

	err = restSrv.Stop()
	if err != nil {
		t.Errorf("Failed to stop the REST Server. Error: %v", err)
	}
}

func TestRestServerListenFailures(t *testing.T) {
	t.Parallel()
	restSrv, err := NewRestServer(nil, "")
	if err != nil {
		t.Errorf("Could not start RestServer")
	}
	restSrv.Stop()

	_, err = NewRestServer(nil, ":65536")
	if err == nil {
		t.Errorf("Should see listener errors for the invalid port: %v", err)
	}
}
