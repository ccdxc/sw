// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"fmt"

	stringconv "strconv"

	"github.com/pensando/sw/nic/agent/dscagent"
	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
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
			name = sn.agent.InfraAPI.GetDscName()
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
		go func(ag *dscagent.DSCAgent) {
			found := CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == it.config.NumHosts, nil
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
					name = snl.agent.InfraAPI.GetDscName()
				}
				epname := fmt.Sprintf("testWorkload-%s-%s", name, name)
				epmeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
					ObjectMeta: api.ObjectMeta{
						Tenant:    "default",
						Namespace: "default",
						Name:      epname,
					},
				}
				sep, perr := ag.PipelineAPI.HandleEndpoint(agentTypes.Get, epmeta)
				if perr != nil {
					waitCh <- fmt.Errorf("Endpoint %s not found in netagent(%v), err=%v", epname, ag.InfraAPI.GetDscName(), perr)
					return
				}

				if sep[0].Spec.NodeUUID == ag.InfraAPI.GetDscName() {
					foundLocal = true
				}
			}
			if !foundLocal {
				waitCh <- fmt.Errorf("No local endpoint found on %s", ag.InfraAPI.GetDscName())
				return
			}

			waitCh <- nil
		}(sn.agent)

		name, err := strconv.ParseMacAddr(sn.macAddr)
		if err != nil {
			name = sn.agent.InfraAPI.GetDscName()
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
		go func(ag *dscagent.DSCAgent) {
			if !CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == 0, nil
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

// Test pagination
func (it *veniceIntegSuite) TestVeniceIntegWorkloadPaginated(c *C) {
	numWorkloads := 20 // For pagination test
	wrloads := make(map[string]*workload.Workload)
	it.createHostObjects()
	defer it.deleteHostObjects()

	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	workLoadCount := 0
	// create 20 workload on each NIC/host
	for i, sn := range it.snics {

		for j := 0; j < numWorkloads; j++ {
			// workload params
			var name string
			if name, err = strconv.ParseMacAddr(sn.macAddr); err != nil {
				name = sn.agent.InfraAPI.GetDscName()
			}
			wrloadName := fmt.Sprintf("testWorkload-%s-%s", name, stringconv.Itoa(workLoadCount))

			wl := &workload.Workload{
				TypeMeta: api.TypeMeta{Kind: "Workload"},
				ObjectMeta: api.ObjectMeta{
					Name:      wrloadName,
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
			wrloads[wrloadName] = wl

			// create workload
			_, err = it.restClient.WorkloadV1().Workload().Create(ctx, wl)
			AssertOk(c, err, "Error creating workload:"+wrloadName)
			log.Infof("Created workload: %+v", *wl)
			workLoadCount++
		}
	}

	ctx, err = it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// Test pagination

	// list workloads 5 till 9 (acc to 0 index)
	var numExpResults int32 = 5
	wrlist, err := it.restClient.WorkloadV1().Workload().List(ctx, &api.ListWatchOptions{From: 6, MaxResults: numExpResults, SortOrder: api.ListWatchOptions_ByCreationTime.String()})
	AssertOk(c, err, "Error listing workloads")
	Assert(c, (len(wrlist) == int(numExpResults)), fmt.Sprintf("Invalid number of workloads: expected %d found %d", len(wrlist), numExpResults))

	for _, gwr := range wrlist {
		AssertEquals(c, gwr.Spec.HostName, wrloads[gwr.ObjectMeta.Name].Spec.HostName, "workload params did not match")
	}

	// list workloads 7 till 15 (acc to 0 index), sorted creation time reverse
	numExpResults = 9
	wrlist, err = it.restClient.WorkloadV1().Workload().List(ctx, &api.ListWatchOptions{From: 7, MaxResults: numExpResults, SortOrder: api.ListWatchOptions_ByCreationTimeReverse.String()})
	AssertOk(c, err, "Error listing workloads")
	Assert(c, (len(wrlist) == int(numExpResults)), fmt.Sprintf("Invalid number of workloads: expected %d found %d", len(wrlist), numExpResults))

	for _, gwr := range wrlist {
		AssertEquals(c, gwr.Spec.HostName, wrloads[gwr.ObjectMeta.Name].Spec.HostName, "workload params did not match")
	}

	// List all Workloads
	numExpResults = int32(workLoadCount)
	wrlist, err = it.restClient.WorkloadV1().Workload().List(ctx, &api.ListWatchOptions{})
	AssertOk(c, err, "Error listing workloads")
	Assert(c, (len(wrlist) == int(numExpResults)), fmt.Sprintf("Invalid number of workloads: expected %d found %d", len(wrlist), numExpResults))

	for _, gwr := range wrlist {
		AssertEquals(c, gwr.Spec.HostName, wrloads[gwr.ObjectMeta.Name].Spec.HostName, "workload params did not match")
	}

	// List last 10 workloads, set
	wrlist, err = it.restClient.WorkloadV1().Workload().List(ctx, &api.ListWatchOptions{From: (int32(workLoadCount) - numExpResults), MaxResults: numExpResults})
	AssertOk(c, err, "Error listing workloads")
	Assert(c, (len(wrlist) == int(numExpResults)), fmt.Sprintf("Invalid number of workloads: expected %d found %d", len(wrlist), numExpResults))

	for _, gwr := range wrlist {
		AssertEquals(c, gwr.Spec.HostName, wrloads[gwr.ObjectMeta.Name].Spec.HostName, "workload params did not match")
	}

	// Delete the workloads
	for wlName, wl := range wrloads {
		_, err = it.apisrvClient.WorkloadV1().Workload().Delete(ctx, &wl.ObjectMeta)
		AssertOk(c, err, "Error deleting workload "+wlName)
	}

	fmt.Println("Done test")
}
