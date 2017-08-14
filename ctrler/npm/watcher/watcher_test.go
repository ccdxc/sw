// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package watcher

import (
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/ctrler/npm/statemgr"
	. "github.com/pensando/sw/utils/testutils"
)

// dummy writer
type dummyWriter struct {
	// no fields
}

func (d *dummyWriter) WriteNetwork(nw *network.Network) error {
	return nil
}

func (d *dummyWriter) WriteEndpoint(ep *network.Endpoint, update bool) error {
	return nil
}

func (d *dummyWriter) WriteSecurityGroup(sg *network.SecurityGroup) error {
	return nil
}

func (d *dummyWriter) WriteSgPolicy(sgp *network.Sgpolicy) error {
	return nil
}

func TestNetworkWatcher(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, "", "")
	if err != nil {
		t.Fatalf("Error creating api server watcher. Err: %v", err)
		return
	}

	// create a network
	err = watcher.CreateNetwork("default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating network")

	// verify network got created
	AssertEventually(t, func() bool {
		_, nerr := stateMgr.FindNetwork("default", "default")
		return (nerr == nil)
	}, "Network not found in statemgr")
	nw, err := stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")
	Assert(t, (nw.Spec.IPv4Subnet == "10.1.1.0/24"), "Network subnet did not match", nw)
	Assert(t, (nw.Spec.IPv4Gateway == "10.1.1.254"), "Network gateway did not match", nw)
	Assert(t, (nw.TypeMeta.Kind == "Network"), "Network type meta did not match", nw)

	// trigger a delete event
	err = watcher.DeleteNetwork("default", "default")
	AssertOk(t, err, "Error deleting network")

	// verify network is removed
	AssertEventually(t, func() bool {
		_, nerr := stateMgr.FindNetwork("default", "default")
		return (nerr != nil)
	}, "Network still found after deleting")

	// close the channels
	watcher.Stop()
}

func TestVmmEndpointWatcher(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, "", "")
	if err != nil {
		t.Fatalf("Error creating api server watcher. Err: %v", err)
		return
	}

	// create a network
	err = watcher.CreateNetwork("default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating network")
	AssertEventually(t, func() bool {
		_, nerr := stateMgr.FindNetwork("default", "default")
		return (nerr == nil)
	}, "Network not found in statemgr")

	// create a endpoint
	var attrs []string
	err = watcher.CreateEndpoint("default", "default", "testEndpoint", "testVm", "01:02:03:04:05:06", "testHost", "20.1.1.1", attrs, 101)
	AssertOk(t, err, "Error creating endpoint")

	// verify endpoint got created
	AssertEventually(t, func() bool {
		_, perr := stateMgr.FindObject("Endpoint", "default", "testEndpoint")
		return (perr == nil)
	}, "Endpoint not found in statemgr")
	ep, err := stateMgr.FindObject("Endpoint", "default", "testEndpoint")
	AssertOk(t, err, "endpoint not found")
	Assert(t, (ep.GetObjectKind() == "Endpoint"), "Incorrect object kind", ep)
	Assert(t, (ep.GetObjectMeta().Name == "testEndpoint"), "Incorrect object params", ep.GetObjectMeta())

	// test duplicate create
	watcher.CreateEndpoint("default", "default", "testEndpoint", "testVm", "01:02:03:04:05:06", "testHost", "20.1.1.1", attrs, 101)
	time.Sleep(time.Millisecond * 10)

	// delete the endpoint
	err = watcher.DeleteEndpoint("default", "default", "testEndpoint", "testVm", "01:02:03:04:05:06", "testHost", "20.1.1.1")
	AssertOk(t, err, "Error deleting endpoint")

	// verify endpoint got deleted
	AssertEventually(t, func() bool {
		_, perr := stateMgr.FindObject("Endpoint", "default", "testEndpoint")
		return (perr != nil)
	}, "Endpoint still found in statemgr")

	// tets duplicate delete
	watcher.DeleteEndpoint("default", "default", "testEndpoint", "testVm", "01:02:03:04:05:06", "testHost", "20.1.1.1")

	// close the channels
	watcher.Stop()
	time.Sleep(time.Millisecond * 10)
}

func TestSecurityGroupWatcher(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, "", "")
	if err != nil {
		t.Fatalf("Error creating api server watcher. Err: %v", err)
		return
	}

	// create an sg
	err = watcher.CreateSecurityGroup("default", "testsg", []string{"env:production", "app:procurement"})
	AssertOk(t, err, "Error creating sg")

	// verify sg got created
	AssertEventually(t, func() bool {
		_, serr := stateMgr.FindSecurityGroup("default", "testsg")
		return (serr == nil)
	}, "Sg not found in statemgr")
	sg, err := stateMgr.FindSecurityGroup("default", "testsg")
	AssertOk(t, err, "Could not find the sg")
	Assert(t, (sg.TypeMeta.Kind == "SecurityGroup"), "sg object type meta did not match", sg)
	Assert(t, (len(sg.Spec.WorkloadSelector) == 2), "sg selector did not match", sg)
	Assert(t, (sg.Spec.WorkloadSelector[0] == "env:production"), "sg selector did not match", sg)

	// trigger a delete event
	err = watcher.DeleteSecurityGroup("default", "testsg")
	AssertOk(t, err, "Error deleting sg")

	// verify network is removed
	AssertEventually(t, func() bool {
		_, serr := stateMgr.FindSecurityGroup("default", "testsg")
		return (serr != nil)
	}, "Sg still found in statemgr")

	// close the channels
	watcher.Stop()
}

func TestSgPolicyWatcher(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, "", "")
	if err != nil {
		t.Fatalf("Error creating api server watcher. Err: %v", err)
		return
	}

	// create an sg
	err = watcher.CreateSecurityGroup("default", "testsg", []string{"env:production", "app:procurement"})
	AssertOk(t, err, "Error creating sg")

	// wait a little for watch even to propagate
	AssertEventually(t, func() bool {
		_, serr := stateMgr.FindSecurityGroup("default", "testsg")
		return (serr == nil)
	}, "Sg not found in statemgr")

	// rules
	inrules := []network.SGRule{
		{
			Ports:  "tcp/80",
			Action: "Allow",
		},
	}
	outrules := []network.SGRule{
		{
			Ports:  "tcp/80",
			Action: "Allow",
		},
	}

	// create an sg policy
	err = watcher.CreateSgpolicy("default", "pol1", []string{"testsg"}, inrules, outrules)
	AssertOk(t, err, "Error creating sg policy")

	// verify the sg policy exists
	AssertEventually(t, func() bool {
		_, serr := stateMgr.FindSgpolicy("default", "pol1")
		return (serr == nil)
	}, "Sg policy not found in statemgr")
	sgp, err := stateMgr.FindSgpolicy("default", "pol1")
	AssertOk(t, err, "Could not find the sg policy")
	Assert(t, (sgp.TypeMeta.Kind == "Sgpolicy"), "sg policy object type meta did not match", sgp)
	Assert(t, (len(sgp.Spec.AttachGroups) == 1), "sg policy attachment did not match", sgp)
	Assert(t, (sgp.Spec.AttachGroups[0] == "testsg"), "sg policy attachment did not match", sgp)
	Assert(t, (len(sgp.Spec.InRules) == 1), "sg policy rules did not match", sgp)
	Assert(t, (len(sgp.Spec.OutRules) == 1), "sg policy rules did not match", sgp)

	err = watcher.DeleteSgpolicy("default", "pol1")
	AssertOk(t, err, "Error deleting sg policy")

	// verify sgpolicy is gone
	AssertEventually(t, func() bool {
		_, serr := stateMgr.FindSgpolicy("default", "pol1")
		return (serr != nil)
	}, "Sg policy still found in statemgr")

	// close the channels
	watcher.Stop()
}
