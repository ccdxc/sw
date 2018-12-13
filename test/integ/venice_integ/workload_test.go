// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"context"
	"fmt"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func (it *veniceIntegSuite) TestVeniceIntegWorkload(c *C) {
	wrloads := make([]workload.Workload, it.numAgents)

	// create a wait channel
	waitCh := make(chan error, it.numAgents*2)

	// create a network using REST api
	nw, err := it.createNetwork("default", "default", "Vlan-1", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "Error creating network")

	// verify network gets created in agent
	AssertEventually(c, func() (bool, interface{}) {
		_, cerr := it.agents[0].NetworkAgent.FindNetwork(nw.ObjectMeta)
		return (cerr == nil), nil
	}, "Network not found in agent", "100ms", "10s")

	// create a workload on each NIC/host
	for i, ag := range it.agents {
		// workload params
		wrloads[i] = workload.Workload{
			TypeMeta: api.TypeMeta{Kind: "Workload"},
			ObjectMeta: api.ObjectMeta{
				Name:      fmt.Sprintf("testWorkload-%s", ag.NetworkAgent.NodeUUID),
				Namespace: "default",
				Tenant:    "default",
			},
			Spec: workload.WorkloadSpec{
				HostName: fmt.Sprintf("host%d", i),
				Interfaces: []workload.WorkloadIntfSpec{
					{
						MACAddress:   ag.NetworkAgent.NodeUUID,
						MicroSegVlan: uint32(i + 100),
						ExternalVlan: 1,
					},
				},
			},
		}

		// create workload
		_, err := it.apisrvClient.WorkloadV1().Workload().Create(context.Background(), &wrloads[i])
		AssertOk(c, err, "Error creating workload")
	}

	// wait for all endpoints to be propagated to other agents
	for i, ag := range it.agents {
		go func(ag *netagent.Agent, dp *datapath.Datapath) {
			found := CheckEventually(func() (bool, interface{}) {
				return ((dp.GetEndpointCount() == it.numAgents) && (len(ag.NetworkAgent.ListEndpoint()) == it.numAgents)), nil
			}, "10ms", "10s")
			if !found {
				waitCh <- fmt.Errorf("Endpoint count incorrect in datapath")
				return
			}
			foundLocal := false
			for _, ag := range it.agents {
				epname := fmt.Sprintf("testWorkload-%s-%s", ag.NetworkAgent.NodeUUID, ag.NetworkAgent.NodeUUID)
				eps, perr := dp.FindEndpoint(fmt.Sprintf("%s|%s", "default", epname))
				if perr != nil || len(eps.Request) != 1 {
					waitCh <- fmt.Errorf("Endpoint %s not found in datapath, eps=%+v, err=%v", epname, eps, perr)
					return
				}
				sep, perr := ag.NetworkAgent.FindEndpoint("default", "default", epname)
				if perr != nil {
					waitCh <- fmt.Errorf("Endpoint %s not found in netagent(%v), err=%v, db: %+v", epname, ag.NetworkAgent.NodeUUID, perr, ag.NetworkAgent.EndpointDB)
					return
				}

				if sep.Spec.NodeUUID == ag.NetworkAgent.NodeUUID {
					foundLocal = true
				}

			}
			if !foundLocal {
				waitCh <- fmt.Errorf("No local endpoint found on %s", ag.NetworkAgent.NodeUUID)
				return
			}

			waitCh <- nil
		}(ag, it.datapaths[i])
	}

	// wait for all goroutines to complete
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint info incorrect in datapath")
	}

	// delete workloads
	for i := range it.agents {
		_, err := it.apisrvClient.WorkloadV1().Workload().Delete(context.Background(), &wrloads[i].ObjectMeta)
		AssertOk(c, err, "Error creating workload")
	}

	// verify all endpoints are gone
	for i, ag := range it.agents {
		go func(ag *netagent.Agent, dp *datapath.Datapath) {
			if !CheckEventually(func() (bool, interface{}) {
				return (dp.GetEndpointCount() == 0), nil
			}, "10ms", "10s") {
				waitCh <- fmt.Errorf("Endpoint was not deleted from datapath")
				return
			}

			waitCh <- nil
		}(ag, it.datapaths[i])
	}

	// wait for all goroutines to complete
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint delete error")
	}

	// delete the network
	_, err = it.deleteNetwork("default", "Vlan-1")
	AssertOk(c, err, "Error deleting network")
}
