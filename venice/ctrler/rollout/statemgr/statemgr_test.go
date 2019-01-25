package statemgr

import (
	"testing"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	roproto "github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/memdb"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// dummy writer
type dummyWriter struct {
	// no fields
}

func (d *dummyWriter) WriteRollout(ro *roproto.Rollout) error {
	return nil
}
func (d *dummyWriter) Close() error {
	return nil
}

func TestVeniceRolloutWatch(t *testing.T) {
	// Create VeniceRolloutObject and see that its properly created and that watchers see the updates to the object

	// create  state manager
	stateMgr, err := NewStatemgr(&dummyWriter{})
	AssertOk(t, err, "Error creating StateMgr")
	defer stateMgr.Stop()

	// start a watch
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	err = stateMgr.WatchObjects("VeniceRollout", watchChan)
	AssertOk(t, err, "Error creating the Watch for VeniceRollout")

	ro := protos.VeniceRollout{
		TypeMeta: api.TypeMeta{
			Kind: "VeniceRollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   t.Name(),
			Tenant: "default",
		},
	}
	err = stateMgr.CreateVeniceRolloutState(&ro, &RolloutState{})
	AssertOk(t, err, "Error creating the Venice Rollout")

	// verify we get a watch event
	select {
	case wnt, ok := <-watchChan:
		Assert(t, ok, "Error reading from channel", wnt)
		Assert(t, wnt.Obj.GetObjectMeta().Name == t.Name(), "Received invalid venice Rollout", wnt)
		Assert(t, wnt.Obj.GetObjectKind() == "VeniceRollout", "Received invalid Kind venice Rollout", wnt)

	case <-time.After(time.Second):
		t.Fatalf("Timed out while waiting for channel event")
	}

	// verify the list of Venice Rollouts
	vro, err := stateMgr.ListVeniceRollouts()
	AssertOk(t, err, "Error listing Venice Rollouts")
	Assert(t, len(vro) == 1, "Incorrect number of veniceRollout received", vro)
	Assert(t, vro[0].Name == t.Name(), "Incorrect veniceRollout Present", vro[0])

	stateMgr.DeleteVeniceRolloutState(&ro)
	vro, err = stateMgr.ListVeniceRollouts()
	AssertOk(t, err, "Error listing Venice Rollouts")
	Assert(t, len(vro) == 0, "veniceRollout Exists even after deletion", vro)
}

// checkNoVeniceRolloutHelper returns true if there are no venice rollouts
func checkNoVeniceRolloutHelper(t *testing.T, stateMgr *Statemgr) (bool, interface{}) {
	sros, err := stateMgr.ListVeniceRollouts()
	AssertOk(t, err, "Error Listing VeinceRollouts")
	if sros == nil || len(sros) == 0 {
		return true, nil
	}
	return false, nil
}

// 	checkNoPendingVeniceOp returns true if the Op has not been issued (or it has been issued, then should have status)
//		returns false if the Op is present in Spec and Not in Status
func checkNoPendingVeniceOp(t *testing.T, stateMgr *Statemgr, node string, op protos.VeniceOp, version string) (bool, interface{}) {
	vros, err := stateMgr.ListVeniceRollouts()
	AssertOk(t, err, "Error Listing VeniceRollouts")
	for _, vro := range vros {
		if vro.Name != node {
			continue
		}
		found := false
		for _, o := range vro.Spec.Ops {
			if o.Op == op && o.Version == version {
				found = true
			}
		}
		if !found {
			return true, "op not found in spec"
		}
		_, found = vro.status[op]
		if found {
			return true, "Status also present"
		}

		return false, vro
	}
	return true, "venice node not present"
}

// 	Add a response on node if op is present in the Spec and not present in Status
func addVeniceResponseHelper(t *testing.T, stateMgr *Statemgr, node string, op protos.VeniceOp, version string) (bool, interface{}) {
	vros, err := stateMgr.ListVeniceRollouts()
	AssertOk(t, err, "Error Listing VeniceRollouts")
	for _, vro := range vros {
		if vro.Name != node {
			continue
		}
		found := false
		for _, o := range vro.Spec.Ops {
			if o.Op == op && o.Version == version {
				found = true
			}
		}
		if !found {
			return false, "op not found in spec"
		}
		_, found = vro.status[op]
		if found {
			return false, "Status already present"
		}

		var newOpStatus []protos.VeniceOpStatus
		for _, s := range vro.status {
			newOpStatus = append(newOpStatus, s)
		}
		newOpStatus = append(newOpStatus, protos.VeniceOpStatus{
			Op:       op,
			Version:  version,
			OpStatus: "success",
			Message:  "successful op in venice",
		})
		status := protos.VeniceRolloutStatus{
			OpStatus: newOpStatus,
		}
		vro.UpdateVeniceRolloutStatus(&status)
		return true, vro

	}
	return false, "venice node not present"
}

func IsFSMInState(t *testing.T, stateMgr *Statemgr, roName string, st rofsmState) (bool, interface{}) {
	ros, err := stateMgr.GetRolloutState("default", roName)
	AssertOk(t, err, "Error Listing ServiceRollouts")
	return ros.currentState == st, ros.currentState
}

// 	Add a response if service status is pending and returns true
func addServiceResponseFilter(t *testing.T, stateMgr *Statemgr, version string) (bool, interface{}) {
	sros, err := stateMgr.ListServiceRollouts()
	AssertOk(t, err, "Error Listing ServiceRollouts")
	for _, sro := range sros {
		found := false
		for _, o := range sro.Spec.Ops {
			if o.Version == version {
				found = true
			}
		}
		if !found {
			return false, "op not found in spec"
		}
		_, found = sro.status[protos.ServiceOp_ServiceRunVersion]
		if found {
			return false, "Status already present"
		}

		var newOpStatus []protos.ServiceOpStatus
		for _, s := range sro.status {
			newOpStatus = append(newOpStatus, s)
		}
		newOpStatus = append(newOpStatus, protos.ServiceOpStatus{
			Op:       protos.ServiceOp_ServiceRunVersion,
			Version:  version,
			OpStatus: "success",
			Message:  "successful op in Service",
		})
		status := protos.ServiceRolloutStatus{
			OpStatus: newOpStatus,
		}
		sro.UpdateServiceRolloutStatus(&status)
		return true, sro
	}
	return false, "service Rollout not present"
}

// 	checkNoPendingSmartNICOp returns true if the Op has not been issued (or it has been issued, then should have status)
//		returns false if the Op is present in Spec and Not in Status
func checkNoPendingSmartNICOp(t *testing.T, stateMgr *Statemgr, node string, op protos.SmartNICOp, version string) (bool, interface{}) {
	sros, err := stateMgr.ListSmartNICRollouts()
	AssertOk(t, err, "Error Listing SmartNICRollouts")
	for _, sro := range sros {
		if sro.Name != node {
			continue
		}
		found := false
		for _, o := range sro.Spec.Ops {
			if o.Op == op && o.Version == version {
				found = true
			}
		}
		if !found {
			return true, "op not found in spec"
		}
		if sro.status[op].OpStatus != "" {
			return true, "Status also present"
		}
		return false, sro
	}
	return true, "smartnic not present"
}

// 	Add a response on node if op is present in the Spec and not present in Status
func addSmartNICResponseFilter(t *testing.T, stateMgr *Statemgr, snic string, op protos.SmartNICOp, version, status string) (bool, interface{}) {
	sros, err := stateMgr.ListSmartNICRollouts()
	AssertOk(t, err, "Error Listing SmartNICRollouts")
	for _, vro := range sros {
		if vro.Name != snic {
			continue
		}
		found := false
		for _, o := range vro.Spec.Ops {
			if o.Op == op && o.Version == version {
				found = true
			}
		}
		if !found {
			return false, "op not found in spec"
		}
		if vro.status[op].OpStatus != "" {
			return false, "Status also present"
		}

		var newOpStatus []protos.SmartNICOpStatus
		for _, s := range vro.status {
			newOpStatus = append(newOpStatus, s)
		}
		newOpStatus = append(newOpStatus, protos.SmartNICOpStatus{
			Op:       op,
			Version:  version,
			OpStatus: status,
			Message:  status + " op in venice",
		})
		status := protos.SmartNICRolloutStatus{
			OpStatus: newOpStatus,
		}
		vro.UpdateSmartNICRolloutStatus(&status)
		return true, vro

	}
	return false, "smartNIC is not present"
}

func createSNICHelper(stateMgr *Statemgr, name string, labels map[string]string) {
	sn := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{
			Kind: "SmartNIC",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: "default",
			Labels: labels,
		},
	}
	stateMgr.handleSmartNICEvent(kvstore.Created, &sn)
}
func createNodeHelper(stateMgr *Statemgr, name string) {
	node := cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Tenant: "default",
		},
	}
	stateMgr.handleNodeEvent(kvstore.Created, &node)
}

func TestVeniceOnlyRollout(t *testing.T) {
	const version = "v1.1"
	// create  state manager
	stateMgr, err := NewStatemgr(&dummyWriter{})
	AssertOk(t, err, "Error creating StateMgr")
	defer stateMgr.Stop()

	createNode := func(name string) { createNodeHelper(stateMgr, name) }
	createSNIC := func(name string, labels map[string]string) { createSNICHelper(stateMgr, name, labels) }

	createNode("node1")
	createNode("node2")

	createNode("node0")
	n0 := cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "node0",
			Tenant: "default",
		},
	}
	stateMgr.handleNodeEvent(kvstore.Deleted, &n0)

	ro := roproto.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   t.Name(),
			Tenant: "default",
		},
		Spec: roproto.RolloutSpec{
			Version:                     version,
			ScheduledStartTime:          nil,
			Duration:                    "",
			Strategy:                    "",
			MaxParallel:                 3,
			MaxNICFailuresBeforeAbort:   0,
			OrderConstraints:            nil,
			Suspend:                     false,
			SmartNICsOnly:               false,
			SmartNICMustMatchConstraint: true, // hence venice upgrade only
		},
	}
	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &ro,
	}
	stateMgr.RolloutWatcher <- evt

	createSNIC("naples1", map[string]string{"l1": "n1"})

	// Return true if node has op  in Spec and not in status.  Also adds success to its status for this op
	addVeniceResponse := func(node string, op protos.VeniceOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			return addVeniceResponseHelper(t, stateMgr, node, op, version)
		}
	}
	// Return true if node does not have op in its pending operation
	// (i.e either op is not issued or if it has been issued is already present in status)
	checkNoVeniceOutstandingReq := func(node string, op protos.VeniceOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			ret, i := addVeniceResponseHelper(t, stateMgr, node, op, version)
			return !ret, i
		}
	}

	AssertEventually(t, checkNoVeniceOutstandingReq("node1", protos.VeniceOp_VeniceRunVersion), "Expected node1 spec not to have outstanding RunVersion Op")

	AssertEventually(t, addVeniceResponse("node1", protos.VeniceOp_VenicePreCheck), "Expected node1 spec to have outstanding Precheck Op")
	AssertEventually(t, addVeniceResponse("node2", protos.VeniceOp_VenicePreCheck), "Expected node2 spec to have outstanding Precheck Op")
	AssertEventually(t, addVeniceResponse("node1", protos.VeniceOp_VeniceRunVersion), "Expected node1 spec to have outstanding RunVersion Op")
	AssertEventually(t, addVeniceResponse("node2", protos.VeniceOp_VeniceRunVersion), "Expected node2 spec to have outstanding RunVersion Op")

	addServiceResponse := func() func() (bool, interface{}) {
		return func() (bool, interface{}) {
			return addServiceResponseFilter(t, stateMgr, version)
		}
	}
	AssertEventually(t, addServiceResponse(), "Expected Service Rollout to have outstanding Op")

	// Test that there is no request issued for any smartNIC
	checkNoVeniceSmartNICReq := func() (bool, interface{}) {
		sros, err := stateMgr.ListSmartNICRollouts()
		AssertOk(t, err, "Error Listing SmartNICRollouts")
		if sros == nil || len(sros) == 0 {
			return true, nil
		}
		return false, nil
	}
	AssertConsistently(t, checkNoVeniceSmartNICReq, "There should be no smartnic Rollout", "100ms", "1s")

	ros, err := stateMgr.GetRolloutState("default", t.Name())
	AssertOk(t, err, "GetRolloutstate should succeed")
	AssertEventually(t, func() (bool, interface{}) {
		if len(ros.Status.ControllerNodesStatus) != 2 {
			return false, ros.Status
		}
		if len(ros.Status.ControllerServicesStatus) != 1 {
			return false, ros.Status
		}
		if ros.Status.ControllerServicesStatus[0].Phase != roproto.RolloutPhase_COMPLETE.String() ||
			ros.Status.ControllerNodesStatus[0].Phase != roproto.RolloutPhase_COMPLETE.String() ||
			ros.Status.ControllerNodesStatus[1].Phase != roproto.RolloutPhase_COMPLETE.String() {
			return false, ros.Status
		}
		return true, nil
	}, "Phase in Status should be correct")

	stateMgr.RolloutWatcher <- kvstore.WatchEvent{Type: kvstore.Deleted, Object: &ro}
}

func TestSNICOrder(t *testing.T) {
	const version = "v1.1"
	// create  state manager
	stateMgr, err := NewStatemgr(&dummyWriter{})
	AssertOk(t, err, "Error creating StateMgr")
	defer stateMgr.Stop()

	createSNIC := func(name string, labels map[string]string) { createSNICHelper(stateMgr, name, labels) }

	createSNIC("naples1", map[string]string{"l": "n1"})
	createSNIC("naples2", map[string]string{"l": "n2"})

	snStates, err := stateMgr.ListSmartNICs()
	AssertOk(t, err, "Error listing smartNICs")

	sn := orderSmartNICs(nil, false, snStates)
	Assert(t, len(sn) == 1, "Should be one bucket")
	Assert(t, len(sn[0]) == 2, "first bucket should have 2 naples")
	Assert(t, sn[0][0].Name != sn[0][1].Name, "names of naples should be different")

	l1, err := labels.Parse("l=n1")
	AssertOk(t, err, "Error parsing label")
	l12, err := labels.Parse("l in (n1,n2)")
	AssertOk(t, err, "Error parsing label")

	// match only for naples with label n1
	sn = orderSmartNICs([]*labels.Selector{l1}, true, snStates)
	Assert(t, len(sn) == 1, "Should be one bucket")
	Assert(t, len(sn[0]) == 1, "first bucket should have 1 naples")
	Assert(t, sn[0][0].Name == "naples1", "expecting naples1")

	sn = orderSmartNICs([]*labels.Selector{l1}, false, snStates)
	Assert(t, len(sn) == 2, "Should be one bucket")
	Assert(t, len(sn[0]) == 1, "first bucket should have 1 naples")
	Assert(t, len(sn[1]) == 1, "second bucket should have 1 naples")
	Assert(t, sn[0][0].Name == "naples1", "expecting naples1")
	Assert(t, sn[1][0].Name == "naples2", "expecting naples2")

	createSNIC("naples3", map[string]string{"l": "n2"})
	snStates, err = stateMgr.ListSmartNICs()
	AssertOk(t, err, "Error listing smartNICs")

	sn = orderSmartNICs([]*labels.Selector{l1}, false, snStates)
	Assert(t, len(sn) == 2, "Should be one bucket")
	Assert(t, len(sn[0]) == 1, "first bucket should have 1 naples")
	Assert(t, len(sn[1]) == 2, "second bucket should have 2 naples")
	Assert(t, sn[0][0].Name == "naples1", "expecting naples1")

	createSNIC("naples4", map[string]string{"l": "n3"})
	snStates, err = stateMgr.ListSmartNICs()
	AssertOk(t, err, "Error listing smartNICs")
	sn = orderSmartNICs([]*labels.Selector{l12}, false, snStates)
	Assert(t, len(sn) == 2, "Should be one bucket")
	Assert(t, len(sn[0]) == 3, "first bucket should have 3 naples")
	Assert(t, len(sn[1]) == 1, "second bucket should have 1 naples")

}

func TestSNICOnlyRollout(t *testing.T) {
	const version = "v1.1"
	// create  state manager
	stateMgr, err := NewStatemgr(&dummyWriter{})
	AssertOk(t, err, "Error creating StateMgr")
	defer stateMgr.Stop()

	createNode := func(name string) { createNodeHelper(stateMgr, name) }
	createSNIC := func(name string, labels map[string]string) { createSNICHelper(stateMgr, name, labels) }

	createNode("node1")

	createSNIC("naples1", map[string]string{"l1": "n1"})
	createSNIC("naples0", map[string]string{"l1": "n0"})

	sn := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{
			Kind: "SmartNIC",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "naples0",
			Tenant: "default",
		},
	}
	stateMgr.handleSmartNICEvent(kvstore.Deleted, &sn)

	ro := roproto.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   t.Name(),
			Tenant: "default",
		},
		Spec: roproto.RolloutSpec{
			Version:                     version,
			ScheduledStartTime:          nil,
			Duration:                    "",
			Strategy:                    "",
			MaxParallel:                 3,
			MaxNICFailuresBeforeAbort:   0,
			OrderConstraints:            nil,
			Suspend:                     false,
			SmartNICsOnly:               true,
			SmartNICMustMatchConstraint: false, // hence smartnic upgrade only
		},
	}
	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &ro,
	}
	stateMgr.RolloutWatcher <- evt

	// Return true if if veniceRollout was not created
	checkNoVeniceReq := func() (bool, interface{}) {
		return checkNoVeniceRolloutHelper(t, stateMgr)
	}

	AssertConsistently(t, checkNoVeniceReq, "Expected no venice rollouts", "100ms", "1s")

	addSmartNICResponse := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			return addSmartNICResponseFilter(t, stateMgr, snic, op, version, "success")
		}
	}

	// Test that there is no request issued for any smartNIC
	checkNoSmartNICReq := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			ret, i := checkNoPendingSmartNICOp(t, stateMgr, snic, op, version)
			return ret, i
		}
	}
	AssertConsistently(t, checkNoSmartNICReq("naples1", protos.SmartNICOp_SmartNICDisruptiveUpgrade), "Expected naples1 spec not to have RunVersion", "100ms", "1s")
	AssertEventually(t, addSmartNICResponse("naples1", protos.SmartNICOp_SmartNICPreCheckForDisruptive), "Expected naples1 spec to have outstanding Precheck Op")
	AssertEventually(t, addSmartNICResponse("naples1", protos.SmartNICOp_SmartNICDisruptiveUpgrade), "Expected naples1 spec to have outstanding RunVersion Op")
	AssertEventually(t, func() (bool, interface{}) { return IsFSMInState(t, stateMgr, t.Name(), fsmstRolloutSuccess) }, "Expecting Rollout to be succesfull state")

	// Deleting the rollout should delete all the venice/service/smartnic rollout objects
	stateMgr.RolloutWatcher <- kvstore.WatchEvent{Type: kvstore.Deleted, Object: &ro}
	AssertConsistently(t, checkNoVeniceReq, "Expected no venice rollouts", "100ms", "1s")
}

func TestFutureRollout(t *testing.T) {
	const version = "v1.1"
	// create  state manager
	stateMgr, err := NewStatemgr(&dummyWriter{})
	AssertOk(t, err, "Error creating StateMgr")
	defer stateMgr.Stop()

	createSNIC := func(name string, labels map[string]string) { createSNICHelper(stateMgr, name, labels) }
	createSNIC("naples1", map[string]string{"l1": "n1"})

	seconds := time.Now().Unix()
	ts := &api.Timestamp{
		Timestamp: types.Timestamp{
			Seconds: seconds + 5,
		},
	}

	ro := roproto.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   t.Name(),
			Tenant: "default",
		},
		Spec: roproto.RolloutSpec{
			Version:                     version,
			ScheduledStartTime:          ts,
			Duration:                    "",
			Strategy:                    "",
			MaxParallel:                 3,
			MaxNICFailuresBeforeAbort:   0,
			OrderConstraints:            nil,
			Suspend:                     false,
			SmartNICsOnly:               true,
			SmartNICMustMatchConstraint: false, // hence smartnic upgrade only
			UpgradeType:                 roproto.RolloutSpec_Disruptive.String(),
		},
	}
	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &ro,
	}
	stateMgr.RolloutWatcher <- evt

	addSmartNICResponse := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			return addSmartNICResponseFilter(t, stateMgr, snic, op, version, "success")
		}
	}

	// Test that there is no request issued for any smartNIC
	checkNoSmartNICReq := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			ret, i := checkNoPendingSmartNICOp(t, stateMgr, snic, op, version)
			return ret, i
		}
	}
	AssertEventually(t, addSmartNICResponse("naples1", protos.SmartNICOp_SmartNICPreCheckForDisruptive), "Expected naples1 spec to have outstanding Precheck Op")
	AssertConsistently(t, checkNoSmartNICReq("naples1", protos.SmartNICOp_SmartNICDisruptiveUpgrade), "Expected naples1 spec not to have PreCheck till timeout", "100ms", "2s")
	AssertEventually(t, addSmartNICResponse("naples1", protos.SmartNICOp_SmartNICDisruptiveUpgrade), "Expected naples1 spec to have outstanding RunVersion Op")

}

// When a preupgrade fails, the rollout should go to Failed state
func TestPreUpgFail(t *testing.T) {
	const version = "v1.1"
	// create  state manager
	stateMgr, err := NewStatemgr(&dummyWriter{})
	AssertOk(t, err, "Error creating StateMgr")
	defer stateMgr.Stop()

	createNode := func(name string) { createNodeHelper(stateMgr, name) }
	createSNIC := func(name string, labels map[string]string) { createSNICHelper(stateMgr, name, labels) }

	createNode("node1")

	createSNIC("naples1", map[string]string{"l1": "n1"})

	ro := roproto.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   t.Name(),
			Tenant: "default",
		},
		Spec: roproto.RolloutSpec{
			Version:                     version,
			ScheduledStartTime:          nil,
			Duration:                    "",
			Strategy:                    "",
			MaxParallel:                 3,
			MaxNICFailuresBeforeAbort:   0,
			OrderConstraints:            nil,
			Suspend:                     false,
			SmartNICsOnly:               true,
			SmartNICMustMatchConstraint: false, // hence smartnic upgrade only
		},
	}
	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &ro,
	}
	stateMgr.RolloutWatcher <- evt

	addSmartNICResponse := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			return addSmartNICResponseFilter(t, stateMgr, snic, op, version, "fail")
		}
	}

	AssertEventually(t, addSmartNICResponse("naples1", protos.SmartNICOp_SmartNICPreCheckForDisruptive), "Expected naples1 spec to have outstanding Precheck Op")
	AssertEventually(t, func() (bool, interface{}) { return IsFSMInState(t, stateMgr, t.Name(), fsmstRolloutFail) }, "Expecting Rollout to be in Failure  state", "100ms", "2s")
}

// When one smartNIC does not respond to preupgrade, the rollout should go to Failed state
func TestPreUpgTimeout(t *testing.T) {

	savedPreupgradeTimeout := preUpgradeTimeout
	preUpgradeTimeout = 100 * time.Millisecond
	defer func() {
		preUpgradeTimeout = savedPreupgradeTimeout
	}()

	const version = "v1.1"
	// create  state manager
	stateMgr, err := NewStatemgr(&dummyWriter{})
	AssertOk(t, err, "Error creating StateMgr")
	defer stateMgr.Stop()

	createNode := func(name string) { createNodeHelper(stateMgr, name) }
	createSNIC := func(name string, labels map[string]string) { createSNICHelper(stateMgr, name, labels) }

	createNode("node1")

	createSNIC("naples1", map[string]string{"l1": "n1"})
	createSNIC("naples0", map[string]string{"l1": "n1"})

	ro := roproto.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   t.Name(),
			Tenant: "default",
		},
		Spec: roproto.RolloutSpec{
			Version:                     version,
			ScheduledStartTime:          nil,
			Duration:                    "",
			Strategy:                    "",
			MaxParallel:                 3,
			MaxNICFailuresBeforeAbort:   0,
			OrderConstraints:            nil,
			Suspend:                     false,
			SmartNICsOnly:               true,
			SmartNICMustMatchConstraint: false, // hence smartnic upgrade only
		},
	}
	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &ro,
	}
	stateMgr.RolloutWatcher <- evt

	addSmartNICResponse := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			return addSmartNICResponseFilter(t, stateMgr, snic, op, version, "success")
		}
	}

	AssertEventually(t, addSmartNICResponse("naples1", protos.SmartNICOp_SmartNICPreCheckForDisruptive), "Expected naples1 spec to have outstanding Precheck Op")
	AssertEventually(t, func() (bool, interface{}) { return IsFSMInState(t, stateMgr, t.Name(), fsmstRolloutFail) }, "Expecting Rollout to be in Failure  state", "100ms", "2s")
}

// When number of failures is Lessthan or Equal to MaxFailures, rollout should be considered success
func TestMaxFailuresNotHit(t *testing.T) {

	savedPreupgradeTimeout := preUpgradeTimeout
	preUpgradeTimeout = 100 * time.Millisecond
	defer func() {
		preUpgradeTimeout = savedPreupgradeTimeout
	}()

	const version = "v1.1"
	// create  state manager
	stateMgr, err := NewStatemgr(&dummyWriter{})
	AssertOk(t, err, "Error creating StateMgr")
	defer stateMgr.Stop()

	createSNIC := func(name string, labels map[string]string) { createSNICHelper(stateMgr, name, labels) }
	createSNIC("naples1", map[string]string{"l1": "n1"})
	createSNIC("naples0", map[string]string{"l1": "n1"})

	ro := roproto.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   t.Name(),
			Tenant: "default",
		},
		Spec: roproto.RolloutSpec{
			Version:                     version,
			ScheduledStartTime:          nil,
			Duration:                    "",
			Strategy:                    "",
			MaxParallel:                 3,
			MaxNICFailuresBeforeAbort:   1,
			OrderConstraints:            nil,
			Suspend:                     false,
			SmartNICsOnly:               true,
			SmartNICMustMatchConstraint: false, // hence smartnic upgrade only
		},
	}
	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &ro,
	}
	stateMgr.RolloutWatcher <- evt

	addSmartNICResponse := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			return addSmartNICResponseFilter(t, stateMgr, snic, op, version, "success")
		}
	}
	addSmartNICFailResponse := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			return addSmartNICResponseFilter(t, stateMgr, snic, op, version, "failure")
		}
	}
	AssertEventually(t, addSmartNICResponse("naples1", protos.SmartNICOp_SmartNICPreCheckForDisruptive), "Expected naples1 spec to have outstanding Precheck Op")
	AssertEventually(t, addSmartNICResponse("naples0", protos.SmartNICOp_SmartNICPreCheckForDisruptive), "Expected naples0 spec to have outstanding Precheck Op")
	AssertEventually(t, addSmartNICResponse("naples0", protos.SmartNICOp_SmartNICDisruptiveUpgrade), "Expected naples0 spec to have outstanding Disruptive Upgrade")
	AssertEventually(t, addSmartNICFailResponse("naples1", protos.SmartNICOp_SmartNICDisruptiveUpgrade), "Expected naples1 spec to have outstanding Disruptive Upgrade")
	AssertEventually(t, func() (bool, interface{}) { return IsFSMInState(t, stateMgr, t.Name(), fsmstRolloutSuccess) }, "Expecting Rollout to be in Success  state", "100ms", "2s")
}

// When number of failures is > MaxFailures, rollout should be in failed state.
func TestMaxFailuresHit(t *testing.T) {

	savedPreupgradeTimeout := preUpgradeTimeout
	preUpgradeTimeout = 100 * time.Millisecond
	defer func() {
		preUpgradeTimeout = savedPreupgradeTimeout
	}()

	const version = "v1.1"
	// create  state manager
	stateMgr, err := NewStatemgr(&dummyWriter{})
	AssertOk(t, err, "Error creating StateMgr")
	defer stateMgr.Stop()

	createSNIC := func(name string, labels map[string]string) { createSNICHelper(stateMgr, name, labels) }
	createSNIC("naples1", map[string]string{"l1": "n1"})
	createSNIC("naples0", map[string]string{"l1": "n1"})

	ro := roproto.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   t.Name(),
			Tenant: "default",
		},
		Spec: roproto.RolloutSpec{
			Version:                     version,
			ScheduledStartTime:          nil,
			Duration:                    "",
			Strategy:                    "",
			MaxParallel:                 3,
			MaxNICFailuresBeforeAbort:   0,
			OrderConstraints:            nil,
			Suspend:                     false,
			SmartNICsOnly:               true,
			SmartNICMustMatchConstraint: false, // hence smartnic upgrade only
		},
	}
	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &ro,
	}
	stateMgr.RolloutWatcher <- evt

	addSmartNICResponse := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			return addSmartNICResponseFilter(t, stateMgr, snic, op, version, "success")
		}
	}
	addSmartNICFailResponse := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			return addSmartNICResponseFilter(t, stateMgr, snic, op, version, "failure")
		}
	}
	AssertEventually(t, addSmartNICResponse("naples1", protos.SmartNICOp_SmartNICPreCheckForDisruptive), "Expected naples1 spec to have outstanding Precheck Op")
	AssertEventually(t, addSmartNICResponse("naples0", protos.SmartNICOp_SmartNICPreCheckForDisruptive), "Expected naples0 spec to have outstanding Precheck Op")
	AssertEventually(t, addSmartNICResponse("naples0", protos.SmartNICOp_SmartNICDisruptiveUpgrade), "Expected naples0 spec to have outstanding Disruptive Upgrade")
	AssertEventually(t, addSmartNICFailResponse("naples1", protos.SmartNICOp_SmartNICDisruptiveUpgrade), "Expected naples1 spec to have outstanding Disruptive Upgrade")
	AssertEventually(t, func() (bool, interface{}) { return IsFSMInState(t, stateMgr, t.Name(), fsmstRolloutFail) }, "Expecting Rollout to be in Failure  state", "100ms", "2s")
}

func TestExponentialRollout(t *testing.T) {
	const version = "v1.1"
	// create  state manager
	stateMgr, err := NewStatemgr(&dummyWriter{})
	AssertOk(t, err, "Error creating StateMgr")
	defer stateMgr.Stop()

	createNode := func(name string) { createNodeHelper(stateMgr, name) }
	createSNIC := func(name string, labels map[string]string) { createSNICHelper(stateMgr, name, labels) }

	createNode("node1")

	createSNIC("naples1", map[string]string{"l1": "n1"})
	createSNIC("naples0", map[string]string{"l1": "n0"})

	sn := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{
			Kind: "SmartNIC",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "naples0",
			Tenant: "default",
		},
	}
	stateMgr.handleSmartNICEvent(kvstore.Deleted, &sn)

	ro := roproto.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   t.Name(),
			Tenant: "default",
		},
		Spec: roproto.RolloutSpec{
			Version:                     version,
			ScheduledStartTime:          nil,
			Duration:                    "",
			Strategy:                    roproto.RolloutSpec_EXPONENTIAL.String(),
			MaxParallel:                 0,
			MaxNICFailuresBeforeAbort:   0,
			OrderConstraints:            nil,
			Suspend:                     false,
			SmartNICsOnly:               true,
			SmartNICMustMatchConstraint: false, // hence smartnic upgrade only
		},
	}
	evt := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &ro,
	}
	stateMgr.RolloutWatcher <- evt

	// Return true if if veniceRollout was not created
	checkNoVeniceReq := func() (bool, interface{}) {
		return checkNoVeniceRolloutHelper(t, stateMgr)
	}

	AssertConsistently(t, checkNoVeniceReq, "Expected no venice rollouts", "100ms", "1s")

	addSmartNICResponse := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			return addSmartNICResponseFilter(t, stateMgr, snic, op, version, "success")
		}
	}

	// Test that there is no request issued for any smartNIC
	checkNoSmartNICReq := func(snic string, op protos.SmartNICOp) func() (bool, interface{}) {
		return func() (bool, interface{}) {
			ret, i := checkNoPendingSmartNICOp(t, stateMgr, snic, op, version)
			return ret, i
		}
	}
	ros, err := stateMgr.GetRolloutState("default", t.Name())
	AssertOk(t, err, "GetRolloutstate should succeed")
	AssertConsistently(t, checkNoSmartNICReq("naples1", protos.SmartNICOp_SmartNICDisruptiveUpgrade), "Expected naples1 spec not to have RunVersion", "100ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ros.Status.SmartNICsStatus) == 0 {
			return false, nil
		}
		if ros.Status.SmartNICsStatus[0].Phase == roproto.RolloutPhase_PRE_CHECK.String() {
			return true, nil
		}
		return false, ros.Status.SmartNICsStatus[0].Phase
	}, "Status should be in PRE_CHECK")

	AssertEventually(t, addSmartNICResponse("naples1", protos.SmartNICOp_SmartNICPreCheckForDisruptive), "Expected naples1 spec to have outstanding Precheck Op")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ros.Status.SmartNICsStatus) == 0 {
			return false, nil
		}
		if ros.Status.SmartNICsStatus[0].Phase == roproto.RolloutPhase_PROGRESSING.String() {
			return true, nil
		}
		return false, ros.Status.SmartNICsStatus[0].Phase
	}, "Status should be in PROGRESSING")

	AssertEventually(t, addSmartNICResponse("naples1", protos.SmartNICOp_SmartNICDisruptiveUpgrade), "Expected naples1 spec to have outstanding RunVersion Op")
	AssertEventually(t, func() (bool, interface{}) {
		if len(ros.Status.SmartNICsStatus) == 0 {
			return false, nil
		}
		if ros.Status.SmartNICsStatus[0].Phase == roproto.RolloutPhase_COMPLETE.String() {
			return true, nil
		}
		return false, ros.Status.SmartNICsStatus[0].Phase
	}, "Status should be in Complete")

	AssertEventually(t, func() (bool, interface{}) { return IsFSMInState(t, stateMgr, t.Name(), fsmstRolloutSuccess) }, "Expecting Rollout to be successful state")

	// Deleting the rollout should delete all the venice/service/smartnic rollout objects
	stateMgr.RolloutWatcher <- kvstore.WatchEvent{Type: kvstore.Deleted, Object: &ro}
}
