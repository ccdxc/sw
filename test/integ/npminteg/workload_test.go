// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"fmt"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestNpmWorkloadCreateDelete tests smartNic, host and workload create/delete operations
func (it *integTestSuite) TestNpmWorkloadCreateDelete(c *C) {
	// create a wait channel
	waitCh := make(chan error, it.numAgents*2)

	// create a host for each agent
	for idx, ag := range it.agents {
		err := it.CreateHost(fmt.Sprintf("testHost-%d", idx), ag.nagent.NetworkAgent.NodeUUID)
		AssertOk(c, err, "Error creating host")
	}

	// create a network in controller
	// FIXME: we shouldnt have to create this network
	err := it.CreateNetwork("default", "default", "Vlan-1", "10.1.0.0/22", "10.1.1.254")
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "Vlan-1")
		return (nerr == nil), nil
	}, "Network not found in statemgr")

	// wait till agent has the network
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			ometa := api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "Vlan-1"}
			_, nerr := ag.nagent.NetworkAgent.FindNetwork(ometa)
			return (nerr == nil), nil
		}, "Network not found in agent")
	}

	// create a workload on each host
	for i, ag := range it.agents {
		err = it.CreateWorkload("default", "default", fmt.Sprintf("testWorkload-%s", ag.nagent.NetworkAgent.NodeUUID), fmt.Sprintf("testHost-%d", i), ag.nagent.NetworkAgent.NodeUUID, uint32(100+i), 1)
		AssertOk(c, err, "Error creating workload")
	}

	// verify the network got created for external vlan
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "Vlan-1")
		return (nerr == nil), nil
	}, "Network not found in statemgr")

	// wait for all endpoints to be propagated to other agents
	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			found := CheckEventually(func() (bool, interface{}) {
				return len(ag.nagent.NetworkAgent.ListEndpoint()) == it.numAgents, nil
			}, "10ms", it.pollTimeout())
			if !found {
				waitCh <- fmt.Errorf("Endpoint count incorrect in datapath")
				return
			}
			foundLocal := false
			for _, nag := range it.agents {
				epname := fmt.Sprintf("testWorkload-%s-%s", nag.nagent.NetworkAgent.NodeUUID, nag.nagent.NetworkAgent.NodeUUID)
				eps, perr := ag.nagent.NetworkAgent.FindEndpoint("default", "default", epname)
				if perr != nil {
					waitCh <- fmt.Errorf("Endpoint %s not found in datapath, eps=%+v, err=%v", epname, eps, perr)
					return
				}
				sep, perr := ag.nagent.NetworkAgent.FindEndpoint("default", "default", epname)
				if perr != nil {
					waitCh <- fmt.Errorf("Endpoint %s not found in netagent, err=%v", epname, perr)
					return
				}
				if sep.Spec.NodeUUID == ag.nagent.NetworkAgent.NodeUUID {
					foundLocal = true
				}
			}
			if !foundLocal {
				waitCh <- fmt.Errorf("No local endpoint found on %s", ag.nagent.NetworkAgent.NodeUUID)
				return
			}
			waitCh <- nil
		}(ag)
	}

	// wait for all goroutines to complete
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint info incorrect in datapath")

	}

	// now delete the workloads
	for _, ag := range it.agents {
		err := it.DeleteWorkload("default", "default", fmt.Sprintf("testWorkload-%s", ag.nagent.NetworkAgent.NodeUUID))
		AssertOk(c, err, "Error deleting workload")
	}

	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			if !CheckEventually(func() (bool, interface{}) {
				return len(ag.nagent.NetworkAgent.ListEndpoint()) == 0, nil
			}, "10ms", it.pollTimeout()) {
				waitCh <- fmt.Errorf("Endpoint was not deleted from datapath")
				return
			}

			waitCh <- nil
		}(ag)
	}

	// wait for all goroutines to complete
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint delete error")
	}

	// delete the network
	err = it.DeleteNetwork("default", "Vlan-1")
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.ctrler.StateMgr.FindNetwork("default", "Vlan-1")
		return (nerr != nil), nil
	}, "Network still found in statemgr")
}
