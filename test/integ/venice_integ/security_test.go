// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"context"
	"fmt"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func (it *veniceIntegSuite) TestVeniceIntegSecurityPolicy(c *C) {
	// sg policy params
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "policy1",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules: []*security.SGRule{
				{
					FromIPAddresses: []string{"10.0.0.0/24"},
					ToIPAddresses:   []string{"11.0.0.0/24"},
					Apps:            []string{"tcp/80"},
					Action:          "PERMIT",
				},
			},
		},
	}

	// create security policy
	_, err := it.apisrvClient.SecurityV1().SGPolicy().Create(context.Background(), &sgp)
	AssertOk(c, err, "Error creating security policy")

	// verify policy gets created in agent
	AssertEventually(c, func() (bool, interface{}) {
		notFound := false
		for _, ag := range it.agents {
			rsgp, cerr := ag.NetworkAgent.FindSGPolicy(sgp.ObjectMeta)
			if (cerr != nil) || (rsgp.Name != sgp.Name) {
				notFound = true
			}
		}
		return (notFound == false), nil
	}, "SgPolicy not found in agent", "100ms", "10s")

	// delete policy
	_, err = it.apisrvClient.SecurityV1().SGPolicy().Delete(context.Background(), &sgp.ObjectMeta)
	AssertOk(c, err, "Error deleting sgpolicy")

	// verify policy gets deleted in agent
	AssertEventually(c, func() (bool, interface{}) {
		found := false
		for _, ag := range it.agents {
			_, cerr := ag.NetworkAgent.FindSGPolicy(sgp.ObjectMeta)
			if cerr == nil {
				found = true
			}
		}
		return (found == false), nil
	}, "SgPolicy still found in agent", "100ms", "10s")
}

func (it *veniceIntegSuite) TestVeniceIntegSecuritygroup(c *C) {
	wrloads := make([]workload.Workload, it.numAgents)

	// create a wait channel
	waitCh := make(chan error, it.numAgents*2)

	// sg params
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "group1",
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}),
		},
	}

	// create security policy
	_, err := it.apisrvClient.SecurityV1().SecurityGroup().Create(context.Background(), &sg)
	AssertOk(c, err, "Error creating security group")

	// verify policy gets created in agent
	AssertEventually(c, func() (bool, interface{}) {
		notFound := false
		var cerr error
		var rsg *netproto.SecurityGroup
		for _, ag := range it.agents {
			rsg, cerr = ag.NetworkAgent.FindSecurityGroup(sg.ObjectMeta)
			if (cerr != nil) || (rsg.Name != sg.Name) {
				notFound = true
			}
		}
		return (notFound == false), []interface{}{rsg, cerr}
	}, "security group not found in agent", "100ms", "10s")

	// create a workload on each NIC/host
	for i := range it.agents {
		// workload params
		wrloads[i] = workload.Workload{
			TypeMeta: api.TypeMeta{Kind: "Workload"},
			ObjectMeta: api.ObjectMeta{
				Name:      fmt.Sprintf("testWorkload%d", i),
				Namespace: "default",
				Tenant:    "default",
				Labels:    labels.Set{"env": "production", "app": "procurement"},
			},
			Spec: workload.WorkloadSpec{
				HostName: fmt.Sprintf("host%d", i),
				Interfaces: map[string]workload.WorkloadIntfSpec{
					fmt.Sprintf("00:01:02:03:04:%02d", i): workload.WorkloadIntfSpec{
						MicroSegVlan: 100,
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
			for i := range it.agents {
				epname := fmt.Sprintf("testWorkload%d-00:01:02:03:04:%02d", i, i)
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
				// verify endpoint is part of the security group
				if (len(sep.Spec.SecurityGroups) != 1) || (sep.Spec.SecurityGroups[0] != sg.Name) {
					waitCh <- fmt.Errorf("Endpoint has invalid security group. {%+v}", sep)
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

	// delete the security group
	_, err = it.apisrvClient.SecurityV1().SecurityGroup().Delete(context.Background(), &sg.ObjectMeta)
	AssertOk(c, err, "Error creating workload")

	// verify policy gets removed from agent
	AssertEventually(c, func() (bool, interface{}) {
		found := false
		for _, ag := range it.agents {
			_, cerr := ag.NetworkAgent.FindSecurityGroup(sg.ObjectMeta)
			if cerr == nil {
				found = true
			}
		}
		return (found == false), nil
	}, "security group still found in agent", "100ms", "10s")

	// verify SG to endpoint association is removed
	for i, ag := range it.agents {
		go func(ag *netagent.Agent, dp *datapath.Datapath) {
			found := CheckEventually(func() (bool, interface{}) {
				for i := range it.agents {
					epname := fmt.Sprintf("testWorkload%d-00:01:02:03:04:%02d", i, i)
					sep, perr := ag.NetworkAgent.FindEndpoint("default", "default", epname)
					if perr != nil {
						return false, perr
					}
					if len(sep.Spec.SecurityGroups) != 0 {
						return false, sep
					}
				}
				return true, nil
			}, "10ms", "10s")
			if !found {
				waitCh <- fmt.Errorf("Endpoint count incorrect in datapath")
				return
			}
			for i := range it.agents {
				epname := fmt.Sprintf("testWorkload%d-00:01:02:03:04:%02d", i, i)

				sep, perr := ag.NetworkAgent.FindEndpoint("default", "default", epname)
				if perr != nil {
					waitCh <- fmt.Errorf("Endpoint %s not found in netagent(%v), err=%v, db: %+v", epname, ag.NetworkAgent.NodeUUID, perr, ag.NetworkAgent.EndpointDB)
					return
				}

				// verify endpoint is part of the security group
				if len(sep.Spec.SecurityGroups) != 0 {
					waitCh <- fmt.Errorf("Endpoint has invalid security group. {%+v}", sep)
				}
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
