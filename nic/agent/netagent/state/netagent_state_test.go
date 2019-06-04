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

func TestNetAgentCfgPersistence(t *testing.T) {
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
	AssertEquals(t, uint64(types.VrfOffset+1), defVrf.Status.VrfID, "VRF ID did not match")
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
	AssertEquals(t, uint64(types.VrfOffset+1), defVrf1.Status.VrfID, "VRF ID did not match")
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
