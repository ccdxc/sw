// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package watcher

import (
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/ctrler/npm/statemgr"
	"github.com/pensando/sw/venice/utils/debug"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// dummy writer
type dummyWriter struct {
	// no fields
}

func (d *dummyWriter) WriteNetwork(nw *network.Network) error {
	return nil
}

func (d *dummyWriter) WriteEndpoint(ep *workload.Endpoint, update bool) error {
	return nil
}

func (d *dummyWriter) WriteSecurityGroup(sg *security.SecurityGroup) error {
	return nil
}

func (d *dummyWriter) WriteSgPolicy(sgp *security.Sgpolicy) error {
	return nil
}

func (d *dummyWriter) WriteTenant(tn *cluster.Tenant) error {
	return nil
}

func (d *dummyWriter) Close() error {
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
	watcher, err := NewWatcher(stateMgr, "", "", nil, debug.New(t.Name()).Build())
	if err != nil {
		t.Fatalf("Error creating api server watcher. Err: %v", err)
		return
	}

	// create a tenant
	err = watcher.CreateTenant("testTenant", "testTenant")
	AssertOk(t, err, "Error creating tenant")
	// verify tenant got created
	AssertEventually(t, func() (bool, interface{}) {
		_, terr := stateMgr.FindTenant("testTenant")
		return terr == nil, nil
	}, "Tenant not found in statemgr")
	ts, err := stateMgr.FindTenant("testTenant")
	AssertOk(t, err, "Could not find the tenant")
	AssertEquals(t, "testTenant", ts.Name, "Tenant name did not match")

	// create a network
	err = watcher.CreateNetwork("testTenant", "testTenant", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating network")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindNetwork("testTenant", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found in statemgr")
	nw, err := stateMgr.FindNetwork("testTenant", "testNetwork")
	AssertOk(t, err, "Could not find the network")
	Assert(t, (nw.Spec.IPv4Subnet == "10.1.1.0/24"), "Network subnet did not match", nw)
	Assert(t, (nw.Spec.IPv4Gateway == "10.1.1.254"), "Network gateway did not match", nw)
	Assert(t, (nw.TypeMeta.Kind == "Network"), "Network type meta did not match", nw)

	// trigger a delete event
	err = watcher.DeleteNetwork("testTenant", "testNetwork")
	AssertOk(t, err, "Error deleting network")

	// verify network is removed
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindNetwork("testTenant", "testNetwork")
		return (nerr != nil), nil
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
	watcher, err := NewWatcher(stateMgr, "", "", nil, debug.New(t.Name()).Build())
	if err != nil {
		t.Fatalf("Error creating api server watcher. Err: %v", err)
		return
	}

	// create a tenant
	err = watcher.CreateTenant("testTenant", "testTenant")
	AssertOk(t, err, "Error creating tenant")
	// verify tenant got created
	AssertEventually(t, func() (bool, interface{}) {
		_, terr := stateMgr.FindTenant("testTenant")
		return terr == nil, nil
	}, "Tenant not found in statemgr")
	ts, err := stateMgr.FindTenant("testTenant")
	AssertOk(t, err, "Could not find the tenant")
	AssertEquals(t, "testTenant", ts.Name, "Tenant name did not match")

	// create a network
	err = watcher.CreateNetwork("testTenant", "testTenant", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating network")
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindNetwork("testTenant", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found in statemgr")

	// create a endpoint
	var attrs map[string]string
	err = watcher.CreateEndpoint("testTenant", "testTenant", "testNetwork", "testEndpoint", "testVm", "01:02:03:04:05:06", "testHost", "20.1.1.1", attrs, 101)
	AssertOk(t, err, "Error creating endpoint")

	// verify endpoint got created
	AssertEventually(t, func() (bool, interface{}) {
		_, perr := stateMgr.FindObject("Endpoint", "testTenant", "testEndpoint")
		return (perr == nil), nil
	}, "Endpoint not found in statemgr")
	ep, err := stateMgr.FindObject("Endpoint", "testTenant", "testEndpoint")
	AssertOk(t, err, "endpoint not found")
	Assert(t, (ep.GetObjectKind() == "Endpoint"), "Incorrect object kind", ep)
	Assert(t, (ep.GetObjectMeta().Name == "testEndpoint"), "Incorrect object params", ep.GetObjectMeta())

	// test duplicate create
	watcher.CreateEndpoint("testTenant", "testTenant", "testNetwork", "testEndpoint", "testVm", "01:02:03:04:05:06", "testHost", "20.1.1.1", attrs, 101)
	time.Sleep(time.Millisecond * 10)

	// delete the endpoint
	err = watcher.DeleteEndpoint("testTenant", "testNetwork", "testEndpoint", "testVm", "01:02:03:04:05:06", "testHost", "20.1.1.1")
	AssertOk(t, err, "Error deleting endpoint")

	// verify endpoint got deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, perr := stateMgr.FindObject("Endpoint", "testTenant", "testEndpoint")
		return (perr != nil), nil
	}, "Endpoint still found in statemgr")

	// tets duplicate delete
	watcher.DeleteEndpoint("testTenant", "testNetwork", "testEndpoint", "testVm", "01:02:03:04:05:06", "testHost", "20.1.1.1")

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
	watcher, err := NewWatcher(stateMgr, "", "", nil, debug.New(t.Name()).Build())
	if err != nil {
		t.Fatalf("Error creating api server watcher. Err: %v", err)
		return
	}

	// create a tenant
	err = watcher.CreateTenant("testTenant", "testTenant")
	AssertOk(t, err, "Error creating tenant")
	// verify tenant got created
	AssertEventually(t, func() (bool, interface{}) {
		_, terr := stateMgr.FindTenant("testTenant")
		return terr == nil, nil
	}, "Tenant not found in statemgr")
	ts, err := stateMgr.FindTenant("testTenant")
	AssertOk(t, err, "Could not find the tenant")
	AssertEquals(t, "testTenant", ts.Name, "Tenant name did not match")

	// create an sg
	err = watcher.CreateSecurityGroup("testTenant", "testTenant", "testsg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating sg")

	// verify sg got created
	AssertEventually(t, func() (bool, interface{}) {
		_, serr := stateMgr.FindSecurityGroup("testTenant", "testsg")
		return (serr == nil), nil
	}, "Sg not found in statemgr")
	sg, err := stateMgr.FindSecurityGroup("testTenant", "testsg")
	AssertOk(t, err, "Could not find the sg")
	Assert(t, (sg.TypeMeta.Kind == "SecurityGroup"), "sg object type meta did not match", sg)
	Assert(t, (len(sg.Spec.WorkloadSelector.GetRequirements()) == 2), "sg selector did not match", sg)
	Assert(t, strings.Contains(sg.Spec.WorkloadSelector.Print(), "env=production"), "sg selector did not match", sg)

	// trigger a delete event
	err = watcher.DeleteSecurityGroup("testTenant", "testsg")
	AssertOk(t, err, "Error deleting sg")

	// verify network is removed
	AssertEventually(t, func() (bool, interface{}) {
		_, serr := stateMgr.FindSecurityGroup("testTenant", "testsg")
		return (serr != nil), nil
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
	watcher, err := NewWatcher(stateMgr, "", "", nil, debug.New(t.Name()).Build())
	if err != nil {
		t.Fatalf("Error creating api server watcher. Err: %v", err)
		return
	}
	// create a tenant
	err = watcher.CreateTenant("testTenant", "testTenant")
	AssertOk(t, err, "Error creating tenant")
	// verify tenant got created
	AssertEventually(t, func() (bool, interface{}) {
		_, terr := stateMgr.FindTenant("testTenant")
		return terr == nil, nil
	}, "Tenant not found in statemgr")
	ts, err := stateMgr.FindTenant("testTenant")
	AssertOk(t, err, "Could not find the tenant")
	AssertEquals(t, "testTenant", ts.Name, "Tenant name did not match")

	// create an sg
	err = watcher.CreateSecurityGroup("testTenant", "testTenant", "testsg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating sg")

	// wait a little for watch even to propagate
	AssertEventually(t, func() (bool, interface{}) {
		_, serr := stateMgr.FindSecurityGroup("testTenant", "testsg")
		return (serr == nil), nil
	}, "Sg not found in statemgr")

	// rules
	inrules := []security.SGRule{
		{
			Ports:  "tcp/80",
			Action: "Allow",
		},
	}
	outrules := []security.SGRule{
		{
			Ports:  "tcp/80",
			Action: "Allow",
		},
	}

	// create an sg policy
	err = watcher.CreateSgpolicy("testTenant", "testTenant", "pol1", []string{"testsg"}, inrules, outrules)
	AssertOk(t, err, "Error creating sg policy")

	// verify the sg policy exists
	AssertEventually(t, func() (bool, interface{}) {
		_, serr := stateMgr.FindSgpolicy("testTenant", "pol1")
		return (serr == nil), nil
	}, "Sg policy not found in statemgr")
	sgp, err := stateMgr.FindSgpolicy("testTenant", "pol1")
	AssertOk(t, err, "Could not find the sg policy")
	Assert(t, (sgp.TypeMeta.Kind == "Sgpolicy"), "sg policy object type meta did not match", sgp)
	Assert(t, (len(sgp.Spec.AttachGroups) == 1), "sg policy attachment did not match", sgp)
	Assert(t, (sgp.Spec.AttachGroups[0] == "testsg"), "sg policy attachment did not match", sgp)
	Assert(t, (len(sgp.Spec.InRules) == 1), "sg policy rules did not match", sgp)
	Assert(t, (len(sgp.Spec.OutRules) == 1), "sg policy rules did not match", sgp)

	err = watcher.DeleteSgpolicy("testTenant", "pol1")
	AssertOk(t, err, "Error deleting sg policy")

	// verify sgpolicy is gone
	AssertEventually(t, func() (bool, interface{}) {
		_, serr := stateMgr.FindSgpolicy("testTenant", "pol1")
		return (serr != nil), nil
	}, "Sg policy still found in statemgr")

	// close the channels
	watcher.Stop()
}

func TestTenantWatcher(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create state manager. Err: %v", err)
		return
	}

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, "", "", nil, debug.New(t.Name()).Build())
	if err != nil {
		t.Fatalf("Error creating api server watcher. Err: %v", err)
		return
	}

	// create a tenant
	err = watcher.CreateTenant("testTenant", "testTenant")
	AssertOk(t, err, "Error creating tenant")

	// verify tenant got created
	AssertEventually(t, func() (bool, interface{}) {
		_, terr := stateMgr.FindTenant("testTenant")
		return terr == nil, nil
	}, "Tenant not found in statemgr")
	ts, err := stateMgr.FindTenant("testTenant")
	AssertOk(t, err, "Could not find the tenant")
	AssertEquals(t, "testTenant", ts.Name, "Tenant name did not match")

	// trigger a delete event
	err = watcher.DeleteTenant("testTenant")
	AssertOk(t, err, "Error deleting tenant")

	// verify tenant is removed
	AssertEventually(t, func() (bool, interface{}) {
		_, terr := stateMgr.FindTenant("testTenant")
		return (terr != nil), nil
	}, "Tenant still found after deleting")

	// close the channels
	watcher.Stop()
}

func TestRestartWatchers(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create state manager. Err: %v", err)
		return
	}

	w, err := NewWatcher(stateMgr, "", "", nil, debug.New(t.Name()).Build())
	if err != nil {
		t.Fatalf("Error creating api server watcher. Err: %v", err)
		return
	}

	// Trigger watch restarts by forcibly closing the channels
	close(w.netWatcher)
	close(w.sgPolicyWatcher)
	close(w.sgWatcher)
	close(w.vmmEpWatcher)
	close(w.tenantWatcher)

	// Sleep for 30 seconds to ensure that all the watchers are restarted. Do this only in test code.
	time.Sleep(time.Second * 30)

	// Ensure object creates go through.
	// create a tenant
	err = w.CreateTenant("testTenant", "testTenant")
	AssertOk(t, err, "Error creating tenant")
	// verify tenant got created
	AssertEventually(t, func() (bool, interface{}) {
		_, terr := stateMgr.FindTenant("testTenant")
		return terr == nil, nil
	}, "Tenant not found in statemgr")
	ts, err := stateMgr.FindTenant("testTenant")
	AssertOk(t, err, "Could not find the tenant")
	AssertEquals(t, "testTenant", ts.Name, "Tenant name did not match")

	// create a network
	err = w.CreateNetwork("testTenant", "testTenant", "testNetwork", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating network")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindNetwork("testTenant", "testNetwork")
		return nerr == nil, nil
	}, "Network not found in statemgr")
	nw, err := stateMgr.FindNetwork("testTenant", "testNetwork")
	AssertOk(t, err, "Could not find the network")
	Assert(t, nw.Spec.IPv4Subnet == "10.1.1.0/24", "Network subnet did not match", nw)
	Assert(t, nw.Spec.IPv4Gateway == "10.1.1.254", "Network gateway did not match", nw)
	Assert(t, nw.TypeMeta.Kind == "Network", "Network type meta did not match", nw)

	// security group and security group policy objects
	err = w.CreateSecurityGroup("testTenant", "testTenant", "testsg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating sg")

	// wait a little for watch even to propagate
	AssertEventually(t, func() (bool, interface{}) {
		_, serr := stateMgr.FindSecurityGroup("testTenant", "testsg")
		return serr == nil, nil
	}, "Sg not found in statemgr")

	// rules
	inrules := []security.SGRule{
		{
			Ports:  "tcp/80",
			Action: "Allow",
		},
	}
	outrules := []security.SGRule{
		{
			Ports:  "tcp/80",
			Action: "Allow",
		},
	}

	// create an sg policy
	err = w.CreateSgpolicy("testTenant", "testTenant", "pol1", []string{"testsg"}, inrules, outrules)
	AssertOk(t, err, "Error creating sg policy")

	// verify the sg policy exists
	AssertEventually(t, func() (bool, interface{}) {
		_, serr := stateMgr.FindSgpolicy("testTenant", "pol1")
		return serr == nil, nil
	}, "Sg policy not found in statemgr")
	sgp, err := stateMgr.FindSgpolicy("testTenant", "pol1")
	AssertOk(t, err, "Could not find the sg policy")
	Assert(t, sgp.TypeMeta.Kind == "Sgpolicy", "sg policy object type meta did not match", sgp)
	Assert(t, len(sgp.Spec.AttachGroups) == 1, "sg policy attachment did not match", sgp)
	Assert(t, sgp.Spec.AttachGroups[0] == "testsg", "sg policy attachment did not match", sgp)
	Assert(t, len(sgp.Spec.InRules) == 1, "sg policy rules did not match", sgp)
	Assert(t, len(sgp.Spec.OutRules) == 1, "sg policy rules did not match", sgp)

	// create a endpoint
	var attrs map[string]string
	err = w.CreateEndpoint("testTenant", "testTenant", "testNetwork", "testEndpoint", "testVm", "01:02:03:04:05:06", "testHost", "20.1.1.1", attrs, 101)
	AssertOk(t, err, "Error creating endpoint")

	// verify endpoint got created
	AssertEventually(t, func() (bool, interface{}) {
		_, perr := stateMgr.FindObject("Endpoint", "testTenant", "testEndpoint")
		return perr == nil, nil
	}, "Endpoint not found in statemgr")
	ep, err := stateMgr.FindObject("Endpoint", "testTenant", "testEndpoint")
	AssertOk(t, err, "endpoint not found")
	Assert(t, ep.GetObjectKind() == "Endpoint", "Incorrect object kind", ep)
	Assert(t, ep.GetObjectMeta().Name == "testEndpoint", "Incorrect object params", ep.GetObjectMeta())
	w.Stop()

}
