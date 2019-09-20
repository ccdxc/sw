// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"fmt"
	"io/ioutil"
	"os"
	"strings"
	"testing"

	"github.com/pensando/sw/api"
	hal "github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	apps = []*netproto.App{
		{TypeMeta: api.TypeMeta{Kind: "App"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "dns",
			},
			Spec: netproto.AppSpec{
				ProtoPorts: []string{"udp/53"},
				ALG: &netproto.ALG{
					DNS: &netproto.DNS{
						DropLargeDomainPackets: true,
						DropMultiZonePackets:   true,
						QueryResponseTimeout:   "30s",
					},
				},
			},
		},
		{TypeMeta: api.TypeMeta{Kind: "App"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "ftp",
			},
			Spec: netproto.AppSpec{
				ProtoPorts: []string{"tcp/21"},
				ALG: &netproto.ALG{
					FTP: &netproto.FTP{
						AllowMismatchIPAddresses: true,
					},
				},
			},
		},
	}

	sgPolicies = []*netproto.NetworkSecurityPolicy{
		{
			TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "testNetworkSecurityPolicy",
			},
			Spec: netproto.NetworkSecurityPolicySpec{
				AttachTenant: true,
				Rules: []netproto.PolicyRule{
					{
						Action: "PERMIT",
						Src: &netproto.MatchSelector{
							Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						},
						Dst: &netproto.MatchSelector{
							Addresses: []string{"192.168.0.1 - 192.168.1.0"},
							AppConfigs: []*netproto.AppConfig{
								{
									Port:     "80",
									Protocol: "tcp",
								},
							},
						},
					},
					{
						Action:  "PERMIT",
						AppName: "ftp",
						Src: &netproto.MatchSelector{
							Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						},
						Dst: &netproto.MatchSelector{
							Addresses: []string{"192.168.0.1 - 192.168.1.0"},
						},
					},
				},
			},
		},
	}

	nets = []*netproto.Network{
		{
			TypeMeta: api.TypeMeta{Kind: "Network"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Name:      "default",
				Namespace: "default",
			},
			Spec: netproto.NetworkSpec{
				IPv4Subnet:  "10.1.1.0/24",
				IPv4Gateway: "10.1.1.254",
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Network"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Name:      "_internal-network",
				Namespace: "default",
			},
			Spec: netproto.NetworkSpec{
				IPv4Subnet:  "10.1.1.0/24",
				IPv4Gateway: "10.1.1.254",
			},
		},
	}

	eps = []*netproto.Endpoint{
		{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Name:      "testEndpoint",
				Namespace: "default",
			},
			Spec: netproto.EndpointSpec{
				EndpointUUID: "testEndpointUUID",
				WorkloadUUID: "testWorkloadUUID",
				NetworkName:  "default",
				MacAddress:   "4242.4242.4242",
			},
		},
		{
			// Dummy internal objects to simulate lateral objects
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Name:      "_internal-testEndpoint",
				Namespace: "default",
			},
			Spec: netproto.EndpointSpec{
				EndpointUUID: "testEndpointUUID",
				WorkloadUUID: "testWorkloadUUID",
				NetworkName:  "_internal-network",
				MacAddress:   "4242.4242.4242",
			},
		},
	}
)

func TestNetAgentCfgPreCreatedObjects(t *testing.T) {
	db, err := ioutil.TempFile("", "netagent.db")
	defer os.Remove(db.Name())
	AssertOk(t, err, "Failed to create temp file")
	dp, err := hal.NewHalDatapath("mock")
	AssertOk(t, err, "Failed to instantiate mock datapath")

	ag, err := NewNetAgent(dp, db.Name(), nil)

	AssertOk(t, err, "Failed to create new netagent: ")
	oMeta := api.ObjectMeta{
		Name:      "default",
		Namespace: "default",
		Tenant:    "default",
	}

	_, err = ag.FindTenant(oMeta)
	AssertOk(t, err, "Failed to find default tenant")

	defVrf, err := ag.FindVrf(oMeta)
	AssertOk(t, err, "Failed to find default VRF")
	AssertEquals(t, uint64(types.VrfOffset), defVrf.Status.VrfID, "VRF ID did not match")
	err = ag.Stop()
	AssertOk(t, err, "Failed to stop agent")

	// Simulate restart by pointing a new Agent to the same DB
	newAg, err := NewNetAgent(dp, db.Name(), nil)
	defer newAg.Stop()
	AssertOk(t, err, "Failed to create new netagent: ")
	_, err = newAg.FindTenant(oMeta)
	AssertOk(t, err, "Failed to find default tenant")

	defVrf1, err := newAg.FindVrf(oMeta)
	AssertOk(t, err, "Failed to find default VRF")
	AssertEquals(t, uint64(types.VrfOffset), defVrf1.Status.VrfID, "VRF ID did not match")
}

func TestNetAgentConfigPersistence(t *testing.T) {
	db, err := ioutil.TempFile("", "netagent.db")
	defer os.Remove(db.Name())
	AssertOk(t, err, "Failed to create temp file")
	dp, err := hal.NewHalDatapath("mock")
	AssertOk(t, err, "Failed to instantiate mock datapath")

	ag, err := NewNetAgent(dp, db.Name(), nil)

	AssertOk(t, err, "Failed to create new netagent: ")

	network := &netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "skynet",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
	}

	endpoint := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "testEndpoint",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "skynet",
			MacAddress:   "4242.4242.4242",
		},
	}

	dns := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []string{"udp/53"},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
				},
			},
		},
	}

	ftp := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "ftp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []string{"tcp/21"},
			ALG: &netproto.ALG{
				FTP: &netproto.FTP{
					AllowMismatchIPAddresses: true,
				},
			},
		},
	}

	sgPolicy := &netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					AppName: "dns",
				},
			},
		},
	}

	tun := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testTunnel",
		},
		Spec: netproto.TunnelSpec{
			Type:        "VXLAN",
			AdminStatus: "UP",
			Src:         "10.1.1.1",
			Dst:         "192.168.1.1",
		},
	}

	err = ag.CreateNetwork(network)
	AssertOk(t, err, "Failed to create network")

	err = ag.CreateEndpoint(endpoint)
	AssertOk(t, err, "Failed to create endpoint")

	err = ag.CreateApp(&dns)
	AssertOk(t, err, "Failed to create app")

	err = ag.CreateApp(&ftp)
	AssertOk(t, err, "Failed to create app")

	err = ag.CreateNetworkSecurityPolicy(sgPolicy)
	AssertOk(t, err, "Failed to create sg policy")

	err = ag.CreateTunnel(&tun)
	AssertOk(t, err, "Error creating tunnel")

	// Ensure that they are present in the state
	_, err = ag.FindNetwork(network.ObjectMeta)
	AssertOk(t, err, "Failed to find network")

	_, err = ag.FindEndpoint(endpoint.ObjectMeta)
	AssertOk(t, err, "Failed to find endpoint")

	_, err = ag.FindApp(dns.ObjectMeta)
	AssertOk(t, err, "Failed to find dns app")

	_, err = ag.FindApp(ftp.ObjectMeta)
	AssertOk(t, err, "Failed to find ftp app")

	_, err = ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "Failed to find sg policy")

	tunnel, err := ag.FindTunnel(tun.ObjectMeta)
	AssertOk(t, err, "Tunnel was not found in DB")
	Assert(t, tunnel.Name == "testTunnel", "Tunnel names did not match", tunnel)

	// Stop the agent
	ag.Stop()

	newAgent, err := NewNetAgent(dp, db.Name(), nil)
	AssertOk(t, err, "Failed to create new netagent: ")

	// Make an explicit call to replay configs. In real HW this will be triggered by the mode switch
	err = newAgent.ReplayConfigs()
	AssertOk(t, err, "Failed to replay configs")

	// Ensure that they are present in the state
	_, err = newAgent.FindNetwork(network.ObjectMeta)
	AssertOk(t, err, "Failed to find network")

	_, err = newAgent.FindEndpoint(endpoint.ObjectMeta)
	AssertOk(t, err, "Failed to find endpoint")

	_, err = newAgent.FindApp(dns.ObjectMeta)
	AssertOk(t, err, "Failed to find app")

	_, err = newAgent.FindApp(ftp.ObjectMeta)
	AssertOk(t, err, "Failed to find app")

	_, err = newAgent.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "Failed to find sg policy")

	tunnel, err = ag.FindTunnel(tun.ObjectMeta)
	AssertOk(t, err, "Tunnel was not found in DB")
	Assert(t, tunnel.Name == "testTunnel", "Tunnel names did not match", tunnel)

}

func TestNaplesPurgeConfigs(t *testing.T) {
	db, err := ioutil.TempFile("", "netagent.db")
	defer os.Remove(db.Name())
	AssertOk(t, err, "Failed to create temp file")
	dp, err := hal.NewHalDatapath("mock")
	AssertOk(t, err, "Failed to instantiate mock datapath")

	ag, err := NewNetAgent(dp, db.Name(), nil)

	AssertOk(t, err, "Failed to create new netagent: ")

	// Create Networks
	for _, n := range nets {
		err := ag.CreateNetwork(n)
		AssertOk(t, err, "Failed to create network")
	}

	// Create Endpoints
	for _, e := range eps {
		err := ag.CreateEndpoint(e)
		AssertOk(t, err, "Failed to create endpoint")
	}

	// Create Apps
	for _, a := range apps {
		err := ag.CreateApp(a)
		AssertOk(t, err, "Failed to create app")
	}

	// Create SG Policy
	for _, s := range sgPolicies {
		err := ag.CreateNetworkSecurityPolicy(s)
		AssertOk(t, err, "Failed to create SG Policy")
	}

	err = ag.PurgeConfigs()
	AssertOk(t, err, "Failed to purge configs.")

	curApps := ag.ListApp()
	AssertEquals(t, 0, len(curApps), "Purge config must delete all netagent apps")

	curSGPolicies := ag.ListNetworkSecurityPolicy()
	AssertEquals(t, 0, len(curSGPolicies), "Purge config must delete all netagent sg policies")

	curEPs := ag.ListEndpoint()
	for _, e := range curEPs {
		// Ignore internal EPs
		if !strings.Contains(e.Name, "_internal") {
			t.Errorf("Purge failed to delete netagent Endpoints. Found: %v", e)
		}
	}

	curNWs := ag.ListNetwork()
	for _, n := range curNWs {
		// Ignore internal EPs
		if !strings.Contains(n.Name, "_internal") {
			t.Errorf("Purge failed to delete netagent Networks. Found: %v", n)
		}
	}

}

type mockCtrler struct {
	epdb map[string]*netproto.Endpoint
}

func (ctrler *mockCtrler) EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	key := fmt.Sprintf("endpoint|%v|%v|%v", strings.ToLower(epinfo.Tenant), strings.ToLower(epinfo.Namespace), strings.ToLower(epinfo.Name))
	ctrler.epdb[key] = epinfo
	return epinfo, nil
}

func (ctrler *mockCtrler) EndpointAgeoutNotif(epinfo *netproto.Endpoint) error {
	return nil
}

func (ctrler *mockCtrler) IsConnected() bool {
	return false
}

func (ctrler *mockCtrler) EndpointDeleteReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	key := fmt.Sprintf("endpoint|%v|%v|%v", strings.ToLower(epinfo.Tenant), strings.ToLower(epinfo.Namespace), strings.ToLower(epinfo.Name))
	delete(ctrler.epdb, key)
	return epinfo, nil
}

// mock delphi service
type mockService struct {
}

func (svc *mockService) OnMountComplete() {
	log.Infof("Mock service OnMountComplete() got called")
}

func (svc *mockService) Name() string {
	return "netagent-test"
}

// createNetAgent creates a netagent scaffolding
func createNetAgent(t *testing.T) (*Nagent, *mockCtrler, *hal.Datapath) {
	dp, err := hal.NewHalDatapath("mock")
	if err != nil {
		t.Fatalf("could not create a mock datapath")
	}
	ct := &mockCtrler{
		epdb: make(map[string]*netproto.Endpoint),
	}

	cl, err := gosdk.NewClient(&mockService{})
	if err != nil {
		t.Fatalf("Error creating delphi client. Err: %v", err)
	}

	// create new network agent
	nagent, err := NewNetAgent(dp, "", cl)

	if err != nil {
		t.Fatalf("Error creating network agent. Err: %v", err)
		return nil, nil, nil
	}

	// fake controller intf
	nagent.RegisterCtrlerIf(ct)
	return nagent, ct, dp
}
