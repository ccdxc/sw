// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"context"
	"fmt"
	"time"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func (it *integTestSuite) TestNpmSgPolicy(c *C) {
	// if not present create the default tenant
	it.CreateTenant("default")
	// sg policy
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       "default",
			Namespace:    "default",
			Name:         "testpolicy",
			GenerationID: "1",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					Action:          "PERMIT",
					ToIPAddresses:   []string{"10.1.1.1/24"},
					FromIPAddresses: []string{"10.1.1.1/24"},
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
				},
			},
		},
	}

	// create sg policy
	_, err := it.apisrvClient.SecurityV1().SGPolicy().Create(context.Background(), &sgp)
	AssertOk(c, err, "error creating sg policy")

	// verify agent state has the policy and has the rules
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			gsgp, gerr := ag.nagent.NetworkAgent.FindSGPolicy(sgp.ObjectMeta)
			if gerr != nil {
				return false, nil
			}
			if len(gsgp.Spec.Rules) != len(sgp.Spec.Rules) {
				return false, gsgp
			}
			return true, nil
		}, fmt.Sprintf("Sg policy not correct in agent. DB: %v", ag.nagent.NetworkAgent.ListSGPolicy()), "10ms", it.pollTimeout())
	}

	// verify sgpolicy status reflects propagation status
	AssertEventually(c, func() (bool, interface{}) {
		tsgp, gerr := it.ctrler.StateMgr.FindSgpolicy(sgp.Tenant, sgp.Name)
		if err != nil {
			return false, gerr
		}
		if tsgp.SGPolicy.Status.PropagationStatus.GenerationID != sgp.ObjectMeta.GenerationID {
			return false, tsgp
		}
		if (tsgp.SGPolicy.Status.PropagationStatus.Updated != int32(it.numAgents)) || (tsgp.SGPolicy.Status.PropagationStatus.Pending != 0) ||
			(tsgp.SGPolicy.Status.PropagationStatus.MinVersion != "") {
			return false, tsgp
		}
		return true, nil
	}, "SgPolicy status was not updated after creating the policy", "100ms", it.pollTimeout())

	// wait a little so that we dont cause a race condition between NPM write ans updates
	time.Sleep(time.Millisecond * 10)

	// update the policy
	newRule := security.SGRule{
		Action:          "PERMIT",
		ToIPAddresses:   []string{"10.2.1.1/24"},
		FromIPAddresses: []string{"10.2.1.1/24"},
		ProtoPorts: []security.ProtoPort{
			{
				Protocol: "tcp",
				Ports:    "81",
			},
		},
	}
	sgp.Spec.Rules = append(sgp.Spec.Rules, newRule)
	sgp.ObjectMeta.GenerationID = "2"
	_, err = it.apisrvClient.SecurityV1().SGPolicy().Update(context.Background(), &sgp)
	AssertOk(c, err, "error updating sg policy")

	// verify agent state updated policy
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			gsgp, gerr := ag.nagent.NetworkAgent.FindSGPolicy(sgp.ObjectMeta)
			if gerr != nil {
				return false, nil
			}
			if len(gsgp.Spec.Rules) != len(sgp.Spec.Rules) {
				return false, gsgp
			}
			return true, nil
		}, fmt.Sprintf("Sg policy not correct in agent. DB: %v", ag.nagent.NetworkAgent.ListSGPolicy()), "10ms", it.pollTimeout())
	}

	// verify sgpolicy status reflects propagation status
	AssertEventually(c, func() (bool, interface{}) {
		tsgp, gerr := it.ctrler.StateMgr.FindSgpolicy(sgp.Tenant, sgp.Name)
		if err != nil {
			return false, gerr
		}
		if (tsgp.SGPolicy.Status.PropagationStatus.Updated != int32(it.numAgents)) || (tsgp.SGPolicy.Status.PropagationStatus.Pending != 0) ||
			(tsgp.SGPolicy.Status.PropagationStatus.MinVersion != "") {
			return false, tsgp
		}
		return true, nil
	}, "SgPolicy status was not updated after updating the policy", "100ms", it.pollTimeout())

	// delete sg policy
	_, err = it.apisrvClient.SecurityV1().SGPolicy().Delete(context.Background(), &sgp.ObjectMeta)
	AssertOk(c, err, "error deleting sg policy")
}

func (it *integTestSuite) TestNpmSgPolicyValidators(c *C) {
	// if not present create the default tenant
	it.CreateTenant("default")
	// sg policy refering an app that doesnt exist
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					Action:          "PERMIT",
					ToIPAddresses:   []string{"10.1.1.1/24"},
					FromIPAddresses: []string{"10.1.1.1/24"},
					Apps:            []string{"invalid"},
				},
			},
		},
	}

	// create sg policy and verify it fails
	_, err := it.apisrvClient.SecurityV1().SGPolicy().Create(context.Background(), &sgp)
	Assert(c, err != nil, "sgpolicy create with invalid app reference didnt fail")

	// create sg policy without app
	sgp.Spec.Rules[0].Apps = []string{}
	_, err = it.apisrvClient.SecurityV1().SGPolicy().Create(context.Background(), &sgp)
	AssertOk(c, err, "error creating sg policy")

	// try updating the policy with invalid app
	sgp.Spec.Rules[0].Apps = []string{"invalid"}
	_, err = it.apisrvClient.SecurityV1().SGPolicy().Update(context.Background(), &sgp)
	Assert(c, err != nil, "sgpolicy update with invalid app reference didnt fail")

	// ICMP app
	icmpApp := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "icmp-app",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			Timeout: "5m",
			ALG: &security.ALG{
				Type: "ICMP",
				Icmp: &security.Icmp{
					Type: "1",
					Code: "2",
				},
			},
		},
	}
	_, err = it.apisrvClient.SecurityV1().App().Create(context.Background(), &icmpApp)
	AssertOk(c, err, "Error creating icmp app")

	// refer the app from sg policy
	sgp.Spec.Rules[0].Apps = []string{"icmp-app"}
	_, err = it.apisrvClient.SecurityV1().SGPolicy().Update(context.Background(), &sgp)
	AssertOk(c, err, "Error updating sgpolicy with reference to icmp app")

	// try deleing the app, it should fail
	_, err = it.apisrvClient.SecurityV1().App().Delete(context.Background(), &icmpApp.ObjectMeta)
	Assert(c, err != nil, "Was able to delete app while sgpolicy was refering to it")

	// finally, delete sg policy
	_, err = it.apisrvClient.SecurityV1().SGPolicy().Delete(context.Background(), &sgp.ObjectMeta)
	AssertOk(c, err, "Error deleting sgpolicy with reference to icmp app")

	// now, we should be able to delete the app
	_, err = it.apisrvClient.SecurityV1().App().Delete(context.Background(), &icmpApp.ObjectMeta)
	AssertOk(c, err, "Error deleting icmp app")
}
