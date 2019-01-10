// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"fmt"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func (it *veniceIntegSuite) TestVeniceIntegWorkload(c *C) {
	wrloads := make([]workload.Workload, it.config.NumHosts)

	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// create a wait channel
	waitCh := make(chan error, it.config.NumHosts*2)

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
		_, err = it.restClient.WorkloadV1().Workload().Create(ctx, &wrloads[i])
		AssertOk(c, err, "Error creating workload")
		log.Infof("Created workload: %+v", &wrloads[i])
	}

	ctx, err = it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// list all workloads
	wrlist, err := it.restClient.WorkloadV1().Workload().List(ctx, &api.ListWatchOptions{})
	AssertOk(c, err, "Error listing workloads")
	Assert(c, (len(wrlist) == len(it.agents)), "Invalid number of workloads")

	for i := range it.agents {
		gwr, gerr := it.apisrvClient.WorkloadV1().Workload().Get(ctx, &wrloads[i].ObjectMeta)
		AssertOk(c, gerr, "Error getting workload")
		AssertEquals(c, gwr.Spec.HostName, wrloads[i].Spec.HostName, "workload params did not match")
	}

	// wait for all endpoints to be propagated to other agents
	for _, ag := range it.agents {
		go func(ag *netagent.Agent) {
			found := CheckEventually(func() (bool, interface{}) {
				return len(ag.NetworkAgent.ListEndpoint()) == it.config.NumHosts, nil
			}, "10ms", it.pollTimeout())
			fmt.Println(found)
			if !found {
				waitCh <- fmt.Errorf("Endpoint count incorrect in datapath")
				return
			}
			foundLocal := false
			for _, nag := range it.agents {
				epname := fmt.Sprintf("testWorkload-%s-%s", nag.NetworkAgent.NodeUUID, nag.NetworkAgent.NodeUUID)
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
		}(ag)
	}

	// wait for all goroutines to complete
	for i := 0; i < it.config.NumHosts; i++ {
		AssertOk(c, <-waitCh, "Endpoint info incorrect in datapath")
	}

	// delete workloads
	for i := range it.agents {
		_, err = it.apisrvClient.WorkloadV1().Workload().Delete(ctx, &wrloads[i].ObjectMeta)
		AssertOk(c, err, "Error deleting workload")
	}

	// verify all endpoints are gone
	for _, ag := range it.agents {
		go func(ag *netagent.Agent) {
			if !CheckEventually(func() (bool, interface{}) {
				return len(ag.NetworkAgent.ListEndpoint()) == 0, nil
			}, "10ms", it.pollTimeout()) {
				waitCh <- fmt.Errorf("Endpoint was not deleted from datapath")
				return
			}

			waitCh <- nil
		}(ag)
	}

	// wait for all goroutines to complete
	for i := 0; i < it.config.NumHosts; i++ {
		AssertOk(c, <-waitCh, "Endpoint delete error")
	}

	// delete the network
	_, err = it.deleteNetwork("default", "Network-Vlan-1")
	AssertOk(c, err, "Error deleting network")
}
