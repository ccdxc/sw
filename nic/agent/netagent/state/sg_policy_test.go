package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestSGPolicyCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachGroup:  []string{"preCreatedSecurityGroup"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: []string{"PERMIT"},
					Src: &netproto.MatchSelector{
						Address:   "10.0.0.0 - 10.0.1.0",
						App:       "L4PORT",
						AppConfig: "80",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.1 - 192.168.1.0",
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindSGPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testSGPolicy", "SGPolicy names did not match", sgp)

	// verify duplicate tenant creations succeed
	err = ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating duplicate sg policy")

	// verify list api works.
	sgpList := ag.ListSGPolicy()
	Assert(t, len(sgpList) == 1, "Incorrect number of sg policies")

	// delete the sg policy and verify its gone from db
	err = ag.DeleteSGPolicy(&sgPolicy)
	AssertOk(t, err, "Error deleting sg policy")
	_, err = ag.FindSGPolicy(sgPolicy.ObjectMeta)
	Assert(t, err != nil, "SG Policy was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteSGPolicy(&sgPolicy)
	Assert(t, err != nil, "deleting non-existing sg policy succeeded", ag)
}

func TestSGPolicyUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachGroup:  []string{"preCreatedSecurityGroup"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: []string{"PERMIT"},
					Src: &netproto.MatchSelector{
						Address:   "10.0.0.0 - 10.0.1.0",
						App:       "L4PORT",
						AppConfig: "80",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.1 - 192.168.1.0",
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindSGPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG policy was not found in DB")
	Assert(t, sgp.Name == "testSGPolicy", "Nat Pool names did not match", sgp)

	sgpSpec := netproto.SGPolicySpec{
		AttachGroup: []string{"preCreatedSecurityGroup"},
		Rules: []netproto.PolicyRule{
			{
				Action: []string{"DENY", "LOG"},
			},
		},
	}

	sgPolicy.Spec = sgpSpec

	err = ag.UpdateSGPolicy(&sgPolicy)
	AssertOk(t, err, "Error updating sg policy")

	updSGPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachGroup:  []string{"preCreatedSecurityGroup"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: []string{"PERMIT"},
					Src: &netproto.MatchSelector{
						Address:   "10.0.0.0 - 10.0.1.0",
						App:       "L4PORT",
						AppConfig: "80",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.1 - 192.168.1.0",
					},
				},
			},
		},
	}
	err = ag.UpdateSGPolicy(&updSGPolicy)
	AssertOk(t, err, "Error updating sg policy")

}

//--------------------- Corner Case Tests ---------------------//

func TestSGPolicyOnMatchAllSrc(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachGroup:  []string{"preCreatedSecurityGroup"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: []string{"PERMIT"},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.1 - 192.168.1.0",
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "SG Policy creation with empty src should match all and pass validation")
}

func TestSGPolicyOnMatchAllDst(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachGroup:  []string{"preCreatedSecurityGroup"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: []string{"PERMIT"},
					Src: &netproto.MatchSelector{
						Address:   "10.0.0.0 - 10.0.1.0",
						App:       "L4PORT",
						AppConfig: "80",
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "SG Policy creation with empty dst should match all and pass validation")
}

func TestSGPolicyOnMatchAll(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachGroup:  []string{"preCreatedSecurityGroup"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: []string{"PERMIT"},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "SG Policy creation with empty src and dst should match all and pass validation")
}

func TestSGPolicyUpdateOnNonExistentSGPolicy(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")
	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachGroup:  []string{"preCreatedSecurityGroup"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: []string{"PERMIT"},
					Src: &netproto.MatchSelector{
						Address:   "10.0.0.0 - 10.0.1.0",
						App:       "L4PORT",
						AppConfig: "80",
					},
					Dst: &netproto.MatchSelector{
						Address: "192.168.0.1 - 192.168.1.0",
					},
				},
			},
		},
	}

	// create sg policy
	err = ag.UpdateSGPolicy(&sgPolicy)
	Assert(t, err != nil, "Nat policy updates on non existing nat policies fail")
}
