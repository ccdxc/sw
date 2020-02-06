// +build ignore

package restapi

import (
	"flag"
	"os"
	"testing"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	tpdatapath "github.com/pensando/sw/nic/agent/tpa/datapath"
	tpstate "github.com/pensando/sw/nic/agent/tpa/state"
	tsdatapath "github.com/pensando/sw/nic/agent/troubleshooting/datapath/hal"
	tsstate "github.com/pensando/sw/nic/agent/troubleshooting/state"
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

func getMgmtIP() string {
	return "10.10.10.10"
}

var datapathKind = flag.String("datapath", agentDatapathKind, "Specify the datapath type. mock | hal")

func setup() (*RestServer, error) {

	dp, err := datapath.NewHalDatapath(datapath.Kind(*datapathKind))
	if err != nil {
		log.Errorf("Could not create HAL datapath. Kind: %v, Error %v", datapathKind, err)
		return nil, err
	}

	// Set tenant creation expectation
	if dp.Kind.String() == "mock" {
		dp.Hal.MockClients.MockTnclient.EXPECT().VrfCreate(gomock.Any(), gomock.Any()).Return(nil, nil)
	}

	nagent, err := state.NewNetAgent(dp, "", nil)
	if err != nil {
		log.Errorf("Could not create net agent")
		return nil, err
	}

	err = populatePreTestData(nagent)

	if err != nil {
		log.Errorf("Could not meet prerequisites for testing Endpoint CRUD Methods")
		return nil, err
	}

	tsdp, err := tsdatapath.NewHalDatapath(tsdatapath.Kind(*datapathKind))
	if err != nil {
		log.Errorf("Could not create troubleshooting HAL datapath. Kind: %v, Error %v", datapathKind, err)
		return nil, err
	}
	tsagent, err := tsstate.NewTsAgent(tsdp, "dummy-node-uuid", nagent, getMgmtIP)
	if err != nil {
		log.Errorf("Could not create ts troubleshooting agent")
		return nil, err
	}

	tpa, err := tpstate.NewTpAgent(nagent, func() string { return "192.168.100.101" }, tpdatapath.MockHal())
	if err != nil {
		log.Fatalf("Error creating telemetry policy agent. Err: %v", err)
	}
	log.Printf("telemetry policy agent {%+v} instantiated", tpa)

	return NewRestServer(nagent, tsagent, tpa, agentRestURL)

}

func populatePreTestData(nagent *state.Nagent) (err error) {
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedVrf",
		},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
	}
	err = nagent.CreateVrf(&vrf)
	if err != nil {
		log.Errorf("Failed to create vrf. {%v}", vrf)
		return
	}

	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedNetwork",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
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
			NetworkName:   "preCreatedNetwork",
			NodeUUID:      "dummy-node-uuid",
			IPv4Addresses: []string{"10.1.1.0/24"},
			MacAddress:    "4242.4242.4242",
		},
	}
	err = nagent.CreateEndpoint(&ep)
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

	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachGroup:  []string{"preCreatedSecurityGroup"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Port:     "80",
								Protocol: "tcp",
							},
						},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
				},
			},
		},
	}

	err = nagent.CreateNetworkSecurityPolicy(&sgPolicy)
	if err != nil {
		log.Errorf("Failed to create SG policy. {%v}", sgPolicy)
		return
	}

	tunnel := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedTunnel",
		},
		Spec: netproto.TunnelSpec{
			Type:        "VXLAN",
			AdminStatus: "UP",
			Src:         "10.1.1.1",
			Dst:         "192.168.1.1",
		},
	}

	err = nagent.CreateTunnel(&tunnel)
	if err != nil {
		log.Errorf("Failed to create tunnel. {%v}", tunnel)
		return
	}

	enic := netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedInterface",
		},
		Spec: netproto.InterfaceSpec{
			Type:        "ENIC",
			AdminStatus: "UP",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "UP",
		},
	}

	err = nagent.CreateInterface(&enic)
	if err != nil {
		log.Errorf("Failed to create ENIC. {%v}", enic)
		return
	}

	port := netproto.Port{
		TypeMeta: api.TypeMeta{Kind: "Port"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedPort",
		},
		Spec: netproto.PortSpec{
			Speed:        "SPEED_100G",
			Type:         "TYPE_ETHERNET",
			AdminStatus:  "UP",
			BreakoutMode: "BREAKOUT_NONE",
			Lanes:        4,
		},
	}

	err = nagent.CreatePort(&port)
	if err != nil {
		log.Errorf("Failed to create port. {%v} | Err: %v", enic, err)
		return
	}

	secProfile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "10s",
				TCP:                "1m",
				TCPDrop:            "5s",
				TCPConnectionSetup: "300ms",
				TCPClose:           "1h",
				Drop:               "30s",
				UDP:                "5s",
				UDPDrop:            "1s",
				ICMP:               "100ms",
				ICMPDrop:           "1h10m15s",
			},
			RateLimits: &netproto.RateLimits{
				TcpHalfOpenSessionLimit: 0,
				UdpActiveSessionLimit:   0,
				IcmpActiveSessionLimit:  0,
				OtherActiveSessionLimit: 0,
			},
		},
	}

	err = nagent.CreateSecurityProfile(&secProfile)
	if err != nil {
		log.Errorf("Failed to create security profile. {%v}", secProfile)
		return
	}

	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					QueryResponseTimeout:   "30s",
				},
			},
		},
	}

	err = nagent.CreateApp(&app)
	if err != nil {
		log.Errorf("Failed to create app. %v", app)
	}

	return nil
}

func TestRestServerStartStop(t *testing.T) {
	t.Parallel()
	// Don't need agent
	restSrv, err := NewRestServer(nil, nil, nil, ":0")
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
	restSrv, err := NewRestServer(nil, nil, nil, "")
	if err != nil {
		t.Errorf("Could not start RestServer")
	}
	restSrv.Stop()

	_, err = NewRestServer(nil, nil, nil, ":65536")
	if err == nil {
		t.Errorf("Should see listener errors for the invalid port: %v", err)
	}
}
