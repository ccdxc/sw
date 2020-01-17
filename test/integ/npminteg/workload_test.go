// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"context"
	"fmt"
	"time"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/workload"
	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	//"github.com/pensando/sw/api/generated/cluster"
	//"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/strconv"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestNpmWorkloadCreateDelete tests smartNic, host and workload create/delete operations
func (it *integTestSuite) TestNpmWorkloadCreateDelete(c *C) {
	// if not present create the default tenant
	it.CreateTenant("default")
	// create a wait channel
	waitCh := make(chan error, it.numAgents*2)

	// create a workload on each host
	for i := range it.agents {
		macAddr := fmt.Sprintf("0002.0000.%02x00", i)
		err := it.CreateWorkload("default", "default", fmt.Sprintf("testWorkload-%d", i), fmt.Sprintf("testHost-%d", i), macAddr, uint32(100+i), 1)
		AssertOk(c, err, "Error creating workload")
	}

	// verify the network got created for external vlan
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "Network-Vlan-1")
		return (nerr == nil), nil
	}, "Network not found in statemgr")

	// wait for all endpoints to be propagated to other agents
	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			found := CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == it.numAgents, nil
			}, "10ms", it.pollTimeout())
			if !found {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				log.Infof("Endpoint count expected [%v] found [%v]", it.numAgents, len(endpoints))
				waitCh <- fmt.Errorf("Endpoint count incorrect in datapath")
				return
			}
			foundLocal := false
			for idx := range it.agents {
				macAddr := fmt.Sprintf("0002.0000.%02x00", idx)
				name, _ := strconv.ParseMacAddr(macAddr)
				epname := fmt.Sprintf("testWorkload-%d-%s", idx, name)
				epmeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
					ObjectMeta: api.ObjectMeta{
						Tenant:    "default",
						Namespace: "default",
						Name:      epname,
					},
				}
				sep, perr := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.Get, epmeta)
				if perr != nil {
					waitCh <- fmt.Errorf("Endpoint %s not found in netagent, err=%v", epname, perr)
					return
				}
				if sep[0].Spec.NodeUUID == ag.dscAgent.InfraAPI.GetDscName() {
					foundLocal = true
				}
			}
			if !foundLocal {
				waitCh <- fmt.Errorf("No local endpoint found on %s", ag.dscAgent.InfraAPI.GetDscName())
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
	for idx := range it.agents {
		err := it.DeleteWorkload("default", "default", fmt.Sprintf("testWorkload-%d", idx))
		AssertOk(c, err, "Error deleting workload")
	}

	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			if !CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == 0, nil
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
	err := it.DeleteNetwork("default", "Network-Vlan-1")
	c.Assert(err, IsNil)
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", "Network-Vlan-1")
		return (nerr != nil), nil
	}, "Network still found in statemgr")
}

func (it *integTestSuite) TestNpmWorkloadValidators(c *C) {
	// if not present create the default tenant
	it.CreateTenant("default")

	macAddr := fmt.Sprintf("0002.0002.0000")
	wpname, _ := strconv.ParseMacAddr(macAddr)
	err := it.CreateWorkload("default", "default", "testWorkload-validator", "invalidHost", macAddr, 101, 1)
	Assert(c, err != nil, "was able to create workload without a host")

	// create a host
	err = it.CreateHost("testHost", "0001.0203.0405")
	AssertOk(c, err, "Error creating host")

	// create workload for the new host
	err = it.CreateWorkload("default", "default", "testWorkload-validator", "testHost", macAddr, 101, 1)
	AssertOk(c, err, "Error creating Workload")

	// verify we can find the endpoint
	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.npmCtrler.StateMgr.FindEndpoint("default", fmt.Sprintf("testWorkload-validator-%s", wpname))
		return (nerr == nil), nil
	}, "Endpoint not found in statemgr")

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
					MACAddress:   macAddr,
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
	time.Sleep(time.Millisecond * 10)

	// finalyy delete the host
	err = it.DeleteHost("testHost")
	AssertOk(c, err, "Error deleting host")
}

func (it *integTestSuite) TestNpmWorkloadUpdate(c *C) {
	c.Skip("Skipping till we fix workload update issue")

	// if not present create the default tenant
	it.CreateTenant("default")
	// create a wait channel
	waitCh := make(chan error, it.numAgents*2)

	// create a workload on each host
	for i := range it.agents {
		macAddr := fmt.Sprintf("0002.0000.%02x00", i)
		err := it.CreateWorkload("default", "default", fmt.Sprintf("testWorkload-%d", i), fmt.Sprintf("testHost-%d", i), macAddr, uint32(100+i), 1)
		AssertOk(c, err, "Error creating workload")
	}

	// wait for all endpoints to be propagated to other agents
	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			found := CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == it.numAgents, nil
			}, "10ms", it.pollTimeout())
			if !found {
				waitCh <- fmt.Errorf("Endpoint count incorrect in datapath")
				return
			}
			waitCh <- nil
		}(ag)
	}

	// wait for all goroutines to complete
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint info incorrect in datapath")
	}

	numIter := 10
	numChange := 10
	// update workloads
	for iter := 0; iter < numIter; iter++ {
		for chidx := 0; chidx < numChange; chidx++ {
			for i := range it.agents {
				macAddr := fmt.Sprintf("0002.00%02x.%02x%02x", iter, i, chidx)
				err := it.UpdateWorkload("default", "default", fmt.Sprintf("testWorkload-%d", i), fmt.Sprintf("testHost-%d", i), macAddr, uint32(100+i), 1)
				AssertOk(c, err, "Error creating workload")
			}
		}

		// wait for all endpoints to be propagated to other agents
		for _, ag := range it.agents {
			go func(ag *Dpagent) {
				found := CheckEventually(func() (bool, interface{}) {
					epMeta := netproto.Endpoint{
						TypeMeta: api.TypeMeta{Kind: "Endpoint"},
					}
					endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
					if len(endpoints) != it.numAgents {
						log.Warnf("Incorrect endpoint count %d on agent %v", len(endpoints), ag.dscAgent.InfraAPI.GetDscName())
						return false, nil
					}

					for idx := range it.agents {
						macAddr := fmt.Sprintf("0002.00%02x.%02x%02x", iter, idx, numChange-1)
						name, _ := strconv.ParseMacAddr(macAddr)
						epname := fmt.Sprintf("testWorkload-%d-%s", idx, name)
						epmeta := netproto.Endpoint{
							TypeMeta: api.TypeMeta{Kind: "Endpoint"},
							ObjectMeta: api.ObjectMeta{
								Tenant:    "default",
								Namespace: "default",
								Name:      epname,
							},
						}
						_, perr := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.Get, epmeta)
						if perr != nil {
							log.Warnf("Could not find endpoint %v on agent %v", epname, ag.dscAgent.InfraAPI.GetDscName())
							return false, nil
						}
					}

					return true, nil
				}, "10ms", it.pollTimeout())
				if !found {
					waitCh <- fmt.Errorf("Endpoint count incorrect in datapath")
					return
				}

				waitCh <- nil
			}(ag)
		}

		// wait for all goroutines to complete
		for i := 0; i < it.numAgents; i++ {
			AssertOk(c, <-waitCh, "Endpoint info incorrect in datapath")

		}
	}

	// now delete the workloads
	for idx := range it.agents {
		err := it.DeleteWorkload("default", "default", fmt.Sprintf("testWorkload-%d", idx))
		AssertOk(c, err, "Error deleting workload")
	}

	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			if !CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == 0, nil
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
}

func (it *integTestSuite) TestNpmHostUpdate(c *C) {
	c.Skip("Skipping this test since we dont allow host update in this release")

	// if not present create the default tenant
	it.CreateTenant("default")
	// create a wait channel
	waitCh := make(chan error, it.numAgents*2)

	// create a workload on each host
	for i := range it.agents {
		macAddr := fmt.Sprintf("0002.0000.%02x00", i)
		err := it.CreateWorkload("default", "default", fmt.Sprintf("testWorkload-%d", i), fmt.Sprintf("testHost-%d", i), macAddr, uint32(100+i), 1)
		AssertOk(c, err, "Error creating workload")
	}

	// wait for all endpoints to be propagated to other agents
	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			found := CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == it.numAgents, nil
			}, "10ms", it.pollTimeout())
			if !found {
				waitCh <- fmt.Errorf("Endpoint count incorrect in datapath")
				return
			}
			waitCh <- nil
		}(ag)
	}

	// wait for all goroutines to complete
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint info incorrect in datapath")
	}

	// update host and point it to invalid smartnic
	for i := range it.agents {
		hostName := fmt.Sprintf("testHost-%d", i)
		invSnicMac := fmt.Sprintf("0005.%02x00.0000", i)

		host := cluster.Host{
			TypeMeta: api.TypeMeta{Kind: "Host"},
			ObjectMeta: api.ObjectMeta{
				Name: hostName,
			},
			Spec: cluster.HostSpec{
				DSCs: []cluster.DistributedServiceCardID{
					{
						MACAddress: invSnicMac,
					},
				},
			},
			Status: cluster.HostStatus{},
		}

		_, err := it.apisrvClient.ClusterV1().Host().Update(context.Background(), &host)
		AssertOk(c, err, "Error updating host")
	}

	// verify all the endpoints are deleted
	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			if !CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == 0, nil
			}, "10ms", it.pollTimeout()) {
				waitCh <- fmt.Errorf("Endpoint was not deleted from datapath")
				return
			}

			waitCh <- nil
		}(ag)
	}
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint delete error")
	}

	// update host and point it to correct smartnic
	for i := range it.agents {
		hostName := fmt.Sprintf("testHost-%d", i)
		snicMac := fmt.Sprintf("0001.%02x00.0000", i)

		host := cluster.Host{
			TypeMeta: api.TypeMeta{Kind: "Host"},
			ObjectMeta: api.ObjectMeta{
				Name: hostName,
			},
			Spec: cluster.HostSpec{
				DSCs: []cluster.DistributedServiceCardID{
					{
						MACAddress: snicMac,
					},
				},
			},
			Status: cluster.HostStatus{},
		}

		_, err := it.apisrvClient.ClusterV1().Host().Update(context.Background(), &host)
		AssertOk(c, err, "Error updating host")
	}

	// verify all endpoints come back
	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			found := CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == it.numAgents, nil
			}, "10ms", it.pollTimeout())
			if !found {
				waitCh <- fmt.Errorf("Endpoint count incorrect in datapath")
				return
			}
			waitCh <- nil
		}(ag)
	}
	for i := 0; i < it.numAgents; i++ {
		AssertOk(c, <-waitCh, "Endpoint info incorrect in datapath")
	}

	// now delete the workloads
	for idx := range it.agents {
		err := it.DeleteWorkload("default", "default", fmt.Sprintf("testWorkload-%d", idx))
		AssertOk(c, err, "Error deleting workload")
	}

	for _, ag := range it.agents {
		go func(ag *Dpagent) {
			if !CheckEventually(func() (bool, interface{}) {
				epMeta := netproto.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				}
				endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
				return len(endpoints) == 0, nil
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
}

// TestNpmWorkloadCreateDeleteWithMultiIntf tests workload create/delete with multiple interfaces on it
func (it *integTestSuite) TestNpmWorkloadCreateDeleteWithMultiIntf(c *C) {
	c.Skip("Skipping till we fix workload update issues")

	const numWorkloads = 10
	const numIntf = 10
	const numIter = 10
	// if not present create the default tenant
	it.CreateTenant("default")
	// create a wait channel
	waitCh := make(chan error, it.numAgents*2)

	for iter := 0; iter < numIter; iter++ {
		// create number of workload on first host
		for i := 0; i < numWorkloads; i++ {
			// build workload object
			wr := workload.Workload{
				TypeMeta: api.TypeMeta{Kind: "Workload"},
				ObjectMeta: api.ObjectMeta{
					Name:      fmt.Sprintf("testWorkload-%d", i),
					Namespace: "default",
					Tenant:    "default",
				},
				Spec: workload.WorkloadSpec{
					HostName:   "testHost-0",
					Interfaces: []workload.WorkloadIntfSpec{},
				},
			}

			// each workload has multiple interfaces
			for j := 0; j < numIntf; j++ {
				macAddr := fmt.Sprintf("0002.0000.%02x%02x", i, j)
				wintf := workload.WorkloadIntfSpec{
					MACAddress:   macAddr,
					MicroSegVlan: uint32(i*numIntf + j + 1),
					ExternalVlan: uint32(j + 1),
				}
				wr.Spec.Interfaces = append(wr.Spec.Interfaces, wintf)
			}
			_, err := it.apisrvClient.WorkloadV1().Workload().Create(context.Background(), &wr)
			AssertOk(c, err, "Error creating workload")
		}

		// wait for all endpoints to be propagated to other agents
		for _, ag := range it.agents {
			go func(ag *Dpagent) {
				found := CheckEventually(func() (bool, interface{}) {
					epMeta := netproto.Endpoint{
						TypeMeta: api.TypeMeta{Kind: "Endpoint"},
					}
					endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
					return len(endpoints) == numWorkloads*numIntf, nil
				}, "10ms", it.pollTimeout())
				if !found {
					waitCh <- fmt.Errorf("Endpoint count incorrect in datapath")
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
		for i := 0; i < numWorkloads; i++ {
			// build workload object
			wmeta := api.ObjectMeta{
				Name:      fmt.Sprintf("testWorkload-%d", i),
				Namespace: "default",
				Tenant:    "default",
			}
			_, err := it.apisrvClient.WorkloadV1().Workload().Delete(context.Background(), &wmeta)
			AssertOk(c, err, "Error deleting workload")
		}

		// verify endpoints are gone from apiserver
		AssertEventually(c, func() (bool, interface{}) {
			var listopt api.ListWatchOptions
			eplist, lerr := it.apisrvClient.WorkloadV1().Endpoint().List(context.Background(), &listopt)
			if lerr == nil && len(eplist) == 0 {
				return true, nil
			}
			return false, eplist
		}, "Endpoints still found in apiserver")

		// verify endpoints are gone from agents
		for _, ag := range it.agents {
			go func(ag *Dpagent) {
				if !CheckEventually(func() (bool, interface{}) {
					epMeta := netproto.Endpoint{
						TypeMeta: api.TypeMeta{Kind: "Endpoint"},
					}
					endpoints, _ := ag.dscAgent.PipelineAPI.HandleEndpoint(agentTypes.List, epMeta)
					return len(endpoints) == 0, nil
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
	}

	// delete the networks
	for j := 0; j < numIntf; j++ {
		err := it.DeleteNetwork("default", fmt.Sprintf("Network-Vlan-%d", j+1))
		c.Assert(err, IsNil)
		AssertEventually(c, func() (bool, interface{}) {
			_, nerr := it.npmCtrler.StateMgr.FindNetwork("default", fmt.Sprintf("Network-Vlan-%d", j+1))
			return (nerr != nil), nil
		}, "Network still found in NPM")
	}
}
