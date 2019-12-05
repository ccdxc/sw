// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"fmt"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func (it *veniceIntegSuite) TestVeniceIntegWorkload(c *C) {
	wrloads := make([]workload.Workload, it.config.NumHosts)
	it.createHostObjects()
	defer it.deleteHostObjects()

	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// create a wait channel
	waitCh := make(chan error, it.config.NumHosts*2)

	// create a workload on each NIC/host
	for i, sn := range it.snics {
		// workload params
		var name string
		if name, err = strconv.ParseMacAddr(sn.macAddr); err != nil {
			name = sn.agent.NetworkAgent.NodeUUID
		}
		wrloads[i] = workload.Workload{
			TypeMeta: api.TypeMeta{Kind: "Workload"},
			ObjectMeta: api.ObjectMeta{
				Name:      fmt.Sprintf("testWorkload-%s", name),
				Namespace: "default",
				Tenant:    "default",
			},
			Spec: workload.WorkloadSpec{
				HostName: fmt.Sprintf("host%d", i),
				Interfaces: []workload.WorkloadIntfSpec{
					{
						MACAddress:   sn.macAddr,
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
	Assert(c, (len(wrlist) == len(it.snics)), fmt.Sprintf("Invalid number of workloads: expected %d found %d", len(wrlist), len(it.snics)))

	for i := range it.snics {
		gwr, gerr := it.apisrvClient.WorkloadV1().Workload().Get(ctx, &wrloads[i].ObjectMeta)
		AssertOk(c, gerr, "Error getting workload")
		AssertEquals(c, gwr.Spec.HostName, wrloads[i].Spec.HostName, "workload params did not match")
	}

	// wait for all endpoints to be propagated to other agents
	for i, sn := range it.snics {
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
			for _, snl := range it.snics {
				name, err := strconv.ParseMacAddr(snl.macAddr)
				if err != nil {
					name = snl.agent.NetworkAgent.NodeUUID
				}
				epname := fmt.Sprintf("testWorkload-%s-%s", name, name)
				epmeta := api.ObjectMeta{
					Tenant:    "default",
					Namespace: "default",
					Name:      epname,
				}
				sep, perr := ag.NetworkAgent.FindEndpoint(epmeta)
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
		}(sn.agent)

		name, err := strconv.ParseMacAddr(sn.macAddr)
		if err != nil {
			name = sn.agent.NetworkAgent.NodeUUID
		}
		epname := fmt.Sprintf("testWorkload-%s-%s", name, name)
		epmeta := api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      epname,
		}
		AssertEventually(c, func() (bool, interface{}) {
			gep, gerr := it.apisrvClient.WorkloadV1().Endpoint().Get(ctx, &epmeta)
			if gerr != nil {
				return false, fmt.Errorf("Endpoint %s not found in apiserver", epname)
			}
			if gep.Status.MacAddress != wrloads[i].Spec.Interfaces[0].MACAddress {
				return false, fmt.Errorf("Endpoint %s not found in apiserver", epname)
			}
			return true, nil
		}, fmt.Sprintf("Endpoint %s not found in apiserver", epname), "100ms", it.pollTimeout())
	}

	// wait for all goroutines to complete
	for i := 0; i < it.config.NumHosts; i++ {
		AssertOk(c, <-waitCh, "Endpoint info incorrect in datapath")
	}

	// delete workloads
	for i := range it.snics {
		_, err = it.apisrvClient.WorkloadV1().Workload().Delete(ctx, &wrloads[i].ObjectMeta)
		AssertOk(c, err, "Error deleting workload")
	}

	// verify all endpoints are gone from api server
	AssertEventually(c, func() (bool, interface{}) {
		eplist, err := it.restClient.WorkloadV1().Endpoint().List(ctx, &api.ListWatchOptions{})
		if err != nil {
			return false, nil
		}
		if len(eplist) > 0 {
			return false, eplist
		}
		return true, nil
	}, "Endpoint still found in venice", "100ms", it.pollTimeout())

	// verify all endpoints are gone
	for _, sn := range it.snics {
		go func(ag *netagent.Agent) {
			if !CheckEventually(func() (bool, interface{}) {
				return len(ag.NetworkAgent.ListEndpoint()) == 0, nil
			}, "10ms", it.pollTimeout()) {
				waitCh <- fmt.Errorf("Endpoint was not deleted from datapath")
				return
			}

			waitCh <- nil
		}(sn.agent)
	}

	// wait for all goroutines to complete
	for i := 0; i < it.config.NumHosts; i++ {
		AssertOk(c, <-waitCh, "Endpoint delete error")
	}
	// TODO: add when delete of network is supported
	//_, err = it.deleteNetwork("default", "Network-Vlan-1")
	//AssertOk(c, err, "Error deleting network")
}
