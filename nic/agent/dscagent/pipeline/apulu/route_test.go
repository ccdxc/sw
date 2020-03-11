// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"fmt"
	"testing"

	"github.com/deckarep/golang-set"

	types "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/emstore"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type mockInfraAPI struct {
	cfg types.DistributedServiceCardStatus
}

// List returns a list of objects by kind
func (m *mockInfraAPI) List(kind string) ([][]byte, error) {
	return nil, nil
}

// Read returns a single object of a given kind
func (m *mockInfraAPI) Read(kind, key string) ([]byte, error) {
	return nil, nil
}

// Store stores a single object of a given kind
func (m *mockInfraAPI) Store(kind, key string, data []byte) error {
	return nil
}

// Delete deletes a single object
func (m *mockInfraAPI) Delete(kind, key string) error {
	return nil
}

// AllocateID allocates id based off an offset
func (m *mockInfraAPI) AllocateID(kind emstore.ResourceType, offset int) uint64 {
	return 0
}

// GetDscName returns the DSC Name.
func (m *mockInfraAPI) GetDscName() string {
	return ""
}

// StoreConfig stores NetAgent config
func (m *mockInfraAPI) StoreConfig(cfg types.DistributedServiceCardStatus) {}

// GetConfig returns NetAgent config
func (m *mockInfraAPI) GetConfig() (cfg types.DistributedServiceCardStatus) {
	return m.cfg
}

// NotifyVeniceConnection updates venice connection status from netagent
func (m *mockInfraAPI) NotifyVeniceConnection() {}

// UpdateIfChannel returns a channel for propogating interface state to the netagent
func (m *mockInfraAPI) UpdateIfChannel() chan types.UpdateIfEvent {
	return nil
}

// Close closes the Infra API
func (m *mockInfraAPI) Close() error {
	return nil
}

func TestExpandRoutingConfig(t *testing.T) {
	// Base with unspecified
	in := netproto.RoutingConfig{
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId: "0.0.0.0",
				ASNumber: 1000,
				Neighbors: []*netproto.BGPNeighbor{
					{
						Shutdown:              false,
						IPAddress:             "0.0.0.0",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"ipv4-unicast"},
					},
				},
			},
		},
	}

	mockApi := &mockInfraAPI{
		cfg: types.DistributedServiceCardStatus{
			LoopbackIP:  "",
			Controllers: []string{"10.1.1.1:9009", "10.1.1.2:9009", "11.1.1.1:9009"},
			DSCInterfaceIPs: []types.DSCInterfaceIP{
				{
					IPAddress: "12.1.1.1",
					GatewayIP: "12.1.1.2",
				},
				{
					IPAddress: "12.1.2.1",
					GatewayIP: "12.1.2.2",
				},
			},
		},
	}
	// Base with unspecified + neighbors
	out := expandRoutingConfig(mockApi, "0.0.0.0", &in)
	Assert(t, len(out.Spec.BGPConfig.Neighbors) == 2, "did not match number of neighbors", len(out.Spec.BGPConfig.Neighbors))

	expSet := mapset.NewSetFromSlice([]interface{}{"12.1.1.2:ipv4-unicast", "12.1.2.2:ipv4-unicast"})
	gotSet := mapset.NewSet()
	for _, n := range out.Spec.BGPConfig.Neighbors {
		for _, af := range n.EnableAddressFamilies {
			gotSet.Add(fmt.Sprintf("%v:%v", n.IPAddress, af))
		}
	}
	Assert(t, expSet.Equal(gotSet), "unexected set of neighbort [%v]", gotSet.String())

	in = netproto.RoutingConfig{
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId: "0.0.0.0",
				ASNumber: 1000,
				Neighbors: []*netproto.BGPNeighbor{
					{
						Shutdown:              false,
						IPAddress:             "0.0.0.0",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"ipv4-unicast"},
					},
					{
						Shutdown:              false,
						IPAddress:             "0.0.0.0",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"l2vpn-evpn"},
					},
				},
			},
		},
	}

	// Base with unspecified + neighbors
	out = expandRoutingConfig(mockApi, "0.0.0.0", &in)
	Assert(t, len(out.Spec.BGPConfig.Neighbors) == 5, "did not match number of neighbors", len(out.Spec.BGPConfig.Neighbors))

	expSet = mapset.NewSetFromSlice([]interface{}{"12.1.1.2:ipv4-unicast", "12.1.2.2:ipv4-unicast", "10.1.1.1:l2vpn-evpn", "10.1.1.2:l2vpn-evpn", "11.1.1.1:l2vpn-evpn"})
	gotSet = mapset.NewSet()
	for _, n := range out.Spec.BGPConfig.Neighbors {
		for _, af := range n.EnableAddressFamilies {
			gotSet.Add(fmt.Sprintf("%v:%v", n.IPAddress, af))
		}
	}
	Assert(t, expSet.Equal(gotSet), "unexpected set of neighbort [%v]", gotSet.String())

	// Base no unspecified
	in = netproto.RoutingConfig{
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId: "0.0.0.0",
				ASNumber: 1000,
				Neighbors: []*netproto.BGPNeighbor{
					{
						Shutdown:              false,
						IPAddress:             "10.1.1.8",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"ipv4-unicast"},
					},
				},
			},
		},
	}
	out = expandRoutingConfig(mockApi, "0.0.0.0", &in)
	Assert(t, len(out.Spec.BGPConfig.Neighbors) == 1, "did not match number of neighbors", len(out.Spec.BGPConfig.Neighbors))

	expSet = mapset.NewSetFromSlice([]interface{}{"10.1.1.8:ipv4-unicast"})
	gotSet = mapset.NewSet()
	for _, n := range out.Spec.BGPConfig.Neighbors {
		for _, af := range n.EnableAddressFamilies {
			gotSet.Add(fmt.Sprintf("%v:%v", n.IPAddress, af))
		}
	}
	Assert(t, expSet.Equal(gotSet), "unexected set of neighbort [%v]", gotSet.String())

	// unspecified with specific neighbors
	in = netproto.RoutingConfig{
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId: "0.0.0.0",
				ASNumber: 1000,
				Neighbors: []*netproto.BGPNeighbor{
					{
						Shutdown:              false,
						IPAddress:             "0.0.0.0",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"ipv4-unicast"},
					},
					{
						Shutdown:              false,
						IPAddress:             "0.0.0.0",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"l2vpn-evpn"},
					},
					{
						Shutdown:              false,
						IPAddress:             "10.1.1.8",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"l2vpn-evpn"},
					},
				},
			},
		},
	}
	out = expandRoutingConfig(mockApi, "0.0.0.0", &in)
	Assert(t, len(out.Spec.BGPConfig.Neighbors) == 6, "did not match number of neighbors", len(out.Spec.BGPConfig.Neighbors))

	expSet = mapset.NewSetFromSlice([]interface{}{"12.1.1.2:ipv4-unicast", "12.1.2.2:ipv4-unicast", "10.1.1.1:l2vpn-evpn", "10.1.1.2:l2vpn-evpn", "11.1.1.1:l2vpn-evpn", "10.1.1.8:l2vpn-evpn"})
	gotSet = mapset.NewSet()
	for _, n := range out.Spec.BGPConfig.Neighbors {
		for _, af := range n.EnableAddressFamilies {
			gotSet.Add(fmt.Sprintf("%v:%v", n.IPAddress, af))
		}
	}
	Assert(t, expSet.Equal(gotSet), "unexected set of neighbort [%v]", gotSet.String())

	// With no Venice Controllers
	mockApi.cfg.Controllers = nil
	in = netproto.RoutingConfig{
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId: "0.0.0.0",
				ASNumber: 1000,
				Neighbors: []*netproto.BGPNeighbor{
					{
						Shutdown:              false,
						IPAddress:             "0.0.0.0",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"ipv4-unicast"},
					},
					{
						Shutdown:              false,
						IPAddress:             "0.0.0.0",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"l2vpn-evpn"},
					},
					{
						Shutdown:              false,
						IPAddress:             "10.1.1.8",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"l2vpn-evpn"},
					},
				},
			},
		},
	}

	out = expandRoutingConfig(mockApi, "0.0.0.0", &in)
	Assert(t, len(out.Spec.BGPConfig.Neighbors) == 3, "did not match number of neighbors", len(out.Spec.BGPConfig.Neighbors))

	expSet = mapset.NewSetFromSlice([]interface{}{"12.1.1.2:ipv4-unicast", "12.1.2.2:ipv4-unicast", "10.1.1.8:l2vpn-evpn"})
	gotSet = mapset.NewSet()
	for _, n := range out.Spec.BGPConfig.Neighbors {
		for _, af := range n.EnableAddressFamilies {
			gotSet.Add(fmt.Sprintf("%v:%v", n.IPAddress, af))
		}
	}
	Assert(t, expSet.Equal(gotSet), "unexected set of neighbort [%v]", gotSet.String())
}
