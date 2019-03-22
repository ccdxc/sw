// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"context"
	"fmt"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestNpmWorkloadCreateDelete tests smartNic, host and workload create/delete operations
func (it *integTestSuite) TestNpmWorkloadCreateDelete(c *C) {
	// if not present create the default tenant
	it.CreateTenant("default")
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
		name, err := strconv.ParseMacAddr(ag.nagent.NetworkAgent.NodeUUID)
		if err != nil {
			name = ag.nagent.NetworkAgent.NodeUUID
		}
		err = it.CreateWorkload("default", "default", fmt.Sprintf("testWorkload-%s", name), fmt.Sprintf("testHost-%d", i), ag.nagent.NetworkAgent.NodeUUID, uint32(100+i), 1)
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
			for _, ag := range it.agents {
				name, err := strconv.ParseMacAddr(ag.nagent.NetworkAgent.NodeUUID)
				if err != nil {
					name = ag.nagent.NetworkAgent.NodeUUID
				}
				epname := fmt.Sprintf("testWorkload-%s-%s", name, name)
				epmeta := api.ObjectMeta{
					Tenant:    "default",
					Namespace: "default",
					Name:      epname,
				}
				sep, perr := ag.nagent.NetworkAgent.FindEndpoint(epmeta)
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
		name, err := strconv.ParseMacAddr(ag.nagent.NetworkAgent.NodeUUID)
		if err != nil {
			name = ag.nagent.NetworkAgent.NodeUUID
		}
		err = it.DeleteWorkload("default", "default", fmt.Sprintf("testWorkload-%s", name))
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

func (it *integTestSuite) TestNpmWorkloadValidators(c *C) {
	// if not present create the default tenant
	it.CreateTenant("default")

	err := it.CreateWorkload("default", "default", "testWorkload-validator", "invalidHost", it.agents[0].nagent.NetworkAgent.NodeUUID, 101, 1)
	Assert(c, err != nil, "was able to create workload without a host")

	// create a host
	err = it.CreateHost("testHost", "00:01:02:03:04:05")
	AssertOk(c, err, "Error creating host")

	// create workload for the new host
	err = it.CreateWorkload("default", "default", "testWorkload-validator", "testHost", it.agents[0].nagent.NetworkAgent.NodeUUID, 101, 1)
	AssertOk(c, err, "Error creating Workload")

	// try updating the workload to invalid host
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload-validator",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "invalid",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   it.agents[0].nagent.NetworkAgent.NodeUUID,
					MicroSegVlan: 101,
					ExternalVlan: 1,
				},
			},
		},
	}

	// verify we cant change the host to non-existing value
	_, err = it.apisrvClient.WorkloadV1().Workload().Update(context.Background(), &wr)
	Assert(c, err != nil, "was able to update workload without a host")

	// verify we cant change the host to empty value
	wr.Spec.HostName = ""
	_, err = it.apisrvClient.WorkloadV1().Workload().Update(context.Background(), &wr)
	Assert(c, err != nil, "was able to update workload to empty host")

	// verify we cant delete the host
	err = it.DeleteHost("testHost")
	Assert(c, err != nil, "was able to delete host while workloads are refering to it")

	// delete the workload
	err = it.DeleteWorkload("default", "default", "testWorkload-validator")
	AssertOk(c, err, "Error deleting Workload")

	// finalyy delete the host
	err = it.DeleteHost("testHost")
	AssertOk(c, err, "Error deleting host")
}
