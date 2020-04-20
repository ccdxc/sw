// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package controller

import (
	"testing"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// ##################################### Happy Path Test Cases  #####################################
func TestAppWatch(t *testing.T) {
	var l netutils.TestListenAddr
	err := l.GetAvailablePort()
	AssertOk(t, err, "failed to get an available port for REST Server")

	// Create App
	app := &netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "tcp",
					Port:     "1000",
				},
			},
		},
	}
	appdbcreate.LoadOrStore(app.GetKey(), app)

	//c.ResolverClient = &mock.ResolverClient{}
	o := types.DistributedServiceCardStatus{
		DSCName:     "mock-dsc",
		DSCMode:     "network_managed_inband",
		MgmtIP:      "42.42.42.42/24",
		Controllers: []string{"10.10.10.1"},
		MgmtIntf:    "lo",
	}
	err = c.HandleVeniceCoordinates(o)
	AssertOk(t, err, "Failed to trigger mode change")

	// Check if App is created
	AssertEventually(t, func() (bool, interface{}) {
		apps, err := pipelineAPI.HandleApp(types.Get, *app)
		return err == nil && proto.Equal(&apps[0].Spec, &app.Spec), nil
	}, "NetAgent did not get App Create.")

	// Update App
	updatedApp := &netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
		},
	}
	appdbupdate.LoadOrStore(updatedApp.GetKey(), updatedApp)
	// Check if App is updated
	AssertEventually(t, func() (bool, interface{}) {
		apps, err := pipelineAPI.HandleApp(types.Get, *app)
		return err == nil && proto.Equal(&apps[0].Spec, &updatedApp.Spec), nil
	}, "NetAgent did not get App Update.")

	// Delete App
	appdbdelete.LoadOrStore(updatedApp.GetKey(), updatedApp)
	// Check if App is deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, err := pipelineAPI.HandleApp(types.Get, *app)
		return err != nil, nil
	}, "NetAgent did not get App Delete.")
}

func TestNetworkSecurityPolicyWatch(t *testing.T) {
	var l netutils.TestListenAddr
	err := l.GetAvailablePort()
	AssertOk(t, err, "failed to get an available port for REST Server")

	// Create NetworkSecurityPolicy
	nsp := &netproto.NetworkSecurityPolicy{
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
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "192.168.1.1", "172.16.0.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Port:     "80",
								Protocol: "tcp",
							},
							{
								Port:     "443",
								Protocol: "tcp",
							},
							{
								Port:     "53",
								Protocol: "udp",
							},
							{
								Port:     "0-40",
								Protocol: "tcp",
							},
							{
								Port:     "60000-65535",
								Protocol: "tcp",
							},
							{
								Port:     "0-65535",
								Protocol: "tcp",
							},
						},
					},
				},
			},
		},
	}
	nspdbcreate.LoadOrStore(nsp.GetKey(), nsp)

	o := types.DistributedServiceCardStatus{
		DSCName:     "mock-dsc",
		DSCMode:     "network_managed_inband",
		MgmtIP:      "42.42.42.42/24",
		Controllers: []string{"10.10.10.1"},
		MgmtIntf:    "lo",
	}
	err = c.HandleVeniceCoordinates(o)
	AssertOk(t, err, "Failed to trigger mode change")

	// Check if NetworkSecurityPolicy is created. Cannot use proto.Equal here as NetAgent will add rule hash. Deferring to using rule length instead
	AssertEventually(t, func() (bool, interface{}) {
		nsps, err := pipelineAPI.HandleNetworkSecurityPolicy(types.Get, *nsp)
		return err == nil && len(nsps[0].Spec.Rules) == len(nsp.Spec.Rules), nil
	}, "NetAgent did not get NetworkSecurityPolicy Create.")

	// Update NetworkSecurityPolicy
	updatedNetworkSecurityPolicy := &netproto.NetworkSecurityPolicy{
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
					Action: "DENY",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "192.168.1.1", "172.16.0.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Port:     "80",
								Protocol: "tcp",
							},
							{
								Port:     "443",
								Protocol: "tcp",
							},
							{
								Port:     "53",
								Protocol: "udp",
							},
							{
								Port:     "0-40",
								Protocol: "tcp",
							},
							{
								Port:     "60000-65535",
								Protocol: "tcp",
							},
							{
								Port:     "0-65535",
								Protocol: "tcp",
							},
						},
					},
				},
			},
		},
	}
	nspdbupdate.LoadOrStore(updatedNetworkSecurityPolicy.GetKey(), updatedNetworkSecurityPolicy)
	// Check if NetworkSecurityPolicy is updated. Check if the updated field matches
	AssertEventually(t, func() (bool, interface{}) {
		nsps, err := pipelineAPI.HandleNetworkSecurityPolicy(types.Get, *nsp)
		return err == nil && nsps[0].Spec.Rules[0].Action == updatedNetworkSecurityPolicy.Spec.Rules[0].Action, nil
	}, "NetAgent did not get NetworkSecurityPolicy Update.")

	// Delete NetworkSecurityPolicy
	nspdbdelete.LoadOrStore(updatedNetworkSecurityPolicy.GetKey(), updatedNetworkSecurityPolicy)
	// Check if NetworkSecurityPolicy is deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, err := pipelineAPI.HandleNetworkSecurityPolicy(types.Get, *nsp)
		return err != nil, nil
	}, "NetAgent did not get NetworkSecurityPolicy Delete.")
}

func TestNetworkWatch(t *testing.T) {
	var l netutils.TestListenAddr
	err := l.GetAvailablePort()
	AssertOk(t, err, "failed to get an available port for REST Server")

	// Create Network
	network := &netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetwork",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 32,
		},
	}
	networkdbcreate.LoadOrStore(network.GetKey(), network)

	o := types.DistributedServiceCardStatus{
		DSCName:     "mock-dsc",
		DSCMode:     "network_managed_inband",
		MgmtIP:      "42.42.42.42/24",
		Controllers: []string{"10.10.10.1"},
		MgmtIntf:    "lo",
	}
	err = c.HandleVeniceCoordinates(o)
	AssertOk(t, err, "Failed to trigger mode change")

	// Check if Network is created
	AssertEventually(t, func() (bool, interface{}) {
		networks, err := pipelineAPI.HandleNetwork(types.Get, *network)
		return err == nil && proto.Equal(&networks[0].Spec, &network.Spec), nil
	}, "NetAgent did not get Network Create.")

	// Update Network
	updatedNetwork := &netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetwork",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
	}
	networkdbupdate.LoadOrStore(updatedNetwork.GetKey(), updatedNetwork)
	// Check if Network is updated
	AssertEventually(t, func() (bool, interface{}) {
		networks, err := pipelineAPI.HandleNetwork(types.Get, *network)
		return err == nil && proto.Equal(&networks[0].Spec, &updatedNetwork.Spec), nil
	}, "NetAgent did not get Network Update.")

	// Delete Network
	networkdbdelete.LoadOrStore(updatedNetwork.GetKey(), updatedNetwork)
	// Check if Network is deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, err := pipelineAPI.HandleNetwork(types.Get, *network)
		return err != nil, nil
	}, "NetAgent did not get Network Delete.")
}

func TestEndpointWatch(t *testing.T) {
	t.Skip("Currently unstable.")
	var l netutils.TestListenAddr
	err := l.GetAvailablePort()
	AssertOk(t, err, "failed to get an available port for REST Server")

	// Create Endpoint
	endpoint := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   "skywalker",
			IPv4Addresses: []string{"10.1.1.1", "10.1.1.2"},
			MacAddress:    "baba.baba.baba",
			NodeUUID:      "luke",
			UsegVlan:      42,
		},
		Status: netproto.EndpointStatus{
			EnicID: 42, // Simulate a local EP
		},
	}

	network := &netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "skywalker",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 32,
		},
	}
	networkdbcreate.LoadOrStore(network.GetKey(), network)

	// Check if Network is created
	AssertEventually(t, func() (bool, interface{}) {
		networks, err := pipelineAPI.HandleNetwork(types.Get, *network)
		if err != nil {
			// Simulate re-transmits
			networkdbcreate.LoadOrStore(network.GetKey(), network)
		}
		return err == nil && proto.Equal(&networks[0].Spec, &network.Spec), nil
	}, "NetAgent did not get Network Create.", "10s", "2m")

	endpointdbcreate.LoadOrStore(endpoint.GetKey(), endpoint)

	o := types.DistributedServiceCardStatus{
		DSCName:     "mock-dsc",
		DSCMode:     "network_managed_inband",
		MgmtIP:      "42.42.42.42/24",
		Controllers: []string{"10.10.10.1"},
		MgmtIntf:    "lo",
	}
	err = c.HandleVeniceCoordinates(o)
	AssertOk(t, err, "Failed to trigger mode change")

	// Check if Endpoint is created
	AssertEventually(t, func() (bool, interface{}) {
		endpoints, err := pipelineAPI.HandleEndpoint(types.Get, *endpoint)
		return err == nil && proto.Equal(&endpoints[0].Spec, &endpoint.Spec), nil
	}, "NetAgent did not get Endpoint Create.", "30s", "1m")

	// Update Endpoint
	updatedEndpoint := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   "skywalker",
			IPv4Addresses: []string{"42.42.42.42"},
			MacAddress:    "baba.baba.baba",
			NodeUUID:      "luke",
			UsegVlan:      42,
		},
		Status: netproto.EndpointStatus{
			EnicID: 42, // Simulate a local EP
		},
	}
	endpointdbupdate.LoadOrStore(updatedEndpoint.GetKey(), updatedEndpoint)
	// Check if Endpoint is updated
	AssertEventually(t, func() (bool, interface{}) {
		endpoints, err := pipelineAPI.HandleEndpoint(types.Get, *endpoint)
		return err == nil && proto.Equal(&endpoints[0].Spec, &updatedEndpoint.Spec), nil
	}, "NetAgent did not get Endpoint Update.", "30s", "1m")

	// Delete Endpoint
	endpointdbdelete.LoadOrStore(updatedEndpoint.GetKey(), updatedEndpoint)
	// Check if Endpoint is deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, err := pipelineAPI.HandleEndpoint(types.Get, *endpoint)
		return err != nil, nil
	}, "NetAgent did not get Endpoint Delete.")
}
