package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestRoutingConfigCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	name := "testCfg"
	updateIP := "101.1.1.1"
	rtcfg := netproto.RoutingConfig{
		TypeMeta: api.TypeMeta{Kind: "RoutingConfig"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      name,
		},
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId: "1.1.1.1",
				ASNumber: 100,
			},
			EVPNConfig: &netproto.EVPNConfig{
				Shutdown: false,
			},
		},
		Status: netproto.RoutingConfigStatus{},
	}

	cfg := &netproto.BGPNeighbor{
		Shutdown:              false,
		IPAddress:             "100.1.1.1",
		RemoteAS:              1,
		MultiHop:              1,
		EnableAddressFamilies: []string{"IPV4", "EVPN"},
	}

	rtcfg.Spec.BGPConfig.Neighbors = append(rtcfg.Spec.BGPConfig.Neighbors, cfg)

	preExistingCount := len(ag.ListRoutingConfig())

	// create RoutingConfig
	err := ag.CreateRoutingConfig(&rtcfg)
	AssertOk(t, err, "Error creating RoutingConfig")
	p, err := ag.FindRoutingConfig(rtcfg.ObjectMeta)
	AssertOk(t, err, "RoutingConfig was not found in DB")
	Assert(t, p.Name == name, "RoutingConfig name did not match", rtcfg)

	// verify list api works.
	cfgList := ag.ListRoutingConfig()
	Assert(t, len(cfgList) == preExistingCount+1, "Incorrect number of Routing configs")

	// update the RoutingConfig
	rtcfg.Spec.BGPConfig.Neighbors[0].IPAddress = updateIP
	err = ag.UpdateRoutingConfig(&rtcfg)
	AssertOk(t, err, "Error updating RoutingConfig")
	p, err = ag.FindRoutingConfig(rtcfg.ObjectMeta)
	AssertOk(t, err, "RoutingConfig was not found in DB")
	Assert(t, (p.Spec.BGPConfig.Neighbors[0].IPAddress == updateIP), "Neighbor IPAddr did not match", rtcfg)

	// delete the routingconfig and verify its gone from db
	err = ag.DeleteRoutingConfig(rtcfg.Tenant, rtcfg.Namespace, rtcfg.Name)
	AssertOk(t, err, "Error deleting RoutingConfig")
	_, err = ag.FindRoutingConfig(rtcfg.ObjectMeta)
	Assert(t, (err != nil), "RoutingConfig was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteRoutingConfig(rtcfg.Tenant, rtcfg.Namespace, rtcfg.Name)
	Assert(t, err != nil, "deleting non-existing RoutingConfig succeeded", ag)
}
