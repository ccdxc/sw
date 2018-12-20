// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"fmt"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/utils/kvstore"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func (it *integTestSuite) TestNpmSgPolicy(c *C) {
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
					Action:        "PERMIT",
					ToIPAddresses: []string{"10.1.1.1/24"},
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
	err := it.ctrler.Watchr.SgpolicyEvent(kvstore.Created, &sgp)
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
		}, fmt.Sprintf("Sg policy not correct in agent. DB: %v", ag.datapath.DB.SgPolicyDB), "10ms", it.pollTimeout())
	}

	// verify sgpolicy status reflects propagation status
	AssertEventually(c, func() (bool, interface{}) {
		tsgp, gerr := it.ctrler.StateMgr.FindSgpolicy(sgp.Tenant, sgp.Name)
		if err != nil {
			return false, gerr
		}
		if tsgp.Status.PropagationStatus.GenerationID != sgp.ObjectMeta.GenerationID {
			return false, tsgp
		}
		if (tsgp.Status.PropagationStatus.Updated != int32(it.numAgents)) || (tsgp.Status.PropagationStatus.Pending != 0) ||
			(tsgp.Status.PropagationStatus.MinVersion != "") {
			return false, tsgp
		}
		return true, nil
	}, "SgPolicy status was not updated after creating the policy", "100ms", it.pollTimeout())

	// update the policy
	newRule := security.SGRule{
		Action:        "PERMIT",
		ToIPAddresses: []string{"10.1.1.1/24"},
		ProtoPorts: []security.ProtoPort{
			{
				Protocol: "tcp",
				Ports:    "80",
			},
		},
	}
	sgp.Spec.Rules = append(sgp.Spec.Rules, newRule)
	sgp.ObjectMeta.GenerationID = "2"
	err = it.ctrler.Watchr.SgpolicyEvent(kvstore.Updated, &sgp)
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
		}, fmt.Sprintf("Sg policy not correct in agent. DB: %v", ag.datapath.DB.SgPolicyDB), "10ms", it.pollTimeout())
	}

	// verify sgpolicy status reflects propagation status
	AssertEventually(c, func() (bool, interface{}) {
		tsgp, gerr := it.ctrler.StateMgr.FindSgpolicy(sgp.Tenant, sgp.Name)
		if err != nil {
			return false, gerr
		}
		if tsgp.Status.PropagationStatus.GenerationID != sgp.ObjectMeta.GenerationID {
			return false, tsgp
		}
		if (tsgp.Status.PropagationStatus.Updated != int32(it.numAgents)) || (tsgp.Status.PropagationStatus.Pending != 0) ||
			(tsgp.Status.PropagationStatus.MinVersion != "") {
			return false, tsgp
		}
		return true, nil
	}, "SgPolicy status was not updated after updating the policy", "100ms", it.pollTimeout())
}
