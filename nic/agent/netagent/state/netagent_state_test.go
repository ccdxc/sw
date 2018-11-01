// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"testing"

	"fmt"
	"strings"

	hal "github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
)

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

func (ctrler *mockCtrler) EndpointDeleteReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	key := fmt.Sprintf("endpoint|%v|%v|%v", strings.ToLower(epinfo.Tenant), strings.ToLower(epinfo.Namespace), strings.ToLower(epinfo.Name))
	delete(ctrler.epdb, key)
	return epinfo, nil
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

	// create new network agent
	nagent, err := NewNetAgent(dp, state.AgentMode_MANAGED, "")

	if err != nil {
		t.Fatalf("Error creating network agent. Err: %v", err)
		return nil, nil, nil
	}

	// fake controller intf
	nagent.RegisterCtrlerIf(ct)
	return nagent, ct, dp
}
